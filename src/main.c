#include <pebble.h>

static Window *s_main_window; 
static TextLayer *s_time_layer;
static GFont s_time_font;

static void main_window_load(Window *window) {
  s_time_layer = text_layer_create(GRect(0, 0, 144, 168));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERA_8));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
//  text_layer_set_overflow_mode(s_time_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  fonts_unload_custom_font(s_time_font);
  text_layer_destroy(s_time_layer);
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char fmt[] = "\
+--------------------------+\n\
|   [ BENTHOR'S  WATCH ]   |\n\
+--------------------------+\n\
|         %H:%M:%S         |\n\
|      %3a %F      |\n\
+--------------------------+\n\
| Battery Percent:    %%3u%%%% |\n\
| Battery Charging:  %%5s |\n\
| Charger Connected: %%5s |\n\
| Bluetooth:  %%12s |\n\
| Heading:         %%3i %%3i |\n\
| Accel X:           %%5i |\n\
| Accel Y:           %%5i |\n\
| Accel Z:           %%5i |\n\
|                          |\n\
|                          |\n\
|                          |\n\
|                          |\n\
+--------------------------+";

  static char temp2[580];
  strftime(temp2, sizeof(temp2), fmt, tick_time);

  static char buffer[580];

  BatteryChargeState state = battery_state_service_peek();
  static char charging[6];
  static char plugged[6];
  static char connected[12];
  state.is_charging ? strcpy(charging, "true") : strcpy(charging, "false");
  state.is_plugged ? strcpy(plugged, "true") : strcpy(plugged, "false");
  bluetooth_connection_service_peek() ? strcpy(connected, "connected") : strcpy(connected, "disconnected");
  
  CompassHeadingData heading;
  compass_service_peek(&heading);

  AccelData accel;
  accel_service_peek(&accel);

  snprintf(buffer, sizeof(buffer), temp2, state.charge_percent, charging, plugged, connected, TRIGANGLE_TO_DEG(heading.magnetic_heading), TRIGANGLE_TO_DEG(heading.true_heading), accel.x, accel.y, accel.z);
  text_layer_set_text(s_time_layer, buffer);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}


static void init() {
  s_main_window = window_create();
  
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);

//  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  update_time();

}

static void deinit() {
  window_destroy(s_main_window);
}


int main(void) {
  init();
  app_event_loop();
  deinit();
}