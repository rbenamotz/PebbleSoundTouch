#include <pebble.h>
#include "model.h"
  
#define DATA_KEY_IP 0
  
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
    conf_text_color_fg = GColorDarkGreen;
    conf_text_color_bg = GColorMediumSpringGreen;
  #else
    conf_text_color_fg = GColorWhite;
    conf_text_color_bg = GColorBlack;
  #endif

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

