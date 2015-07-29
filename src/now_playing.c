#include <pebble.h>
#include "now_playing.h"
#include "buttons.h"
#include "comm.h"
#define h_artist 40
#define h_track 72
#define spacer 2


static Layer *volume_layer;
static Layer *now_playing_layer;
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
  layer_destroy(volume_layer);
  layer_destroy(now_playing_layer);
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
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_button_selection);
  win_buttons_show();
}




static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, win_buttons_show);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 50, up_short_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 50, down_short_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, select_long_down_hanlder, select_long_up_hanlder);
}

static void drawText(GContext* ctx, GRect r, char* text, GFont* font, GColor bg, GColor fg) {
  graphics_context_set_fill_color(ctx, bg);
  graphics_context_set_text_color(ctx, fg);
  graphics_fill_rect(ctx, r,0, GCornerNone);
  GSize sz = graphics_text_layout_get_content_size(text, font, r, GTextOverflowModeWordWrap, GTextAlignmentCenter);
  int y = r.origin.y;
  int h = r.size.h;
  if (sz.h < r.size.h) {
    y+= (r.size.h - sz.h) / 2 - 1;
    h = sz.h;
  }
  GRect r1 = GRect(r.origin.x, y, r.size.w, h);
  graphics_draw_text(ctx, text, font, r1, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}
static void now_playing_layer_update_callback(Layer *me, GContext* ctx) {
  GFont* f1 = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont* f2 = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GRect bounds = layer_get_frame(me);
  int y = 0;
  GRect r = GRect(0, y, bounds.size.w, h_artist);
  drawText(ctx,r,now_playing_item_name,f1,conf_title_text_color_bg,conf_title_text_color_fg);
  y+=h_artist;
  char * txt;
  if (is_system_off) 
    txt = "System Is OFF";
  else if (strlen(now_playing_track)==0)
    txt = now_playing_station_location;
  else
    txt = now_playing_track;
  r = GRect(0, y, bounds.size.w, h_track);
  drawText(ctx,r,txt,f2,conf_text_color_bg,conf_text_color_fg);
  y+=h_track;
  if (is_system_off)
    txt = "";
  else
    txt = now_playing_album;
  r = GRect(0, y, bounds.size.w, h_artist);
  drawText(ctx,r,txt,f1,conf_text_color_bg,conf_text_color_fg);  
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
  #ifndef PBL_COLOR
  window_set_fullscreen(window,true);
  #endif
  Layer* root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  int y = 0;
  int w = bounds.size.w - ACTION_BAR_WIDTH;
  int h = h_artist * 2 + h_track;
  //now playing
  GRect r = GRect(0, y, w , h);
  y =  h + 5;
  now_playing_layer = layer_create(r);
  layer_set_update_proc(now_playing_layer, now_playing_layer_update_callback);
  layer_add_child(root, now_playing_layer);
  //Volume
  r = GRect(0, y, w, 10);
  volume_layer = layer_create(r);
  layer_set_update_proc(volume_layer, volume_layer_update_callback);
  layer_add_child(root, volume_layer);
  y+=10;

  //Action Bar
  icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_VOLUME_UP);
  icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_VOLUME_DOWN);
  icon_button_selection = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTONS);
  icon_button_shut_off = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_OFF);
  action_bar = action_bar_layer_create();
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_volume_up);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_button_selection);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_volume_down);
  action_bar_layer_set_background_color(action_bar,GColorWhite);
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
  refreshTimer = app_timer_register(1000, callBack, NULL);
}


void win_now_playing_refresh_data() {
  if (!screen_loaded)
    return;
  layer_mark_dirty(volume_layer);
  layer_mark_dirty(now_playing_layer);
}