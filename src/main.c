#include <pebble.h>

#include "network.h"
#include "config.h"
#include <ctype.h>

#define DAY_FRAME       (GRect(0, 17, 144, 168-62))
#define TIME_FRAME      (GRect(0, 33, 144, 168-20))
#define DATE_FRAME      (GRect(0, 90, 144, 168-62))
#define TEMP_FRAME      (GRect(10, 138, 134, 168-62))
#define COND_FRAME      (GRect(0, 138, 134, 168-62))

/* Keep a pointer to the current weather data as a global variable */
static WeatherData *weather_data;

/* Global variables to keep track of the UI elements */
static Window *window;
static TextLayer *date_layer;
static TextLayer *day_layer;
static TextLayer *time_layer;
static TextLayer *temp_layer;
static TextLayer *cond_layer;

GBitmap *line_bmp;
BitmapLayer *line_layer;

static char date_text[] = "XXXXXXXXX 00";
static char day_text[] = "XXXXXXXXX";
static char time_text[] = "00:00";
static char temp_text[] = "XXXXX";

/* Preload the fonts */
GFont font_date;
GFont font_time;

bool bg_colour_is_black = true;

char *upcase(char *str)
{
    char *s = str;
    while (*s) {
        *s++ = toupper((int)*s);
    }
    return str;
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
  if (units_changed & MINUTE_UNIT) {
    // Update the time - Fix to deal with 12 / 24 centering bug
    time_t currentTime = time(0);
    struct tm *currentLocalTime = localtime(&currentTime);

    // Manually format the time as 12 / 24 hour, as specified
    strftime(   time_text, 
                sizeof(time_text), 
                clock_is_24h_style() ? "%R" : "%I:%M", 
                currentLocalTime);

    // Drop the first char of time_text if needed
    if (!clock_is_24h_style() && (time_text[0] == '0')) {
      memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }

    text_layer_set_text(time_layer, time_text);
  }
  if (units_changed & DAY_UNIT) {
    // Update the date - Without a leading 0 on the day of the month
    char month_text[10];
    strftime(month_text, sizeof(month_text), "%B", tick_time);
    snprintf(date_text, sizeof(date_text), "%s %i", month_text, tick_time->tm_mday);
    text_layer_set_text(date_layer, upcase(date_text));

    char day_text_pre[10];
    strftime(day_text_pre, sizeof(day_text_pre), "%A", tick_time);
    snprintf(day_text, sizeof(day_text), "%s", day_text_pre);
    text_layer_set_text(day_layer, upcase(day_text));

    text_layer_set_text(temp_layer, "");
    text_layer_set_text(cond_layer, "LOADING  ");
  }

  // Update the bottom half of the screen: icon and temperature
  static int animation_step = 0;
  if (weather_data->updated == 0 && weather_data->error == WEATHER_E_OK)
  {
    // 'Animate' loading icon until the first successful weather request
    if (animation_step == 0) {
      text_layer_set_text(cond_layer, "LOADING  ");
    }
    else if (animation_step == 1) {
      text_layer_set_text(cond_layer, "LOADING ");
    }
    else if (animation_step >= 2) {
      text_layer_set_text(cond_layer, "LOADING");
    }
    animation_step = (animation_step + 1) % 3;
  }
  else {
    // Update the weather icon and temperature
    if (weather_data->error) {
      text_layer_set_text(cond_layer, "ERROR");
    }
    else {
      // Show the temperature as 'stale' if it has not been updated in 30 minutes
      bool stale = false;
      if (weather_data->updated > time(NULL) + 1800) {
        stale = true;
      }

      // update the temp layer
      snprintf(temp_text, sizeof(temp_text), "%i%s", weather_data->temperature, stale ? " " : "°");
      text_layer_set_text(temp_layer, temp_text);
      

      int c = weather_data->condition;
      if (c < 300) {
        text_layer_set_text(cond_layer, "STORMY");
      }
      // Drizzle
      else if (c < 500) {
        text_layer_set_text(cond_layer, "DRIZZLE");
      }
      // Rain / Freezing rain / Shower rain
      else if (c < 600) {
        text_layer_set_text(cond_layer, "RAINY");
      }
      // Snow
      else if (c < 700) {
        text_layer_set_text(cond_layer, "SNOWY");
      }
      // Fog / Mist / Haze / etc.
      else if (c < 771) {
        text_layer_set_text(cond_layer, "FOGGY");
      }
      // Tornado / Squalls
      else if (c < 800) {
        text_layer_set_text(cond_layer, "WINDY");
      }
      // Sky is clear
      else if (c == 800) {
        text_layer_set_text(cond_layer, "CLEAR");
      }
      // few/scattered/broken clouds
      else if (c < 804) {
        text_layer_set_text(cond_layer, "P.CLOUDY");
      }
      // overcast clouds
      else if (c == 804) {
        text_layer_set_text(cond_layer, "CLOUDY");
      }
      // Extreme
      else if ((c >= 900 && c < 903) || (c > 904 && c < 1000)) {
        text_layer_set_text(cond_layer, "WINDY");
      }
      // Cold
      else if (c == 903) {
        text_layer_set_text(cond_layer, "COLD");
      }
      // Hot
      else if (c == 904) {
        text_layer_set_text(cond_layer, "HOT");
      }
      else {
        text_layer_set_text(cond_layer, "HMM");
      }
    }
  }

  // Refresh the weather info every 15 minutes
  if (units_changed & MINUTE_UNIT && (tick_time->tm_min % 15) == 0)
  {
    request_weather();
  }
}

static void init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, bg_colour_is_black ? GColorBlack : GColorWhite);

  weather_data = malloc(sizeof(WeatherData));
  init_network(weather_data);

  font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AVENIR_BOOK_SUBSET_16));
  font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AVENIR_BOOK_SUBSET_48));

  day_layer = text_layer_create(DAY_FRAME);
  text_layer_set_text_color(day_layer, bg_colour_is_black ? GColorWhite : GColorBlack);
  text_layer_set_background_color(day_layer, GColorClear);
  text_layer_set_font(day_layer, font_date);
  text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_layer));

  time_layer = text_layer_create(TIME_FRAME);
  text_layer_set_text_color(time_layer, bg_colour_is_black ? GColorWhite : GColorBlack);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, font_time);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

  date_layer = text_layer_create(DATE_FRAME);
  text_layer_set_text_color(date_layer, bg_colour_is_black ? GColorWhite : GColorBlack);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, font_date);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

  temp_layer = text_layer_create(TEMP_FRAME);
  text_layer_set_text_color(temp_layer, bg_colour_is_black ? GColorWhite : GColorBlack);
  text_layer_set_background_color(temp_layer, GColorClear);
  text_layer_set_font(temp_layer, font_date);
  text_layer_set_text_alignment(temp_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temp_layer));

  cond_layer = text_layer_create(COND_FRAME);
  text_layer_set_text_color(cond_layer, bg_colour_is_black ? GColorWhite : GColorBlack);
  text_layer_set_background_color(cond_layer, GColorClear);
  text_layer_set_font(cond_layer, font_date);
  text_layer_set_text_alignment(cond_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(cond_layer));

  GBitmap *new_icon =  gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DOTS);
  line_layer = bitmap_layer_create(GRect(8, 126, 128, 1));
  bitmap_layer_set_bitmap(line_layer, new_icon);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(line_layer));

  // Update the screen right away
  time_t now = time(NULL);
  handle_tick(localtime(&now), SECOND_UNIT | MINUTE_UNIT | HOUR_UNIT | DAY_UNIT );
  // And then every second
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

static void deinit(void) {
  window_destroy(window);
  tick_timer_service_unsubscribe();

  text_layer_destroy(time_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(temp_layer);
  text_layer_destroy(cond_layer);
  bitmap_layer_destroy(line_layer);

  fonts_unload_custom_font(font_date);
  fonts_unload_custom_font(font_time);

  free(weather_data);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
