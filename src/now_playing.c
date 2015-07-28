#include <pebble.h>
#include "now_playing.h"
#include "buttons.h"
#include "comm.h"
#define h_artist 40
#define h_track 72
#define spacer 2


static TextLayer* txt_artist;
static TextLayer* txt_track;
static TextLayer* txt_album;
static TextLayer* txt_item_name;
static Layer *volume_layer;
static ActionBarLayer *action_bar;
static bool screen_loaded = false;
static AppTimer *refreshTimer;
static GBitmap* icon_volume_up;
static GBitmap* icon_volume_down;
static GBitmap* icon_button_selection;
static GBitmap* icon_button_shut_off;

static void callBack()  {
  read_now_playing();
  if (screen_loaded) {
    refreshTimer = app_timer_register(5000, callBack, NULL);
  }
}

static void onUnload(Window *window) {
  text_layer_destroy(txt_artist);
  text_layer_destroy(txt_track);
  text_layer_destroy(txt_album);
  text_layer_destroy(txt_item_name);
  layer_destroy(volume_layer);
  action_bar_layer_destroy(action_bar);
  gbitmap_destroy(icon_volume_up);
  gbitmap_destroy(icon_volume_down);
  gbitmap_destroy(icon_button_selection);
  gbitmap_destroy(icon_button_shut_off);
  screen_loaded = false;  
}

void up_short_click_handler(ClickRecognizerRef recognizer, void *context) {
  change_volume(1);
}
void down_short_click_handler(ClickRecognizerRef recognizer, void *context) {
  change_volume(-1);
}
void select_long_down_hanlder (ClickRecognizerRef recognizer, void *context) {
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_button_shut_off);
}
void select_long_up_hanlder (ClickRecognizerRef recognizer, void *context) {
  shut_off();
}




static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, win_buttons_show);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 50, up_short_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 50, down_short_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, select_long_down_hanlder, select_long_up_hanlder);
}

static void volume_layer_update_callback(Layer *me, GContext* ctx) {
  GRect bounds = layer_get_frame(volume_layer);
  GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, r);
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorOrange);
  #endif
  int w = (bounds.size.w - 2) * volume  / 100;
  r = GRect(1, 1, w, bounds.size.h-2);
  graphics_fill_rect(ctx, r, 0, GCornerNone);
}



void win_now_playing_show() {
  screen_loaded = true;
  Window* window = window_create();
  Layer* root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  int y = 0;
  int w = bounds.size.w - ACTION_BAR_WIDTH;
  //item name  
  GRect r = GRect(0, 0, w , h_artist);
  txt_item_name =  text_layer_create(r);
  text_layer_set_text_alignment(txt_item_name, GTextAlignmentCenter);
  text_layer_set_font(txt_item_name, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  #ifdef PBL_COLOR
    text_layer_set_background_color(txt_item_name,GColorImperialPurple);
    text_layer_set_text_color(txt_item_name, GColorWhite);
  #endif
  layer_add_child(root, text_layer_get_layer(txt_item_name));
  y+=r.size.h;
  //Track
  r = GRect(0, y, w, h_track);
  txt_track =  text_layer_create(r);
  text_layer_set_text_alignment(txt_track, GTextAlignmentCenter);
  text_layer_set_font(txt_track, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(root, text_layer_get_layer(txt_track));
  y+=r.size.h;
  //Album
  r = GRect(0, y, w, h_artist);
  txt_album =  text_layer_create(r);
  text_layer_set_text_alignment(txt_album, GTextAlignmentCenter);
  text_layer_set_font(txt_album, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(root, text_layer_get_layer(txt_album));
  y+=r.size.h;
  //Volume
  r = GRect(2, y, w-4, 10);
  volume_layer = layer_create(r);
  layer_set_update_proc(volume_layer, volume_layer_update_callback);
  layer_add_child(root, volume_layer);
  y+=10;

  //Action Bar
  //icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_VOL_UP);
  //icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_VOL_DOWN);
  //icon_button_selection = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PRESETS);
  //icon_button_shut_off = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_OFF);
  action_bar = action_bar_layer_create();
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_volume_up);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_button_selection);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_volume_down);
  #ifdef PBL_COLOR
    action_bar_layer_set_background_color(action_bar,GColorCeleste);
  #else
    action_bar_layer_set_background_color(action_bar,GColorWhite);
  #endif
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
  
  action_bar_layer_add_to_window(action_bar, window);  
  
  //Handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .unload = onUnload
  });
  //Show the window
  window_stack_pop_all(false);
  window_stack_push(window, false);
  read_volume();
  callBack();
}


void win_now_playing_refresh_data() {
  if (!screen_loaded)
    return;
  layer_mark_dirty(volume_layer);
  if (is_system_off) {
    //text_layer_set_text(txt_artist, "");
    text_layer_set_text(txt_track, "System Is Off");
    text_layer_set_text(txt_album, "");
    return;
  }
  if (strlen(now_playing_track)==0)
    text_layer_set_text(txt_track, now_playing_station_location);
  else
    text_layer_set_text(txt_track, now_playing_track);
  text_layer_set_text(txt_item_name,now_playing_item_name);
  text_layer_set_text(txt_album, now_playing_album);
}