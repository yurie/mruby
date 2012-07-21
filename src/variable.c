/*
** variable.c - mruby variables
**
** See Copyright Notice in mruby.h
*/

#include "mruby.h"
#include "mruby/class.h"
#include "mruby/variable.h"
#include "error.h"
#include "mruby/array.h"
#include "mruby/seglist.h"

#ifdef ENABLE_REGEXP
#include "re.h"
#endif

static void
mark_tbl(mrb_state *mrb, mrb_seglist *iv)
{
  if (!iv) return;
  seglist_gc_mark_value(mrb, iv);
}

void
mrb_gc_mark_gv(mrb_state *mrb)
{
  mark_tbl(mrb, mrb->globals);
}

void
mrb_gc_free_gv(mrb_state *mrb)
{
	seglist_clear(mrb, mrb->globals);
}

void
mrb_gc_mark_iv(mrb_state *mrb, struct RObject *obj)
{
  mark_tbl(mrb, obj->iv);
}

size_t
mrb_gc_mark_iv_size(mrb_state *mrb, struct RObject *obj)
{
  mrb_seglist *iv = obj->iv;

  if (!iv) return 0;
  return seglist_size(mrb, iv);
}

void
mrb_gc_free_iv(mrb_state *mrb, struct RObject *obj)
{
	seglist_clear(mrb, obj->iv);
}

mrb_value
mrb_vm_special_get(mrb_state *mrb, mrb_sym i)
{
  return mrb_fixnum_value(0);
}

void
mrb_vm_special_set(mrb_state *mrb, mrb_sym i, mrb_value v)
{
}

static mrb_value
ivget(mrb_state *mrb, mrb_seglist *iv, mrb_sym sym)
{
  mrb_value obj = seglist_get_item(mrb, iv, sym);
	if (mrb_undef_p(obj)) {
		return mrb_nil_value();
	} else {
		return obj;
	}

}

mrb_value
mrb_obj_iv_get(mrb_state *mrb, struct RObject *obj, mrb_sym sym)
{
  if (!obj->iv) {
    return mrb_nil_value();
  }
  return ivget(mrb, obj->iv, sym);
}

static int
obj_iv_p(mrb_value obj)
{
  switch (mrb_type(obj)) {
    case MRB_TT_OBJECT:
    case MRB_TT_CLASS:
    case MRB_TT_MODULE:
    case MRB_TT_HASH:
    case MRB_TT_DATA:
      return TRUE;
    default:
      return FALSE;
  }
}

mrb_value
mrb_iv_get(mrb_state *mrb, mrb_value obj, mrb_sym sym)
{
  if (obj_iv_p(obj)) {
    return mrb_obj_iv_get(mrb, mrb_obj_ptr(obj), sym);
  }
  return mrb_nil_value();
}

static void
ivset(mrb_state *mrb, mrb_seglist *iv, mrb_sym sym, mrb_value v)
{
  seglist_put_item(mrb, iv, sym, v);
}

void
mrb_obj_iv_set(mrb_state *mrb, struct RObject *obj, mrb_sym sym, mrb_value v)
{
	mrb_seglist *iv;
	if (!obj->iv) {
		iv = obj->iv = seglist_new(mrb);
	} else {
		iv = obj->iv;
	}
	mrb_write_barrier(mrb, (struct RBasic*)obj);
	ivset(mrb, iv, sym, v);
}

void
mrb_iv_set(mrb_state *mrb, mrb_value obj, mrb_sym sym, mrb_value v) /* mrb_ivar_set */
{
  if (obj_iv_p(obj)) {
    mrb_obj_iv_set(mrb, mrb_obj_ptr(obj), sym, v);
  }
}

mrb_value
mrb_iv_remove(mrb_state *mrb, mrb_value obj, mrb_sym sym)
{
  if (obj_iv_p(obj)) {
    mrb_seglist *iv = mrb_obj_ptr(obj)->iv;
    return seglist_delete_item(mrb, iv, sym);
  }
  return mrb_undef_value();
}

mrb_value
mrb_vm_iv_get(mrb_state *mrb, mrb_sym sym)
{
  /* get self */
  return mrb_iv_get(mrb, mrb->stack[0], sym);
}

void
mrb_vm_iv_set(mrb_state *mrb, mrb_sym sym, mrb_value v)
{
  /* get self */
  mrb_iv_set(mrb, mrb->stack[0], sym, v);
}

/* 15.3.1.3.23 */
/*
 *  call-seq:
 *     obj.instance_variables    -> array
 *
 *  Returns an array of instance variable names for the receiver. Note
 *  that simply defining an accessor does not create the corresponding
 *  instance variable.
 *
 *     class Fred
 *       attr_accessor :a1
 *       def initialize
 *         @iv = 3
 *       end
 *     end
 *     Fred.new.instance_variables   #=> [:@iv]
 */
mrb_value
mrb_obj_instance_variables(mrb_state *mrb, mrb_value self)
{
  mrb_seglist *iv;

  if (obj_iv_p(self)) {
    iv = ROBJECT_IVPTR(self);
    return seglist_get_all_keys(mrb, iv);
  }
  return mrb_ary_new(mrb);
}

mrb_value
mrb_vm_cv_get(mrb_state *mrb, mrb_sym sym)
{
  struct RClass *c = mrb->ci->target_class;
  mrb_value v;

  while (c) {
    if (c->iv) {
    	v = seglist_get_item(mrb,c->iv,sym);
    	if (!mrb_undef_p(v)) {
    		return v;
    	}
    }
    c = c->super;
  }
  return mrb_nil_value();
}

void
mrb_vm_cv_set(mrb_state *mrb, mrb_sym sym, mrb_value v)
{
  struct RClass *c = mrb->ci->target_class;
  mrb_seglist *iv;
  mrb_value obj;

  while (c) {
    if (c->iv) {
      iv = c->iv;
      obj = seglist_get_item(mrb, iv, sym);
      if (!mrb_undef_p(obj)) {
    	  seglist_put_item(mrb, iv, sym, v);
    	  return;
      }
    }
    c = c->super;
  }
  c = mrb->ci->target_class;
  iv = c->iv;
  if (!iv) {
    c->iv = iv = seglist_new(mrb);
  }
  seglist_put_item(mrb, iv, sym, v);
}

int
mrb_const_defined(mrb_state *mrb, mrb_value mod, mrb_sym sym)
{
  struct RClass *m = mrb_class_ptr(mod);
  mrb_seglist *iv = m->iv;
  mrb_value obj;

  if (!iv) return 0;
  obj = seglist_get_item(mrb, iv, sym);
  if (!mrb_undef_p(obj)) {
    return 1;
  }
  return 0;
}

static void
mod_const_check(mrb_state *mrb, mrb_value mod)
{
  switch (mod.tt) {
  case MRB_TT_CLASS:
  case MRB_TT_MODULE:
    break;
  default:
    mrb_raise(mrb, E_TYPE_ERROR, "constant look-up for non class/module");
    break;
  }
}

static mrb_value
const_get(mrb_state *mrb, struct RClass *base, mrb_sym sym)
{
  struct RClass *c = base;
  mrb_seglist *iv;
  mrb_value obj;
  mrb_sym cm = mrb_intern(mrb, "const_missing");

 L_RETRY:
  while (c) {
    if (c->iv) {
      iv = c->iv;
      obj = seglist_get_item(mrb, iv, sym);
      if (!mrb_undef_p(obj)) {
    	  return obj;
      }

      if (mrb_respond_to(mrb, mrb_obj_value(c), cm)) {
        mrb_value argv = mrb_symbol_value(sym);
        return mrb_funcall_argv(mrb, mrb_obj_value(c), "const_missing", 1, &argv);
      }
    }
    c = c->super;
  }

  if (base->tt == MRB_TT_MODULE) {
    c = base = mrb->object_class;
    goto L_RETRY;
  }
  /*
  mrb_raise(mrb, E_NAME_ERROR, "uninitialized constant %s",
            mrb_sym2name(mrb, sym));
  */
  mrb_raise(mrb, E_NAME_ERROR, "uninitialized constant %d",
            sym);
  /* not reached */
  return mrb_nil_value();
}

mrb_value
mrb_const_get(mrb_state *mrb, mrb_value mod, mrb_sym sym)
{
  mod_const_check(mrb, mod);
  return const_get(mrb, mrb_class_ptr(mod), sym);
}

mrb_value
mrb_vm_const_get(mrb_state *mrb, mrb_sym sym)
{
  return const_get(mrb, mrb->ci->target_class, sym);
}

void
mrb_const_set(mrb_state *mrb, mrb_value mod, mrb_sym sym, mrb_value v)
{
  mod_const_check(mrb, mod);
  mrb_iv_set(mrb, mod, sym, v);
}

void
mrb_vm_const_set(mrb_state *mrb, mrb_sym sym, mrb_value v)
{
  mrb_obj_iv_set(mrb, (struct RObject*)mrb->ci->target_class, sym, v);
}

void
mrb_define_const(mrb_state *mrb, struct RClass *mod, const char *name, mrb_value v)
{
  mrb_obj_iv_set(mrb, (struct RObject*)mod, mrb_intern(mrb, name), v);
}

void
mrb_define_global_const(mrb_state *mrb, const char *name, mrb_value val)
{
  mrb_define_const(mrb, mrb->object_class, name, val);
}

mrb_value
mrb_gv_get(mrb_state *mrb, mrb_sym sym)
{
  if (!mrb->globals) {
    return mrb_nil_value();
  }
  return ivget(mrb, mrb->globals, sym);
}

void
mrb_gv_set(mrb_state *mrb, mrb_sym sym, mrb_value v)
{
  mrb_seglist *iv;

  if (!mrb->globals) {
    mrb->globals = seglist_new(mrb);
  }
  iv = mrb->globals;
  ivset(mrb, iv, sym, v);
}

/* 15.3.1.2.4  */
/* 15.3.1.3.14 */
/*
 *  call-seq:
 *     global_variables    -> array
 *
 *  Returns an array of the names of global variables.
 *
 *     global_variables.grep /std/   #=> [:$stdin, :$stdout, :$stderr]
 */
mrb_value
mrb_f_global_variables(mrb_state *mrb, mrb_value self)
{
  char buf[3];
  mrb_seglist *iv = mrb->globals;
  mrb_value ary;
  int i;

  if (iv) {
	  ary = seglist_get_all_keys(mrb, iv);
  }
  buf[0] = '$';
  buf[2] = 0;
  for (i = 1; i <= 9; ++i) {
      buf[1] = (char)(i + '0');
      mrb_ary_push(mrb, ary, mrb_symbol_value(mrb_intern(mrb, buf)));
  }
  return ary;
}


int
mrb_const_defined_at(mrb_state *mrb, struct RClass *klass, mrb_sym id)
{
	  struct RClass * c;
	  mrb_value obj;

	  c = klass;

	  while (c) {
	    if (c->iv) {
	    	obj = seglist_get_item(mrb, c->iv, id);
	    	if (!mrb_undef_p(obj)) {
	    		return (int)1/*Qtrue*/;
	    	}
	    }
	    if (klass != mrb->object_class) {
	    	break;
	    }
	    c = c->super;
	  }
	  return (int)0/*Qfalse*/;
}

mrb_value
mrb_attr_get(mrb_state *mrb, mrb_value obj, mrb_sym id)
{
  //return ivar_get(obj, id, FALSE);
  return mrb_iv_get(mrb, obj, id);
}

struct RClass *
mrb_class_obj_get(mrb_state *mrb, const char *name)
{
  mrb_value mod = mrb_obj_value(mrb->object_class);
  mrb_sym sym = mrb_intern(mrb, name);

  return mrb_class_ptr(mrb_const_get(mrb, mod, sym));
}

