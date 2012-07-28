#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/dump.h"
#include "mruby/string.h"
#include "mruby/proc.h"

#ifdef USE_SYSLOG
extern void     syslog(int prio, const char *format, ...);
#endif

void hello_irep(mrb_state *mrb);

mrb_value
mrb_kernel_syslog(mrb_state *mrb, mrb_value self)
{
  mrb_value str;
  mrb_int i;

  mrb_get_args(mrb, "io", &i, &str);
  struct RString *s = mrb_str_ptr(str);
  int level = (int)i;
#ifdef USE_SYSLOG
  syslog(level, "%s", s->ptr);
#else
  mrb_p(mrb, str);
#endif /* USE_SYSLOG */
  return mrb_true_value();
}

void
mrb_init_syslog(mrb_state *mrb)
{
  struct RClass *krn;

  krn = mrb->kernel_module;
  mrb_define_method(mrb, krn, "syslog", mrb_kernel_syslog, ARGS_REQ(2));
}

void
mrb_init_hello(mrb_state *mrb)
{
  mrb_init_syslog(mrb);
  hello_irep(mrb);
}

