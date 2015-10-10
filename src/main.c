#include <pebble.h>

static Window *s_window;
static Layer *s_canvas;

static GColor random_color() {
  int r = rand()%255;
  int g = rand()%255;
  int b = rand()%255;
  double color_perception = 1 - ( 0.299 * r + 0.587 * g + 0.114 * b)/255;
  if (color_perception > 0.5) {
    return GColorFromRGB(r, g, b);
  } else {
    return random_color();
  }
}

static int32_t get_angle_for_hour(int hour) {
  // Progress through 12 hours, out of 360 degrees
  return (hour * 360) / 12;
}

static int32_t get_angle_for_minute(int min) {
  // Progress through 60 minutes, out of 360 degrees
  return (min * 360) / 60;
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *s_time = localtime(&temp);
  static char timeBuffer[] = "00:00";
  static char dateBuffer[] = "";
  if(clock_is_24h_style() == true) {
    strftime(timeBuffer, sizeof("00:00"), "%H:%M", s_time);
  } else {
    strftime(timeBuffer, sizeof("00:00"), "%I:%M", s_time);
  }
  strftime(dateBuffer, sizeof("DD/MM"), "%d/%m", s_time);
  
  GRect bounds = layer_get_bounds(layer);
  GRect frame_hours = grect_inset(bounds, GEdgeInsets(15));
  GRect frame_minutes = grect_inset(frame_hours, GEdgeInsets(15));
  GRect frame_digital = GRect(frame_minutes.origin.x, frame_minutes.origin.y + 30,
                              frame_minutes.size.w, frame_minutes.size.h - 30);
  GRect frame_date = GRect(frame_digital.origin.x, frame_digital.origin.y + 40,
                              frame_digital.size.w, frame_digital.size.h - 40);

  graphics_context_set_text_color(ctx, random_color());
  graphics_draw_text(ctx, timeBuffer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS),
                     frame_digital, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, dateBuffer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                     frame_date, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  graphics_context_set_fill_color(ctx, random_color());
  graphics_fill_radial(ctx, frame_hours, GOvalScaleModeFitCircle, 10,
                       DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(get_angle_for_hour(s_time->tm_hour)));
  
  graphics_context_set_fill_color(ctx, random_color());
  graphics_fill_radial(ctx, frame_minutes, GOvalScaleModeFitCircle, 10,
                       DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(get_angle_for_minute(s_time->tm_min)));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, layer_update_proc);
  layer_add_child(window_layer, s_canvas);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas);
  window_destroy(s_window);
}

static void tick_handler(struct tm *time_now, TimeUnits changed) {
  layer_mark_dirty(s_canvas);
}

static void init() {
  s_window = window_create();
  window_set_background_color(s_window, GColorWhite);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  window_stack_push(s_window, true);
}

static void deinit() { 
  tick_timer_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
