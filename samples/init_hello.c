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

