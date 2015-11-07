#include <pebble.h>
#include "count_down.h"
#include "scheduler.h"
#include "selector.h"
#include "main.h"
  
static void init(void) {
  main_init();
  count_down_init();
  sched_init();
  selector_init();
}

static void deinit(void) {
  main_deinit();
  count_down_deinit();
  sched_deinit();
  selector_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();

  return 0;
}