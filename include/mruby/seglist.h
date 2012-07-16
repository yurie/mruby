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
    symbol_name key[MRB_SEGMENT_SIZE];
    mrb_value val[MRB_SEGMENT_SIZE];
    struct mrb_segment *next;
} mrb_segment;

typedef struct mrb_seglist
{
	mrb_segment *rootseg;
    int last_len;
} mrb_seglist;

mrb_value seglist_put_item(mrb_state *mrb, mrb_seglist seglist, mrb_sym sym, mrb_value *obj);
mrb_value seglist_clear(mrb_state* mrb, mrb_seglist seglist);
mrb_value seglist_get_item(mrb_state* mrb, mrb_seglist seglist, mrb_sym sym);
mrb_seglist *seglist_new(mrb_state* mrb);
mrb_value seglist_delete_item(mrb_state* mrb, mrb_seglist seglist, mrb_sym sym);
mrb_value seglist_get_all_keys(mrb_state *mrb, mrb_seglist seglist);


#if defined(__cplusplus)
}  /* extern "C" { */
#endif

#endif /* MRUBY_SEGLIST_H */
