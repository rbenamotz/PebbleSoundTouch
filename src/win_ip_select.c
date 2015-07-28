#include <pebble.h>
#include "model.h"
#include "win_ip_select.h"
#include "now_playing.h"
#include "comm.h"
  
static TextLayer* txt_header;
static TextLayer* txt_instructions;
static TextLayer* txt_submit;
static TextLayer* txt_ip_part[4];
static TextLayer* txt_ip_dot[3];
static char dot[2];
static char ip_text[4][4];
static char* header = "IP Address";
static char* submit = "(Long click when done)";
static char* instructions = "Please enter the IP address of your Bose SoundTouch Speaker";
static int current_selection;



static void up_short_click_handler(ClickRecognizerRef recognizer, void *context) {
  change_ip_address(current_selection, 1);
  win_ip_refresh_data();
}
static void down_short_click_handler(ClickRecognizerRef recognizer, void *context) {
  change_ip_address(current_selection, -1);
  win_ip_refresh_data();
}
static void select_short_click_handler(ClickRecognizerRef recognizer, void *context) {
  current_selection++;
  if (current_selection>3)
    current_selection = 0;
  win_ip_refresh_data();
}
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
  persist_data();
  set_ip();
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,up_short_click_handler );
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 50, up_short_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_short_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_short_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 50, down_short_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

static void onUnload(Window *window) {
  for (int i=0; i<3; i++)
    text_layer_destroy(txt_ip_dot[i]);
  for (int i=0; i<4; i++)
    text_layer_destroy(txt_ip_part[i]);
  text_layer_destroy(txt_header);
  text_layer_destroy(txt_instructions);
  text_layer_destroy(txt_submit);
}


void show_win_ip() {
  dot[0] = '.';
  dot[1] = '\0';
  Window* window = window_create();
  Layer* root = window_get_root_layer(window);
  GRect root_bounds = layer_get_bounds(root);
  int y = 0;
  int w = 31;
  int wd = 5;
  int h = 24;
  //header
  GRect r = GRect(0,y,  root_bounds.size.w,24);
  txt_header = text_layer_create(r);
  text_layer_set_font(txt_header, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(txt_header, GTextAlignmentCenter);
  text_layer_set_text(txt_header,header);
  layer_add_child(root, text_layer_get_layer(txt_header));
  y+=24 + 5;
  //Instructions
  r = GRect(0,y,  root_bounds.size.w,24*3);
  txt_instructions = text_layer_create(r);
  text_layer_set_font(txt_instructions, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(txt_instructions, GTextAlignmentLeft);
  text_layer_set_text(txt_instructions,instructions);
  layer_add_child(root, text_layer_get_layer(txt_instructions));
  y+=18*3 + 10;
  //IP Address
  current_selection = 3;
  r = GRect(0, y, w, h);
  GFont* font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  for (int i=0; i<4; i++) {
    txt_ip_part[i] = text_layer_create(r);
    text_layer_set_text_alignment(txt_ip_part[i], GTextAlignmentCenter);
    text_layer_set_font(txt_ip_part[i], font);
    layer_add_child(root, text_layer_get_layer(txt_ip_part[i]));
    if (i<3) {
      GRect rDot = GRect(r.origin.x + w, y, wd, h);
      txt_ip_dot[i] = text_layer_create(rDot);
      text_layer_set_text_alignment(txt_ip_dot[i], GTextAlignmentCenter);
      text_layer_set_text(txt_ip_dot[i], dot);
      text_layer_set_font(txt_ip_dot[i], font);
      layer_add_child(root, text_layer_get_layer(txt_ip_dot[i]));
    }
    r.origin.x+=w + wd;
  }
  y+=24 + 5;
  //Submit
  r = GRect(0,y,  root_bounds.size.w,24);
  txt_submit = text_layer_create(r);
  text_layer_set_font(txt_submit, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(txt_submit, GTextAlignmentCenter);
  text_layer_set_text(txt_submit,submit);
  layer_add_child(root, text_layer_get_layer(txt_submit));

  win_ip_refresh_data();
  //Handlers
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .unload = onUnload
  });
  window_stack_pop_all(false);
  window_stack_push(window, false);
}


void win_ip_refresh_data() {
  for (int i=0; i<4; i++) {
    snprintf(ip_text[i], sizeof(ip_text[i]), "%d", speaker_ip[i]);
    if (i==current_selection) {
      text_layer_set_text_color(txt_ip_part[i], conf_text_color_bg);
      text_layer_set_background_color(txt_ip_part[i], conf_text_color_fg);
    }
    else {
      text_layer_set_text_color(txt_ip_part[i], conf_text_color_fg);
      text_layer_set_background_color(txt_ip_part[i], conf_text_color_bg);
    }
    text_layer_set_text(txt_ip_part[i],ip_text[i]);
  }
}

void set_ip_completed() {
  if (is_ip_validated) {
    win_now_playing_show();
    return;
  }
  vibes_double_pulse();
}