//
//  network.h
//  French for Future
//
//  Created by Julian Lepinski on 2014-02-12
//  Based on Futura Weather by Niknam (https://github.com/Niknam/futura-weather-sdk2.0)
//

#include <pebble.h>

// these are our dictionary keys – they all correspond to stuff in appinfo.json
#define KEY_TEMPERATURE 0
#define KEY_CONDITION 1
#define KEY_SUNRISE 2
#define KEY_SUNSET 3
#define KEY_CURRENT_TIME 4
#define KEY_ERROR 5
#define KEY_COLOURSCHEME 6
#define KEY_REQUEST_UPDATE 42

typedef enum {
  WEATHER_E_OK = 0,
  WEATHER_E_DISCONNECTED,
  WEATHER_E_PHONE,
  WEATHER_E_NETWORK
} WeatherError;

typedef struct {
  int temperature;
  int condition;
  int sunrise;
  int sunset;
  int current_time;
  time_t updated;
} WeatherData;

typedef void (*WeatherUpdateHandler)(WeatherData*);
typedef void (*WeatherErrorHandler)(WeatherError);

void init_network(void);
void close_network(void);

void request_weather(void);

void set_weather_update_handler(WeatherUpdateHandler handler);
void set_weather_error_handler(WeatherErrorHandler handler);
