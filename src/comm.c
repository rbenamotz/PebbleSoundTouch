#include <pebble.h>
#include "comm.h"
#include "model.h"
#include "buttons.h"
#include "win_ip_select.h"
#include "win_main.h"
#include "now_playing.h"
  
#define CMD_SET_IP 0
#define CMD_PUSH_BUTTON 1
#define CMD_JS_READY 2
#define CMD_VALIDATE_IP 3
#define CMD_GET_NOW_PLAYING 4
#define CMD_SET_VOLUME 5  
#define CMD_GET_VOLUME 6
#define CMD_GET_PRESETS 7
#define CMD_SHUT_OFF 8
#define CMD_PREV_TRACK 9
#define CMD_NEXT_TRACK 10
#define CMD_PLAY_PAUSE 11
  
  
#define KEY_CMD_EXECUTED 0
#define KEY_IP_ADDRESS 1
#define KEY_BUTTON_ID 2
#define KEY_CMD_VALUE 3
#define KEY_NEW_VOLUME 4
#define KEY_NOW_PLAYING_ARTIST 10
#define KEY_NOW_PLAYING_TRACK 11
#define KEY_NOW_PLAYING_ALBUM 12
#define KEY_NOW_PLAYING_ITEM_NAME 13
#define KEY_NOW_PLAYING_SOURCE 14
#define KEY_NOW_PLAYING_STATION_LOCATION 15
#define KEY_NOW_PLAYING_PLAY_STATE 16
#define KEY_NOW_PLAYING_TIME_POSITION 17
#define KEY_NOW_PLAYING_TIME_TOTAL 18
#define KEY_VOLUME 20


static char ip_as_text[20];

char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

	
// Called when a message is received from PebbleKitJS
void inbox_received_callback(DictionaryIterator *iterator, void *context)  {
  Tuple *t = dict_read_first(iterator);
  int executdCommand = -1;
  int commandValue = -1;
  while(t != NULL) {
    int i = (int) t->key;
    switch(i) {
      case KEY_CMD_EXECUTED:
        executdCommand = (int) t->value->int32;
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "Command Executed: %i" , executdCommand);
        break;
      case KEY_CMD_VALUE:
        commandValue = (int) t->value->int32;
        if (executdCommand == CMD_JS_READY) {
          is_js_ready = true;
        }
        if (executdCommand == CMD_SET_IP || executdCommand == CMD_VALIDATE_IP) {
          is_ip_validated = (commandValue==1);
        }
        break;
      case KEY_NOW_PLAYING_ARTIST:
        snprintf(now_playing_artist, sizeof(now_playing_artist), "%s", t->value->cstring);
        break;
      case KEY_NOW_PLAYING_TRACK:
        snprintf(now_playing_track, sizeof(now_playing_track), "%s", t->value->cstring);
        break;
      case KEY_NOW_PLAYING_ALBUM:
        snprintf(now_playing_album, sizeof(now_playing_album), "%s", t->value->cstring);
        break;
      case KEY_NOW_PLAYING_ITEM_NAME:
        snprintf(now_playing_item_name, sizeof(now_playing_item_name), "%s", t->value->cstring);
        break;
      case KEY_NOW_PLAYING_SOURCE:
        snprintf(now_playing_source, sizeof(now_playing_source), "%s", t->value->cstring);
        is_system_off = (strcmp("STANDBY",now_playing_source)==0);
        break;
      case KEY_NOW_PLAYING_STATION_LOCATION:
        snprintf(now_playing_station_location, sizeof(now_playing_station_location), "%s", t->value->cstring);
        break;
      case KEY_NOW_PLAYING_PLAY_STATE:
        now_playing_state = (int) t->value->int32;
        break;
      case KEY_VOLUME:
        if (flag_should_ignore_volume_reading)
          break;
        volume = (int) t->value->int32;
        break;
      case KEY_NOW_PLAYING_TIME_POSITION:
        now_playing_time_position = (int) t->value->int32;
        break;
      case KEY_NOW_PLAYING_TIME_TOTAL:
        now_playing_time_total = (int) t->value->int32;
        break;
      default:
        if (i>=1000 && i<1010) { //Presets source 
          snprintf(presets_source[i - 1000], sizeof(presets_source[i - 1000]), "%s", t->value->cstring);
        } 
        if (i>=1010 && i<1020) { //Presets Descriptions 
          snprintf(presets_description[i - 1010], sizeof(presets_description[i - 1010]), "%s", t->value->cstring);
        } 
    }
    t = dict_read_next(iterator);
  }
  win_now_playing_refresh_data();
  win_buttons_refresh_data();
  if (executdCommand == CMD_SET_IP) 
    set_ip_completed();
  if (executdCommand == CMD_VALIDATE_IP) 
    validate_ip_completed();
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbox dropped: %i - %s", reason, translate_error(reason));
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "out_failed_handler: %i - %s", reason, translate_error(reason));
}

void comm_init() {
	// Register AppMessage handlers
	app_message_register_inbox_received(inbox_received_callback); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
}

void set_ip() {
  snprintf(ip_as_text, sizeof(ip_as_text), "%d.%d.%d.%d", speaker_ip[0],speaker_ip[1],speaker_ip[2],speaker_ip[3]);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_cstring(iter, KEY_IP_ADDRESS, ip_as_text);
  dict_write_uint8(iter, KEY_CMD_EXECUTED, CMD_SET_IP);
	dict_write_end(iter);
  app_message_outbox_send();
}

void push_button(int buttonId) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_CMD_EXECUTED, CMD_PUSH_BUTTON);
  dict_write_uint8(iter, KEY_BUTTON_ID, buttonId);
	dict_write_end(iter);
  app_message_outbox_send();
}

void validate_ip() {
  snprintf(ip_as_text, sizeof(ip_as_text), "%d.%d.%d.%d", speaker_ip[0],speaker_ip[1],speaker_ip[2],speaker_ip[3]);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_cstring(iter, KEY_IP_ADDRESS, ip_as_text);
  dict_write_uint8(iter, KEY_CMD_EXECUTED, CMD_VALIDATE_IP);
	dict_write_end(iter);
  app_message_outbox_send();
}

void read_now_playing() {
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE,"read_now_playing");
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_CMD_EXECUTED,CMD_GET_NOW_PLAYING);
	dict_write_end(iter);
  app_message_outbox_send();
}

void read_volume() {
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE,"read_volume");
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_CMD_EXECUTED,CMD_GET_VOLUME);
	dict_write_end(iter);
  app_message_outbox_send();
}

void read_presets() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_CMD_EXECUTED,CMD_GET_PRESETS);
	dict_write_end(iter);
  app_message_outbox_send();
}

void change_volume() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_CMD_EXECUTED,CMD_SET_VOLUME);
  dict_write_uint8(iter, KEY_NEW_VOLUME,volume);
	dict_write_end(iter);
  app_message_outbox_send();
  win_now_playing_refresh_data();
}
void shut_off() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_CMD_EXECUTED,CMD_SHUT_OFF);
	dict_write_end(iter);
  app_message_outbox_send();
}
void prev_track() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_CMD_EXECUTED,CMD_PREV_TRACK);
	dict_write_end(iter);
  app_message_outbox_send();
}
void next_track() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_CMD_EXECUTED,CMD_NEXT_TRACK);
	dict_write_end(iter);
  app_message_outbox_send();
}
void play_pause() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_uint8(iter, KEY_CMD_EXECUTED,CMD_PLAY_PAUSE);
	dict_write_end(iter);
  app_message_outbox_send();
}



