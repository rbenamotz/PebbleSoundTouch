#include <pebble.h>
#pragma once
#define SOURCE_AIR_PLAY "AIRPLAY"  
#define SOURCE_AUX "AUX"  
#define SOURCE_PANDORA "PANDORA"
#define SOURCE_I_HEART_RADIO "IHEART"
#define SOURCE_INTERNET_RADIO "INTERNET_RADIO"
#define SOURCE_STORED_MUSIC "STORED_MUSIC"
#define SOURCE_DEEZER "DEEZER" 
#define SOURCE_SPOTIFY "SPOTIFY"
#define SOURCE_STANDBY "STANDBY"
#define PLAY_STATE_PLAY 1
#define PLAY_STATE_PAUSE 2
#define PLAY_STATE_UNKNOWN 3
  
  

  

void init_model();
int speaker_ip[4];
GColor conf_text_color_fg;
GColor conf_text_color_bg;
GColor conf_title_text_color_fg;
GColor conf_title_text_color_bg;
GColor conf_stroke_color;


void change_ip_address(int part, int delta);
//int selected_button;
bool is_ip_validated;
bool is_ip_set;
bool is_js_ready;
char presets_source[6][100];
char presets_description[6][100];
//int presets_id[6][100];
void persist_data();
char now_playing_artist[100];
char now_playing_track[100];
char now_playing_album[100];
char now_playing_item_name[100];
char now_playing_source[100];
char now_playing_station_location[100];
int now_playing_state;
int now_playing_time_position;
int now_playing_time_total;


int volume;
int selected_channel;
bool is_system_off;

GBitmap* get_image_by_source(char* source);