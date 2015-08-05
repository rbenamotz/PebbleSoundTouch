#include <pebble.h>
#include "buttons.h"

static TextLayer* txt_button[6];
static TextLayer* txt_info;
static GBitmap* bmp_big_off_button;
static BitmapLayer* bmp_source;
static BitmapLayer* bmp_big_off_button_layer;
static char button[6][2];
static bool screen_loaded = false;



static void up_short_click_handler(ClickRecognizerRef recognizer, void *context) {
  selected_channel--;
  if (selected_channel<0)
    selected_channel = 5;
  win_buttons_refresh_data();
}
static void down_short_click_handler(ClickRecognizerRef recognizer, void *context) {
  selected_channel++;
  if (selected_channel>5)
    selected_channel = 0;
  win_buttons_refresh_data();
}
static void select_short_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
  push_button(selected_channel);
  window_stack_pop(true);
}

static void select_long_down_hanlder (ClickRecognizerRef recognizer, void *context) {
  layer_set_hidden(bitmap_layer_get_layer(bmp_big_off_button_layer),false);
}
static void select_long_up_hanlder (ClickRecognizerRef recognizer, void *context) {
  shut_off();
  layer_set_hidden(bitmap_layer_get_layer(bmp_big_off_button_layer),true);
  window_stack_pop(true);
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,up_short_click_handler );
  window_single_click_subscribe(BUTTON_ID_SELECT, select_short_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_short_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_down_hanlder, select_long_up_hanlder);
}
static void onUnload(Window* window) {
  for (int i=0; i<6; i++)
    text_layer_destroy(txt_button[i]);
  text_layer_destroy(txt_info);
  bitmap_layer_destroy(bmp_source);
  bitmap_layer_destroy(bmp_big_off_button_layer);
  gbitmap_destroy(bmp_big_off_button);
  screen_loaded = false;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "buttons.onUnload");
}



void win_buttons_show() {
  screen_loaded = true;
  for (int i=0; i<6; i++)
    snprintf(button[i], sizeof(button[i]), "%d", i+1);
  Window* window = window_create();
  #ifndef PBL_COLOR
  window_set_fullscreen(window,true);
  #endif
  Layer* root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  int w = 42;
  int wd = 4;
  int x = wd;
  int h = 52;
  int y = 5;
  //buttons
  GRect r = GRect(x, y, w, h);
  #ifdef PBL_COLOR
  GFont font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  #else
  GFont* font = fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS);
  #endif
  
  for (int i=0; i<6; i++) {
    txt_button[i] = text_layer_create(r);
    text_layer_set_text_alignment(txt_button[i], GTextAlignmentCenter);
    text_layer_set_text(txt_button[i], button[i]);
    text_layer_set_font(txt_button[i], font);
    layer_add_child(root, text_layer_get_layer(txt_button[i]));
    r.origin.x+=w + wd;
    if (r.origin.x>=144-w) {
      r.origin.x = wd;
      y+=h+5;
      r.origin.y = y;
    }
    if (strcmp(now_playing_item_name, presets_description[i])==0)
      selected_channel = i;
  }
  y = h * 2 + 10 * 2;
  //bitmap source
  r = GRect(0, y , 32, 32);
  bmp_source = bitmap_layer_create(r);
  layer_add_child(root, bitmap_layer_get_layer(bmp_source));
  //text
  r = GRect(32, y, bounds.size.w - 32, bounds.size.h - y);
  txt_info =  text_layer_create(r);
  text_layer_set_text_alignment(txt_info, GTextAlignmentCenter);
  text_layer_set_font(txt_info, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(root, text_layer_get_layer(txt_info));
  //Big Off Button
  bmp_big_off_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BIG_OFF);
  bmp_big_off_button_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(bmp_big_off_button_layer,bmp_big_off_button);
  bitmap_layer_set_alignment(bmp_big_off_button_layer,GAlignCenter);
  layer_set_hidden(bitmap_layer_get_layer(bmp_big_off_button_layer),true);
  layer_add_child(root, bitmap_layer_get_layer(bmp_big_off_button_layer));
  
  //Refresh
  win_buttons_refresh_data();
  //Handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .unload = onUnload
  });
  window_set_click_config_provider(window, click_config_provider);
  //Show the window
  //window_stack_pop_all(false);
  window_stack_push(window, true);
  read_presets();
}


void win_buttons_refresh_data() {
  if (!screen_loaded)
    return;
  for (int i=0; i<6; i++) {
    if (i==selected_channel) {
      text_layer_set_text_color(txt_button[i], conf_title_text_color_fg);
      text_layer_set_background_color(txt_button[i], conf_title_text_color_bg);
    }
    else {
      text_layer_set_text_color(txt_button[i], conf_title_text_color_bg);
      text_layer_set_background_color(txt_button[i], conf_title_text_color_fg);
    }
  }
  text_layer_set_text(txt_info, presets_description[selected_channel]);
  GBitmap *channel_icon = get_image_by_source(presets_source[selected_channel]);
  bitmap_layer_set_bitmap(bmp_source,channel_icon);
  
}