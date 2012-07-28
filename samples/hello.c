#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/dump.h"
#include "mruby/string.h"
#include "mruby/proc.h"

void hello_irep(mrb_state *mrb);

void
mrb_init_hello(mrb_state *mrb)
{
  hello_irep(mrb);
}

#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/string.h"
#include "mruby/proc.h"

static mrb_code iseq_0[] = {
  0x01000048,
  0x018002c0,
  0x01000046,
  0x01000006,
  0x01800006,
  0x02400203,
  0x02800005,
  0x018000a0,
  0x02000005,
  0x010040a0,
  0x0000004a,
};

static mrb_code iseq_1[] = {
  0x02000026,
  0x02004001,
  0x02804001,
  0x03000005,
  0x020000b0,
  0x02804001,
  0x03000005,
  0x020000b0,
  0x02000029,
};

void
hello_irep(mrb_state *mrb)
{
  int n = mrb->irep_len;
  int idx = n;
  int ai;
  mrb_irep *irep;

  mrb_add_irep(mrb, idx+2);

  ai = mrb->arena_idx;
  irep = mrb->irep[idx] = mrb_malloc(mrb, sizeof(mrb_irep));
  irep->idx = idx++;
  irep->flags = 0 | MRB_ISEQ_NOFREE;
  irep->nlocals = 2;
  irep->nregs = 5;
  irep->ilen = 11;
  irep->iseq = iseq_0;
  irep->slen = 2;
  irep->syms = mrb_malloc(mrb, sizeof(mrb_sym)*2);
  irep->syms[0] = mrb_intern(mrb, "cube");
  irep->syms[1] = mrb_intern(mrb, "puts");
  irep->plen = 0;
  irep->pool = NULL;
  mrb->irep_len = idx;
  mrb->arena_idx = ai;

  ai = mrb->arena_idx;
  irep = mrb->irep[idx] = mrb_malloc(mrb, sizeof(mrb_irep));
  irep->idx = idx++;
  irep->flags = 0 | MRB_ISEQ_NOFREE;
  irep->nlocals = 4;
  irep->nregs = 6;
  irep->ilen = 9;
  irep->iseq = iseq_1;
  irep->slen = 1;
  irep->syms = mrb_malloc(mrb, sizeof(mrb_sym)*1);
  irep->syms[0] = mrb_intern(mrb, "*");
  irep->plen = 0;
  irep->pool = NULL;
  mrb->irep_len = idx;
  mrb->arena_idx = ai;

  mrb_run(mrb, mrb_proc_new(mrb, mrb->irep[n]), mrb_top_self(mrb));
}
