/*
 * sample program for mrubyvm
 */

#include <string.h>

#include <mruby.h>
#include <mruby/proc.h>
#include <mruby/data.h>
#include <mruby/compile.h>

void
mrb_init_hello(mrb_state *);

int
main(void)
{
  mrb_state *mrb;

  /* new interpreter instance */
  mrb = mrb_open();
  if (mrb == NULL) {
    fprintf(stderr, "Invalid mrb_state, exiting other driver");
    return EXIT_FAILURE;
  }

  mrb_init_hello(mrb);
  mrb_close(mrb);

  return EXIT_SUCCESS;
}
