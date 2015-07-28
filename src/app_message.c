#include <pebble.h>
#include "model.h"
#include "comm.h"
#include "win_main.h"
#include "win_ip_select.h"

  
void init(void) {
  init_model();
  comm_init();
  if (!is_ip_set)
    show_win_ip();
  else
    show_win_main();
}

void deinit(void) {
	app_message_deregister_callbacks();
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}