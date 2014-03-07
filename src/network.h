//
//  network.h
//  French for Future
//
//  Created by Julian Lepinski on 2014-02-12
//  Based on Futura Weather by Niknam (https://github.com/Niknam/futura-weather-sdk2.0)
//

#include <pebble.h>

// these are our dictionary keys – correspond to stuff in appinfo.json
#define KEY_TEMPERATURE 0
#define KEY_CONDITION 1
#define KEY_ERROR 2
#define KEY_COLOURSCHEME 3
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
  time_t updated;
  WeatherError error;
} WeatherData;

void (*cbf)(bool);
void init_network(WeatherData *weather_data, void (*callbackFunction)(bool));
void close_network();
void request_weather();
