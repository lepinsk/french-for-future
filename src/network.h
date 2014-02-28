#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITION 1
#define KEY_BTC 2
#define KEY_ERROR 3
#define KEY_REQUEST_UPDATE 42
typedef enum {
  UPDATE_E_OK = 0,
  UPDATE_E_DISCONNECTED,
  UPDATE_E_PHONE,
  UPDATE_E_NETWORK
} UpdateError;

typedef struct {
  int temperature;
  int btc_price;
  int condition;
  time_t updated;
  UpdateError error;
} UpdateData;

void init_update(UpdateData *update_data);
void close_network();

void request_update();
