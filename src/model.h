#include <pebble.h>
#pragma once
  

void init_model();
int speaker_ip[4];
GColor conf_text_color_fg;
GColor conf_text_color_bg;
GColor conf_title_text_color_fg;
GColor conf_title_text_color_bg;


void change_ip_address(int part, int delta);
//int selected_button;
bool is_ip_validated;
bool is_ip_set;
bool is_js_ready;
char presets[6][100];
void persist_data();
char now_playing_artist[100];
char now_playing_track[100];
char now_playing_album[100];
char now_playing_item_name[100];
char now_playing_source[100];
char now_playing_station_location[100];

int volume;
int selected_channel;
bool is_system_off;