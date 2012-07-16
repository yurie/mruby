/*
 * seglist.c
 *
 *  Created on: 2012/07/16
 *      Author: yuri
 */

#include "mruby.h"
#include <string.h>


#include "mruby/seglist.h"
#include "mruby/string.h"

mrb_value seglist_find_and_put_item(mrb_state *mrb, mrb_seglist seglist,  mrb_sym sym, mrb_value *obj);

mrb_value seglist_put_item(mrb_state *mrb, mrb_seglist seglist, mrb_sym sym, mrb_value *obj)
{
	// XXX 失敗しないはず
	seglist_find_and_put_item(mrb, seglist,  sym, obj);
	return mrb_true_value();
}

mrb_value seglist_find_and_put_item(mrb_state *mrb, mrb_seglist seglist, mrb_sym sym, mrb_value *obj)
{
	mrb_segment *segment;
	mrb_segment *prev = NULL;
	int i;
	segment = seglist.rootseg;

	while (segment != NULL) {
		for (i=0; i<MRB_SEGMENT_SIZE; i++) {
			mrb_sym seg_key = segment->key[i];
			// 最後のセグメントでlast_lenよりも後の要素の場合
			if ((segment->next == NULL) && (i >= seglist.last_len)) {
				if (obj != NULL) {
					segment->key[i] = sym;
					segment->val[i] = *obj;
					seglist.last_len = i+1;
					return segment->val[i];
				} else {
					return mrb_undef_value();
				}
			}
			if (seg_key == sym) {
				if (obj != NULL) {
					segment->val[i] = *obj;
				}
				return segment->val[i];
			}
		}
		prev = segment;
		segment = segment->next;
	}


	// 見つからなかった
	if (obj) {
		segment = mrb_malloc(mrb, sizeof(mrb_segment));
		segment->next = NULL;
		segment->key[0] = sym;
		segment->val[0] = *obj;
		seglist.last_len = 1;
		if(prev != NULL){
			prev->next = segment;
		}else{
			seglist.rootseg = segment;
		}
		return mrb_true_value();
	} else {
		return mrb_undef_value();
	}
}
mrb_value seglist_clear(mrb_state* mrb, mrb_seglist seglist)
{
	return mrb_true_value();
}

// やっぱり見つからなかったときはundefを返して呼出し側の方でなんとかするようにする
mrb_value seglist_get_item(mrb_state* mrb, mrb_seglist seglist, mrb_sym sym)
{
	return seglist_find_and_put_item(mrb, seglist,  sym, NULL);
}

mrb_seglist *seglist_new(mrb_state *mrb)
{
	mrb_seglist *seglist;

	seglist = mrb_malloc(mrb, sizeof(mrb_seglist));
	seglist->rootseg =  NULL;
	seglist->last_len = 0;

	return seglist;
}

// XXX undefを代入するだけでよかったかも？？？
mrb_value seglist_delete_item(mrb_state *mrb, mrb_seglist seglist, mrb_sym sym)
{
	mrb_segment *segment;
	mrb_segment *prev = NULL;
	mrb_segment *matched_segment = NULL;
	int matched_idx = 0;
	mrb_value matched_value;
	int i;
	segment = seglist.rootseg;

	while (segment != NULL) {
		for (i=0; i<MRB_SEGMENT_SIZE; i++) {
			mrb_sym seg_key = segment->key[i];
			if (seg_key == sym) {
				matched_segment = segment;
				matched_idx = i;
				matched_value = segment->val[i];
			}

			// 最後のセグメントでlast_lenの要素の場合
			if ((segment->next == NULL) && (i == seglist.last_len-1)) {
				// 該当要素がなかったら
				if (!matched_segment) {
					return mrb_undef_value();
				} else {
					// matchしたところに最終要素を入れる
					matched_segment->key[matched_idx] = segment->key[i];
					matched_segment->val[matched_idx] = segment->val[i];

					// 最後の要素がセグメントの先頭だったとき
					if (i==0) {
						//そのセグメントを破棄して前セグメントのnextをNULLに
						if (prev) {
							prev->next = NULL;
							seglist.last_len = MRB_SEGMENT_SIZE;
						} else {
							seglist.rootseg = NULL;
							seglist.last_len = 0;
						}
						mrb_free(mrb, segment);
					} else {
						seglist.last_len = i;
						segment->key[i] = NULL;
						segment->val[i] = NULL;
					}
					return matched_value;
				}
			}
		}
		prev = segment;
		segment = segment->next;
	}

	// rootsegが空のとき
	return mrb_undef_value();
}

mrb_value seglist_get_all_keys(mrb_state *mrb, mrb_seglist seglist)
{
	mrb_segment *segment;
	mrb_value ary;
	int i;
	int len;
	const char *p;

	segment = seglist.rootseg;
	ary = mrb_ary_new(mrb);

	while (segment != NULL) {
		for (i=0; i<MRB_SEGMENT_SIZE; i++) {
			mrb_sym seg_key = segment->key[i];
			mrb_value seg_val = segment->val[i];
	        p = mrb_sym2name_len(mrb, seg_key, &len);
	        if (len > 1 && *p == '@') {
	        	if (mrb_type(seg_val) != MRB_TT_UNDEF) {
	        		mrb_ary_push(mrb, ary, mrb_str_new(mrb, p, len));
	         	 }
	        }
			// 最後のセグメントでlast_lenよりも後の要素の場合
			if ((segment->next == NULL) && (i >= seglist.last_len)) {
				return ary;
			}
		}
		segment = segment->next;
	}
	return ary;

}

