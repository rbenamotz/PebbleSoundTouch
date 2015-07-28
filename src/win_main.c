#include <pebble.h>
#include "win_main.h"
#include "model.h"
#include "win_ip_select.h"
#include "comm.h"
#include "buttons.h"
#include "now_playing.h"
  
static TextLayer* txt_message;
static char message[120];
static AppTimer *jsInitTimer;

static void callBack()  {
  if (!is_js_ready) {
    jsInitTimer = app_timer_register(50, callBack, NULL);
    return;
  }
  validate_ip();
}


static void onUnload(Window *window) {
  text_layer_destroy(txt_message);
}


void show_win_main() {
  snprintf(message, sizeof(message), "Validating IP address\n%d.%d.%d.%d\n Please wait.", speaker_ip[0],speaker_ip[1],speaker_ip[2],speaker_ip[3]);
  Window* window = window_create();
  Layer* root = window_get_root_layer(window);
  GRect root_bounds = layer_get_bounds(root);
  txt_message = text_layer_create(root_bounds);
  text_layer_set_font(txt_message, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(txt_message, GTextAlignmentCenter);
  text_layer_set_text(txt_message,message);
  layer_add_child(root, text_layer_get_layer(txt_message));
  //Handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .unload = onUnload
  });
  window_stack_push(window, false);
  jsInitTimer = app_timer_register(50, callBack, NULL);
}

void validate_ip_completed() {
  if (is_ip_validated) {
    win_now_playing_show();
    return;
  }
  show_win_ip();
}

