/*
 * seglist.h
 *
 *  Created on: 2012/07/16
 *      Author: yuri
 */

#ifndef MRUBY_SEGLIST_H
#define MRUBY_SEGLIST_H

#if defined(__cplusplus)
extern "C" {
#endif

#define MRB_SEGMENT_SIZE 8

typedef struct mrb_segment
{
    mrb_sym key[MRB_SEGMENT_SIZE];
    mrb_value val[MRB_SEGMENT_SIZE];
    struct mrb_segment *next;
} mrb_segment;

typedef struct mrb_seglist
{
	mrb_segment *rootseg;
	uint32_t size;
    int last_len;
} mrb_seglist;

mrb_value seglist_inspect_obj(mrb_state *mrb, mrb_seglist *seglist, mrb_value str);

mrb_value seglist_put_item(mrb_state *mrb, mrb_seglist *seglist, mrb_sym sym, mrb_value value);
mrb_value seglist_clear(mrb_state* mrb, mrb_seglist *seglist);
mrb_value seglist_get_item(mrb_state* mrb, mrb_seglist *seglist, mrb_sym sym);
mrb_seglist *seglist_new(mrb_state* mrb);
mrb_value seglist_delete_item(mrb_state* mrb, mrb_seglist *seglist, mrb_sym sym);
mrb_value seglist_get_all_keys(mrb_state *mrb, mrb_seglist *seglist);
void seglist_gc_mark_value(mrb_state *mrb, mrb_seglist *seglist);
size_t seglist_size(mrb_state *mrb, mrb_seglist *seglist);
mrb_sym seglist_get_key_by_class_val(mrb_state *mrb, mrb_seglist *seglist, struct RClass *c);
mrb_seglist *seglist_copy(mrb_state *mrb, mrb_seglist *seglist);


#if defined(__cplusplus)
}  /* extern "C" { */
#endif

#endif /* MRUBY_SEGLIST_H */
