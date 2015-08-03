#pragma once
  
  

void comm_init();
void set_ip();
void validate_ip();
void read_now_playing();
void push_button(int buttonId);
void showMainScreen();
void read_volume();
void change_volume(int delta);
void read_presets();
void shut_off();
void prev_track();
void next_track();
void play_pause();