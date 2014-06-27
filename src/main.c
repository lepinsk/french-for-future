//
//  main.c
//  French for Future
//
//  Created by Julian Lepinski on 2014-02-12
//  Based on Futura Weather by Niknam (https://github.com/Niknam/futura-weather-sdk2.0)
//

#include <pebble.h>

#include "network.h"
#include "config.h"
#include <ctype.h>

#define LOADING_TIMEOUT 30000

static Window     *window;
static TextLayer  *date_layer;
static TextLayer  *day_layer;
static TextLayer  *time_layer;
static TextLayer  *temp_layer;
static TextLayer  *cond_layer;

static int        condition_global;
static int        condition_temp_global;

static bool s_weather_loaded = false;

static bool charger_connected = NULL;

static bool use_yahoo_weather_conditions = true;

static AppTimer *s_loading_timeout = NULL;

GBitmap           *line_bmp;
BitmapLayer       *line_layer;

static char date_text[]   = "XXXXXXXXX 00";
static char day_text[]    = "XXXXXXXXX";
static char time_text[]   = "00:00";
static char temp_text[]   = "XXXXX";

GFont font_date;
GFont font_time;

bool just_launched = true;
bool currently_displaying_batt = false;

// returns an uppercased version of a char array
char *upcase(char *str) {
  char *s = str;
  while (*s) {
    *s++ = toupper((int)*s);
  }
  return str;
}

// loads the appropriate weather string into cond_layer
static void display_weather_condition(){
  if (use_yahoo_weather_conditions){
    

    if (condition_global < 5 || (condition_global >= 37 && condition_global <= 47)) {
      text_layer_set_text(cond_layer, "STORMY");
    } else if (condition_global < 10) {
      text_layer_set_text(cond_layer, "DRIZZLE");
    } else if (condition_global < 13) {
      text_layer_set_text(cond_layer, "RAINY");
    } else if (condition_global < 17) {
      text_layer_set_text(cond_layer, "SNOWY");
    } else if (condition_global < 18) {
      text_layer_set_text(cond_layer, "HAILY");      
    } else if (condition_global == 20) {
      text_layer_set_text(cond_layer, "FOGGY");
    } else if (condition_global < 24) {
      text_layer_set_text(cond_layer, "WINDY");
    } else if (condition_global < 35 && condition_global > 30) {
      text_layer_set_text(cond_layer, "CLEAR");
    } else if (condition_global == 29 || condition_global == 30 || condition_global == 27 || condition_global == 28) {
      text_layer_set_text(cond_layer, "P.CLOUDY");
    } else if (condition_global == 26) {
      text_layer_set_text(cond_layer, "CLOUDY");
    } else if (condition_global == 24) {
      text_layer_set_text(cond_layer, "WINDY");
    } else if (condition_global == 25) {
      text_layer_set_text(cond_layer, "COLD");
    } else if (condition_global == 36) {
      text_layer_set_text(cond_layer, "HOT");
    } else {
      text_layer_set_text(cond_layer, "HMM");
    }

    /*
    0 tornado
    1 tropical storm
    2 hurricane
    3 severe thunderstorms
    4 thunderstorms
    5 mixed rain and snow
    6 mixed rain and sleet
    7 mixed snow and sleet
    8 freezing drizzle
    9 drizzle
    10  freezing rain
    11  showers
    12  showers
    13  snow flurries
    14  light snow showers
    15  blowing snow
    16  snow
    17  hail
    18  sleet
    19  dust
    20  foggy
    21  haze
    22  smoky
    23  blustery
    24  windy
    25  cold
    26  cloudy
    27  mostly cloudy (night)
    28  mostly cloudy (day)
    29  partly cloudy (night)
    30  partly cloudy (day)
    31  clear (night)
    32  sunny
    33  fair (night)
    34  fair (day)
    35  mixed rain and hail
    36  hot
    37  isolated thunderstorms
    38  scattered thunderstorms
    39  scattered thunderstorms
    40  scattered showers
    41  heavy snow
    42  scattered snow showers
    43  heavy snow
    44  partly cloudy
    45  thundershowers
    46  snow showers
    47  isolated thundershowers
    */

  } else {
    if (condition_global < 300) {
      text_layer_set_text(cond_layer, "STORMY");
    } else if (condition_global < 500) {
      text_layer_set_text(cond_layer, "DRIZZLE");
    } else if (condition_global < 600) {
      text_layer_set_text(cond_layer, "RAINY");
    } else if (condition_global < 700) {
      text_layer_set_text(cond_layer, "SNOWY");
    } else if (condition_global < 771) {
      text_layer_set_text(cond_layer, "FOGGY");
    } else if (condition_global < 800) {
      text_layer_set_text(cond_layer, "WINDY");
    } else if (condition_global == 800) {
      text_layer_set_text(cond_layer, "CLEAR");
    } else if (condition_global < 804) {
      text_layer_set_text(cond_layer, "P.CLOUDY");
    } else if (condition_global == 804) {
      text_layer_set_text(cond_layer, "CLOUDY");
    } else if ((condition_global >= 900 && condition_global < 903) || (condition_global > 904 && condition_global < 1000)) {
      text_layer_set_text(cond_layer, "WINDY");
    } else if (condition_global == 903) {
      text_layer_set_text(cond_layer, "COLD");
    } else if (condition_global == 904) {
      text_layer_set_text(cond_layer, "HOT");
    } else {
      text_layer_set_text(cond_layer, "HMM");
    }
  }
}

static void step_loading_animation() {
  static int animation_step = 0;
  if (animation_step == 0) {
    text_layer_set_text(cond_layer, "LOADING ");
  } else {
    text_layer_set_text(cond_layer, "LOADING.");
  }
  animation_step = (animation_step + 1) % 2;
}

// called every 1s initially, then every 60s
// displays time/date info, as well as weather; requests weather %15min
static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & MINUTE_UNIT) {                                                  // Update the time
    time_t currentTime = time(0);
    struct tm *currentLocalTime = localtime(&currentTime);

    strftime(   time_text,                                                            // Manually format the time as 12 / 24 hour, as specified
                sizeof(time_text), 
                clock_is_24h_style() ? "%R" : "%I:%M", 
                currentLocalTime);

    if (!clock_is_24h_style() && (time_text[0] == '0')) {                             // Drop the first char of time_text if needed
      memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }

    text_layer_set_text(time_layer, time_text);
  }
  if (units_changed & DAY_UNIT) {                                                     // Update the date - Without a leading 0 on the day of the month
    char month_text[10];
    strftime(month_text, sizeof(month_text), "%B", tick_time);
    snprintf(date_text, sizeof(date_text), "%s %i", month_text, tick_time->tm_mday);
    text_layer_set_text(date_layer, upcase(date_text));

    char day_text_pre[10];
    strftime(day_text_pre, sizeof(day_text_pre), "%A", tick_time);
    snprintf(day_text, sizeof(day_text), "%s", day_text_pre);
    text_layer_set_text(day_layer, upcase(day_text));

    text_layer_set_text(temp_layer, "");
    text_layer_set_text(cond_layer, "LOADING ");
  }

  if (!s_weather_loaded) {
    step_loading_animation();
  }

  if ((units_changed & MINUTE_UNIT) && (tick_time->tm_min % 15 == 0)) {             // Refresh the weather info every 15 minutes
    request_weather();
  }
}

static void set_colour(bool dark) {
  bool bg_colour_is_black = false;
  if (dark){
    bg_colour_is_black = true;
  }

  window_set_background_color(window,   bg_colour_is_black ? GColorBlack : GColorWhite);
  text_layer_set_text_color(day_layer,  bg_colour_is_black ? GColorWhite : GColorBlack);
  text_layer_set_text_color(time_layer, bg_colour_is_black ? GColorWhite : GColorBlack);
  text_layer_set_text_color(date_layer, bg_colour_is_black ? GColorWhite : GColorBlack);
  text_layer_set_text_color(temp_layer, bg_colour_is_black ? GColorWhite : GColorBlack);
  text_layer_set_text_color(cond_layer, bg_colour_is_black ? GColorWhite : GColorBlack);
}

void handle_timer(void *data){
  currently_displaying_batt = false;
  display_weather_condition();
}

void show_battery_state(){
  BatteryChargeState battState = battery_state_service_peek();
  static char battery_text[] = "BAT:100%";
  snprintf(battery_text, sizeof(battery_text), "BAT:%d%%", battState.charge_percent);
  currently_displaying_batt = true;
  text_layer_set_text(cond_layer, battery_text);
  app_timer_register(5000, handle_timer, NULL);
}

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  static time_t lastTapTime = 0;
  if ((time(NULL) - lastTapTime) < 6) {                                           // this is our second tap-accel event in <6s
   show_battery_state();
 }
 lastTapTime = time(NULL);
}

static void mark_weather_loaded(void) {
  if (!s_weather_loaded) {
    s_weather_loaded = true;
    // We don't need to run this every second any more.
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  }
  if (s_loading_timeout != NULL) {
    app_timer_cancel(s_loading_timeout);
    s_loading_timeout = NULL;
  }
}

static void handle_weather_update(WeatherData* weather) {
  condition_temp_global = weather->temperature;
  snprintf(temp_text, sizeof(temp_text), "%i%s", weather->temperature, "°");
  text_layer_set_text(temp_layer, temp_text);

  if (!currently_displaying_batt){
    condition_global = weather->condition;
    display_weather_condition();
  }

  mark_weather_loaded();
}

static void handle_weather_error(WeatherError error) {
  // ditch our degree symbol when we've lost our connection
  snprintf(temp_text, sizeof(temp_text), "%i%s", condition_temp_global, " ");
  text_layer_set_text(temp_layer, temp_text);

  mark_weather_loaded();
}

static void handle_loading_timeout(void* unused) {
  s_loading_timeout = NULL;
  if (!s_weather_loaded) {
    handle_weather_error(WEATHER_E_PHONE);
  }
}

static void handle_battery_state_change(BatteryChargeState charge_state) {
  if (charge_state.is_plugged && !charger_connected) {
    // we weren't plugged in and now we are
    charger_connected = true;
    show_battery_state();
  } else if (!charge_state.is_plugged && charger_connected) {
    // we were plugged in and now we're not
    charger_connected = false;
    show_battery_state();
  }
}

static void init(void) {
  window = window_create();
  window_stack_push(window, true);                                                // true= animated woo

  init_network();
  set_weather_update_handler(handle_weather_update);
  set_weather_error_handler(handle_weather_error);

  font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AVENIR_BOOK_SUBSET_18));
  font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AVENIR_BOOK_SUBSET_48));

  day_layer = text_layer_create(DAY_FRAME);
  text_layer_set_background_color(day_layer, GColorClear);
  text_layer_set_font(day_layer, font_date);
  text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_layer));

  time_layer = text_layer_create(TIME_FRAME);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, font_time);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

  date_layer = text_layer_create(DATE_FRAME);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, font_date);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

  temp_layer = text_layer_create(TEMP_FRAME);
  text_layer_set_background_color(temp_layer, GColorClear);
  text_layer_set_font(temp_layer, font_date);
  text_layer_set_text_alignment(temp_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temp_layer));

  cond_layer = text_layer_create(COND_FRAME);
  text_layer_set_background_color(cond_layer, GColorClear);
  text_layer_set_font(cond_layer, font_date);
  text_layer_set_text_alignment(cond_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(cond_layer));

  GBitmap *new_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DOTS);
  line_layer = bitmap_layer_create(GRect(8, 126, 128, 1));
  bitmap_layer_set_bitmap(line_layer, new_icon);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(line_layer));

  set_colour(true);

  time_t now = time(NULL);
  handle_tick(localtime(&now), SECOND_UNIT | MINUTE_UNIT | HOUR_UNIT | DAY_UNIT );      // Update the screen right away
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);                               // And then every second

  // peek the battery state to set our initial charger value
  BatteryChargeState battState = battery_state_service_peek();
  charger_connected = battState.is_plugged;
  battery_state_service_subscribe(&handle_battery_state_change);

  accel_tap_service_subscribe(&accel_tap_handler);                                      // Add an accel tap watcher

  if(bluetooth_connection_service_peek()) {
    s_loading_timeout = app_timer_register(LOADING_TIMEOUT, handle_loading_timeout, NULL);
  } else {
    handle_weather_error(WEATHER_E_DISCONNECTED);
  }
}

static void deinit(void) {
  window_destroy(window);
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  battery_state_service_unsubscribe();

  text_layer_destroy(time_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(temp_layer);
  text_layer_destroy(cond_layer);
  bitmap_layer_destroy(line_layer);

  fonts_unload_custom_font(font_date);
  fonts_unload_custom_font(font_time);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
