#include <pebble.h>
#include "model.h"
  
#define DATA_KEY_IP 0
  
static GBitmap* icon_source[8];

  
void init_model() {
  speaker_ip[0] = 192;
  speaker_ip[1] = 168;
  speaker_ip[2] = 1;
  speaker_ip[3] = 108;
  is_ip_set = false;
  is_ip_validated = false;
  is_js_ready = false;
  if (persist_exists(DATA_KEY_IP)) {
    persist_read_data(DATA_KEY_IP,speaker_ip, sizeof(speaker_ip));
    is_ip_set = true;
  }
  volume = 0;

  #ifdef PBL_COLOR
    conf_text_color_fg = GColorOxfordBlue;
    conf_text_color_bg = GColorWhite;
    conf_title_text_color_fg = GColorBlue;
    conf_title_text_color_bg = GColorCyan;
    conf_stroke_color = GColorBlue;
  #else
    conf_text_color_fg = GColorBlack;
    conf_text_color_bg = GColorWhite;
    conf_title_text_color_fg = GColorWhite;
    conf_title_text_color_bg = GColorBlack;
    conf_stroke_color = GColorBlack;
  #endif
  icon_source[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SOURCE_INTERNET_RADIO);
  icon_source[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SOURCE_PANDORA);
  icon_source[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SOURCE_AIRPLAY);
  icon_source[3] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SOURCE_STORED_MUSIC);
  icon_source[5] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SOURCE_DEEZER);
  icon_source[6] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SOURCE_SPOTIFY);
  icon_source[7] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SOURCE_IHEART_RADIO);
  now_playing_state = PLAY_STATE_UNKNOWN;
  flag_should_ignore_volume_reading = false;
}

void change_ip_address(int part, int delta) {
  speaker_ip[part]+=delta;
  if (speaker_ip[part]<0)
    speaker_ip[part] = 254;
  if (speaker_ip[part]>254)
    speaker_ip[part] = 0;
}

void persist_data() {
  persist_write_data(DATA_KEY_IP, speaker_ip, sizeof(speaker_ip));
}

GBitmap* get_image_by_source(char* source) {
  if (strcmp(SOURCE_INTERNET_RADIO, source)==0) 
    return icon_source[0];  
  if (strcmp(SOURCE_PANDORA, source)==0) 
    return icon_source[1];
  if (strcmp(SOURCE_AIR_PLAY, source)==0) 
    return icon_source[2];
  if (strcmp(SOURCE_STORED_MUSIC, source)==0)
    return icon_source[3];
  if (strcmp(SOURCE_DEEZER, source)==0)
    return icon_source[5];
  if (strcmp(SOURCE_SPOTIFY, source)==0)
    return icon_source[6];
  if (strcmp(SOURCE_I_HEART_RADIO, source)==0) 
    return icon_source[7];
  return NULL;
  
}

