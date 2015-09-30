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
static bool should_ignore_ip_validation = false;


static void callBack()  {
  if (!is_js_ready) {
    jsInitTimer = app_timer_register(100, callBack, NULL);
    return;
  }
  validate_ip();
  jsInitTimer = app_timer_register(2000, callBack, NULL);
}


static void onUnload(Window *window) {
  text_layer_destroy(txt_message);
}

static void manulay_go_to_config() {
  app_timer_cancel(jsInitTimer);
  should_ignore_ip_validation = true;
  show_win_ip();
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,manulay_go_to_config);
  window_single_click_subscribe(BUTTON_ID_SELECT,manulay_go_to_config);
  window_single_click_subscribe(BUTTON_ID_DOWN,manulay_go_to_config);
}



void show_win_main() {
  snprintf(message, sizeof(message), "Validating config.\n Please wait or click any button for IP config");
  Window* window = window_create();
  #ifndef PBL_COLOR
  window_set_fullscreen(window,true);
  #endif  
  Layer* root = window_get_root_layer(window);
  GRect root_bounds = layer_get_bounds(root);
  txt_message = text_layer_create(root_bounds);
  text_layer_set_font(txt_message, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(txt_message, GTextAlignmentLeft);
  text_layer_set_text(txt_message,message);
  layer_add_child(root, text_layer_get_layer(txt_message));
  //Handlers
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .unload = onUnload
  });
  window_stack_push(window, false);
  jsInitTimer = app_timer_register(50, callBack, NULL);
}

void validate_ip_completed() {
  if (should_ignore_ip_validation)
    return;
  app_timer_cancel(jsInitTimer);
  if (is_ip_validated) {
    win_now_playing_show();
    return;
  }
  should_ignore_ip_validation = true;
  show_win_ip();
}

