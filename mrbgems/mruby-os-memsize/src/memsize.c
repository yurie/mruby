#include <mruby.h>
#include <mruby/gc.h>
#include <mruby/hash.h>
#include <mruby/class.h>
#include <mruby/object.h>
#include <mruby/numeric.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/variable.h>
#include <mruby/proc.h>
#include <mruby/value.h>
#include <mruby/range.h>

static mrb_int
os_memsize_of_ivars(mrb_state* mrb, mrb_value obj)
{
  return mrb_obj_iv_tbl_memsize(mrb, obj);
}

static mrb_int
os_memsize_of_irep(mrb_state* state, const struct mrb_irep *irep)
{
  mrb_int size, i;
  size = (irep->slen * sizeof(mrb_sym)) +
         (irep->plen * sizeof(mrb_code)) +
         (irep->ilen * sizeof(mrb_code));

  for(i = 0; i < irep->rlen; i++) {
    size += os_memsize_of_irep(state, irep->reps[i]);
  }
  return size;
}

static mrb_int
os_memsize_of_method(mrb_state* mrb, mrb_value method_obj)
{
  mrb_int size;
  mrb_value proc_value = mrb_obj_iv_get(mrb, mrb_obj_ptr(method_obj),
                                        mrb_intern_lit(mrb, "_proc"));
  struct RProc *proc = mrb_proc_ptr(proc_value);

  size = sizeof(struct RProc);
  if (!MRB_PROC_CFUNC_P(proc)) size += os_memsize_of_irep(mrb, proc->body.irep);
  return size;
}

static mrb_int
os_memsize_of_object(mrb_state* mrb, mrb_value obj)
{
  mrb_int size = 0;

  switch(mrb_type(obj)) {
    case MRB_TT_STRING:
      size += mrb_objspace_page_slot_size();
      if (!RSTR_EMBED_P(RSTRING(obj)) && !RSTR_SHARED_P(RSTRING(obj))) {
        size += RSTRING_CAPA(obj);
      }
      break;
    case MRB_TT_CLASS:
    case MRB_TT_MODULE:
    case MRB_TT_SCLASS:
    case MRB_TT_ICLASS:
      size += mrb_gc_mark_mt_size(mrb, mrb_class_ptr(obj)) * sizeof(mrb_method_t);
      /* fall through */
    case MRB_TT_EXCEPTION:
    case MRB_TT_OBJECT: {
      size += mrb_objspace_page_slot_size();
      size += os_memsize_of_ivars(mrb, obj);
      if (mrb_obj_is_kind_of(mrb, obj, mrb_class_get(mrb, "UnboundMethod")) ||
          mrb_obj_is_kind_of(mrb, obj, mrb_class_get(mrb, "Method"))){
        size += os_memsize_of_method(mrb, obj);
      }
      break;
    }
    case MRB_TT_HASH: {
      size += mrb_objspace_page_slot_size() +
              mrb_os_memsize_of_hash_table(obj);
      break;
    }
    case MRB_TT_ARRAY: {
      mrb_int len = RARRAY_LEN(obj);
      /* Arrays that do not fit within an RArray perform a heap allocation
      *  storing an array of pointers to the original objects*/
      size += mrb_objspace_page_slot_size();
      if(len > MRB_ARY_EMBED_LEN_MAX) size += sizeof(mrb_value *) * len;
      break;
    }
    case MRB_TT_PROC: {
      struct RProc* proc = mrb_proc_ptr(obj);
      size += mrb_objspace_page_slot_size();
      size += MRB_ENV_LEN(proc->e.env) * sizeof(mrb_value);
      if(!MRB_PROC_CFUNC_P(proc)) size += os_memsize_of_irep(mrb, proc->body.irep);
      break;
    }
    case MRB_TT_DATA:
      size += mrb_objspace_page_slot_size();
      break;
#ifndef MRB_WITHOUT_FLOAT
    case MRB_TT_FLOAT:
#ifdef MRB_WORD_BOXING
      size += mrb_objspace_page_slot_size() +
              sizeof(struct RFloat);
#endif
      break;
#endif
    case MRB_TT_RANGE:
#ifndef MRB_RANGE_EMBED
      size += mrb_objspace_page_slot_size() +
              sizeof(struct mrb_range_edges);
#endif
      break;
    case MRB_TT_FIBER: {
      struct RFiber* f = (struct RFiber *)mrb_ptr(obj);
      ptrdiff_t stack_size = f->cxt->stend - f->cxt->stbase;
      ptrdiff_t ci_size = f->cxt->ciend - f->cxt->cibase;

      size += mrb_objspace_page_slot_size() +
        sizeof(struct RFiber) +
        sizeof(struct mrb_context) +
        sizeof(struct RProc *) * f->cxt->esize +
        sizeof(uint16_t *) * f->cxt->rsize +
        sizeof(mrb_value) * stack_size +
        sizeof(mrb_callinfo) * ci_size;
      break;
    }
    case MRB_TT_ISTRUCT:
      size += mrb_objspace_page_slot_size();
      break;
    /*  zero heap size types.
     *  immediate VM stack values, contained within mrb_state, or on C stack */
    case MRB_TT_TRUE:
    case MRB_TT_FALSE:
    case MRB_TT_FIXNUM:
    case MRB_TT_BREAK:
    case MRB_TT_CPTR:
    case MRB_TT_SYMBOL:
    case MRB_TT_FREE:
    case MRB_TT_UNDEF:
    case MRB_TT_ENV:
    /* never used, silences compiler warning
     * not having a default: clause lets the compiler tell us when there is a new
     * TT not accounted for */
    case MRB_TT_MAXDEFINE:
      break;
  }
  return size;
}

/*
 *  call-seq:
 *    ObjectSpace.memsize_of(obj, recurse: false) -> Numeric
 *
 *  Returns the amount of heap memory allocated for object in size_t units.
 *
 *  The return value depends on the definition of size_t on that platform,
 *  therefore the value is not comparable across platform types.
 *
 *  Immediate values such as integers, booleans, symbols and unboxed float numbers
 *  return 0. Additionally special objects which are small enough to fit inside an
 *  object pointer, termed embedded objects, will return the size of the object pointer.
 *  Strings and arrays below a compile-time defined size may be embedded.
 *
 *  Setting recurse: true descends into instance variables, array members,
 *  hash keys and hash values recursively, calculating the child objects and adding to
 *  the final sum. It avoids infinite recursion and over counting objects by
 *  internally tracking discovered object ids.
 *
 */

static mrb_value
os_memsize_of(mrb_state *mrb, mrb_value self)
{
  mrb_int total;
  mrb_value obj;

  mrb_get_args(mrb, "o", &obj);

  total = os_memsize_of_object(mrb, obj);
  return mrb_fixnum_value(total);
}

struct os_memsize_of_all_cb_data {
  mrb_int t;
  struct RClass *type;
};

static int
os_memsize_of_all_cb(mrb_state *mrb, struct RBasic *obj, void *d)
{
  struct os_memsize_of_all_cb_data *data = (struct os_memsize_of_all_cb_data *)d;
  switch (obj->tt) {
  case MRB_TT_FREE: case MRB_TT_ENV:
  case MRB_TT_BREAK: case MRB_TT_ICLASS:
    /* internal objects that should not be counted */
    return MRB_EACH_OBJ_OK;
  default:
    break;
  }
  /* skip Proc objects for methods */
  if (obj->c == NULL) return 0;
  if (data->type == NULL || mrb_obj_is_kind_of(mrb, mrb_obj_value(obj), data->type))
    data->t += os_memsize_of_object(mrb, mrb_obj_value(obj));
  return MRB_EACH_OBJ_OK;
}

/*
 *  call-seq:
 *    ObjectSpace.memsize_of_all([klass]) -> Numeric
 *
 *  Return consuming memory size of all living objects of type klass.
 *
 */

static mrb_value
os_memsize_of_all(mrb_state *mrb, mrb_value self)
{
  struct os_memsize_of_all_cb_data data = { 0 };
  mrb_get_args(mrb, "|c", &data.type);
  mrb_objspace_each_objects(mrb, os_memsize_of_all_cb, &data);
  return mrb_fixnum_value(data.t);
}

void
mrb_mruby_os_memsize_gem_init(mrb_state *mrb)
{
  struct RClass *os = mrb_module_get(mrb, "ObjectSpace");
  mrb_define_class_method(mrb, os, "memsize_of", os_memsize_of, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, os, "memsize_of_all", os_memsize_of_all, MRB_ARGS_OPT(1));
}

void
mrb_mruby_os_memsize_gem_final(mrb_state *mrb)
{
}
