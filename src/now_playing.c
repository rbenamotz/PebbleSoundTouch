#include <pebble.h>
#include "now_playing.h"
#include "buttons.h"
#include "comm.h"
#define h_source 32
#define h_track 112
#define spacer 2


static Layer *now_playing_time_layer;
static Layer *now_playing_layer;
static ActionBarLayer *action_bar;
static bool screen_loaded = false;
static GBitmap* icon_volume_up;
static GBitmap* icon_volume_down;
static GBitmap* icon_button_selection;
static GBitmap* icon_button_shut_off;
static GBitmap* icon_skip_back;
static GBitmap* icon_skip_forward;
static GBitmap* icon_play;
static GBitmap* icon_pause;
static bool shouldShowVolume = false;
static AppTimer *volumeHideTimer = NULL;
static AppTimer *nextVolumeChangeTimer = NULL;
//static AppTimer *refreshTimer;
static int volume_delta = 0;
static bool should_update_select_button = true;
int volume_change_counter;

/*
static void callBack()  {
  return;
  read_now_playing();
  refreshTimer = app_timer_register(5000, callBack, NULL);
}
*/

static void onUnload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "now_playing.onUnload");
  layer_destroy(now_playing_time_layer);
  layer_destroy(now_playing_layer);
  action_bar_layer_destroy(action_bar);
  gbitmap_destroy(icon_volume_up);
  gbitmap_destroy(icon_volume_down);
  gbitmap_destroy(icon_button_selection);
  gbitmap_destroy(icon_button_shut_off);
  gbitmap_destroy(icon_skip_back);
  gbitmap_destroy(icon_skip_forward);
  gbitmap_destroy(icon_play);
  gbitmap_destroy(icon_pause);
  screen_loaded = false;  
}

static void on_appear(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "now_playing.on_appear");
  read_now_playing();
  //refreshTimer = app_timer_register(5000, callBack, NULL); 
}
static void on_disappear(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "now_playing.on_disappear");
  //app_timer_cancel(refreshTimer);
}

void set_action_bar_icons() {
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_skip_back);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_skip_forward);
}

void hideVolumeLayer() {
  shouldShowVolume = false;
  layer_mark_dirty(now_playing_layer);
  volumeHideTimer = NULL;
  set_action_bar_icons();
}



static void handle_volume_change() {
  if (!shouldShowVolume) {
    //first time function called. Need to change icons in the action bar
    //app_timer_cancel(refreshTimer);
    flag_should_ignore_volume_reading = true;
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_volume_up);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_volume_down);
  }
  shouldShowVolume = true;
  volume_change_counter++;
  int newVolume = volume + volume_delta;
  if (newVolume<100 && newVolume>=0) {
    volume = newVolume;
    layer_mark_dirty(now_playing_layer);
  }
  if (volume_change_counter%3==0) {
    change_volume();
  }
  nextVolumeChangeTimer = app_timer_register(50, handle_volume_change,NULL);
}

void up_long_down_hanlder(ClickRecognizerRef recognizer, void *context) {
  volume_delta = 1;
  volume_change_counter = 0;
  handle_volume_change();
}
void down_long_down_hanlder(ClickRecognizerRef recognizer, void *context) {
  volume_delta = -1;
  volume_change_counter = 0;
  handle_volume_change();
}

void up_down_long_up_handler(ClickRecognizerRef recognizer, void *context) {
  app_timer_cancel(nextVolumeChangeTimer);
  change_volume();
  if (volumeHideTimer==NULL)
    volumeHideTimer = app_timer_register(1000, hideVolumeLayer,NULL);
  else
    app_timer_reschedule(volumeHideTimer, 1000);
  flag_should_ignore_volume_reading = false;
  //refreshTimer = app_timer_register(500, callBack, NULL);
}
static void select_long_down_hanlder (ClickRecognizerRef recognizer, void *context) {
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_button_selection);
  should_update_select_button = false;
}
static void select_long_up_hanlder (ClickRecognizerRef recognizer, void *context) {
  should_update_select_button = true;
  win_now_playing_refresh_data();
  win_buttons_show();
}




static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, prev_track);
  window_long_click_subscribe(BUTTON_ID_UP, 0, up_long_down_hanlder, up_down_long_up_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, play_pause);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_down_hanlder, select_long_up_hanlder);
  window_single_click_subscribe(BUTTON_ID_DOWN, next_track);
  window_long_click_subscribe(BUTTON_ID_DOWN, 0, down_long_down_hanlder, up_down_long_up_handler);
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
  graphics_draw_text(ctx, text, font, r1, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}


void draw_volume_as_circle (GContext* ctx, GRect r)
{
  graphics_context_set_stroke_color(ctx, conf_stroke_color);
  graphics_context_set_fill_color(ctx,conf_stroke_color);  
  graphics_context_set_text_color(ctx, conf_stroke_color);
  GPoint center = GPoint(r.origin.x + r.size.w / 2, r.origin.y + r.size.h / 2);
  GPoint perimiter;
  int radius = (r.size.w - 20) / 2;
  int angle0 = 0x10000 * (volume / 100.0);
  int step = 0x10000 / 20;
  for (int angle = 0; angle < 0x10000; angle+=step) 
  {
    perimiter.x = radius * sin_lookup(angle) / TRIG_MAX_RATIO + center.x;
    perimiter.y = radius * -cos_lookup (angle) / TRIG_MAX_RATIO + center.y;
    if (angle<=angle0)
      graphics_fill_circle(ctx, perimiter, 4);
    else
      graphics_fill_circle(ctx, perimiter, 2);
  }
  char volumeAsText[3];
  snprintf(volumeAsText, sizeof(volumeAsText), "%i", volume);
  #ifdef PBL_COLOR
  GFont font = fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS);
  #else
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  #endif
  GSize sz = graphics_text_layout_get_content_size(volumeAsText, font, r, GTextOverflowModeFill, GTextAlignmentCenter);
  int y = r.origin.y + (r.size.h - sz.h) / 2 - 1;
  int h = sz.h;
  GRect r1 = GRect(r.origin.x, y, r.size.w, h);
  graphics_draw_text(ctx, volumeAsText, font, r1, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}



static int drawTrackAndArtist(GContext* ctx, int y, GRect bounds) {
  char * txt1;
  char * txt2;
  txt2 = "";
  if (is_system_off) 
    txt1 = "System Is OFF";
  else if (strlen(now_playing_track)==0)
    txt1 = now_playing_station_location;
  else {
    txt1 = now_playing_track;
    txt2 = now_playing_artist;
  }
  GRect r1 = GRect(2, y, bounds.size.w - 2, h_track);
  GFont* f1 = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  GFont* f2 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  GSize sz = graphics_text_layout_get_content_size(now_playing_track, f1, r1, GTextOverflowModeWordWrap, GTextAlignmentLeft); 
  GRect r2 = GRect(2, y + sz.h + 2, bounds.size.w - 2, h_track - sz.h - 2);
  graphics_draw_text(ctx, txt1, f1, r1, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, txt2, f2, r2, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  return h_track;
}

static int drawItemName(GContext* ctx, int y, GRect bounds) {
  GRect r = GRect(0, y, bounds.size.w, h_source);
  GBitmap *channel_icon = get_image_by_source(now_playing_source);
  if (channel_icon!=NULL) {
    r = GRect(0, y, 32, h_source);
    graphics_draw_bitmap_in_rect(ctx, channel_icon, r);
    r = GRect(33, y, bounds.size.w - 33, h_source);
  }
  GFont* f1 = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  char *txt;
  if (strcmp(SOURCE_AIR_PLAY, now_playing_source)==0 || strcmp(SOURCE_AUX, now_playing_source)==0) 
    txt = now_playing_source;
  else
    txt = now_playing_item_name;
  drawText(ctx,r,txt,f1,conf_text_color_bg,conf_text_color_fg);
  return h_source;
}
static void now_playing_layer_update_callback(Layer *me, GContext* ctx) {
  GRect bounds = layer_get_frame(me);
  if (shouldShowVolume) {
    draw_volume_as_circle(ctx,bounds);
    return;
  }
  int y = 0;
  y+=drawItemName(ctx,y,bounds);
  graphics_context_set_stroke_color(ctx, conf_stroke_color);
  graphics_draw_line(ctx, GPoint(0,y), GPoint(bounds.size.w, y));
  y++;
  y+=drawTrackAndArtist(ctx,y,bounds);
}

static void now_playing_time_layer_update_callback(Layer *me, GContext* ctx) {
  if (now_playing_time_total==0)
    return;
  GRect bounds = layer_get_frame(now_playing_time_layer);
  int y = bounds.size.h / 2;
  graphics_context_set_fill_color(ctx, conf_stroke_color);
  graphics_context_set_stroke_color(ctx, conf_stroke_color);
  graphics_draw_line(ctx,GPoint(0,y) , GPoint(bounds.size.w,y));
  int x = (bounds.size.w - 2) * now_playing_time_position  / now_playing_time_total;
  graphics_fill_circle(ctx, GPoint(x,y), 4);
}



void win_now_playing_show() {
  screen_loaded = true;
  Window* window = window_create();
  #ifndef PBL_COLOR
  window_set_fullscreen(window,true);
  #endif
  Layer* root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  //now playing
  int y = 0;
  int w = bounds.size.w - ACTION_BAR_WIDTH;
  int h = h_source + spacer + h_track;
  GRect r = GRect(0, y, w , h);
  y =  h + 5;
  now_playing_layer = layer_create(r);
  layer_set_update_proc(now_playing_layer, now_playing_layer_update_callback);
  layer_add_child(root, now_playing_layer);
  //Now Playing Time
  r = GRect(0, y, w, 10);
  now_playing_time_layer = layer_create(r);
  layer_set_update_proc(now_playing_time_layer, now_playing_time_layer_update_callback);
  layer_add_child(root, now_playing_time_layer);
  y+=10;

  //Action Bar
  icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_VOLUME_UP);
  icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_VOLUME_DOWN);
  icon_button_selection = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUTTONS);
  icon_button_shut_off = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_OFF);
  icon_skip_back = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SKIP_BW);
  icon_skip_forward = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SKIP_FW);
  icon_play = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLAY);
  icon_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
  action_bar = action_bar_layer_create();
  #ifdef PBL_COLOR
  action_bar_layer_set_background_color(action_bar,GColorBlue);
  #endif
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
  set_action_bar_icons();  
  action_bar_layer_add_to_window(action_bar, window);  
  //Handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .unload = onUnload,
    .appear = on_appear,
    .disappear = on_disappear
  });
  //Show the window
  window_stack_pop_all(false);
  window_stack_push(window, false);
}


void win_now_playing_refresh_data() {
  if (!screen_loaded)
    return;
  layer_mark_dirty(now_playing_time_layer);
  layer_mark_dirty(now_playing_layer);
  if (!should_update_select_button)
    return;
  if (now_playing_state == PLAY_STATE_PLAY)
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_pause);
  else
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_play);
}