#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer_d;
static TextLayer *s_time_layer_u;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static int s_battery_level;

static void update_time(bool withDate) {
  time_t temp = time(NULL);
  struct tm *tick_time_z = gmtime(&temp);
  struct tm *tick_time_lt = localtime(&temp);
  
  static char s_buffer_z[6];
  static char s_buffer_lt[6];
  strftime(s_buffer_z, sizeof(s_buffer_z), "%H%MZ", tick_time_z);
  strftime(s_buffer_lt, sizeof(s_buffer_lt), "%H:%M", tick_time_lt);
  text_layer_set_text(s_time_layer_d, s_buffer_z);
  text_layer_set_text(s_time_layer_u, s_buffer_lt);
  
  if (withDate) {
    static char s_buffer_date[11];
    strftime(s_buffer_date, sizeof(s_buffer_date), "%d.%m.%Y", tick_time_lt);
    text_layer_set_text(s_date_layer, s_buffer_date);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
 update_time(false);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  int width = (int)(float)(((float)s_battery_level /100.0F) * bounds.size.w);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect((bounds.size.w - width) / 2, 0, width, bounds.size.h), 0, GCornerNone);
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_battery_layer);
}

static TextLayer *configure_text_layer(Layer *window_layer, GRect box, GFont font) {
  TextLayer *layer = text_layer_create(box);
  
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_font(layer, font); // todo check other fonts
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(layer));
  
  return layer;
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  GRect bounds = layer_get_bounds(window_layer);
  
  s_time_layer_u = configure_text_layer(window_layer, GRect(0, PBL_IF_ROUND_ELSE(10, 0), bounds.size.w, 50), fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  s_time_layer_d = configure_text_layer(window_layer, GRect(0, bounds.size.h - PBL_IF_ROUND_ELSE(70, 50), bounds.size.w, 50), fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  s_date_layer = configure_text_layer(window_layer, GRect(0, 55, bounds.size.w, 25), fonts_get_system_font(FONT_KEY_GOTHIC_24));
  
  s_battery_layer = layer_create(GRect(0, bounds.size.h / 2, bounds.size.w, 2));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer_u);
  text_layer_destroy(s_time_layer_d);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);
  
  update_time(true);
  battery_callback(battery_state_service_peek());
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}