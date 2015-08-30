#include <ArduinoPebbleSerial.h>

static const uint16_t SERVICE_ID = 0x1001;
static const uint16_t LED_ATTRIBUTE_ID = 0x0001;
static const size_t LED_ATTRIBUTE_LENGTH = 1;
static const uint16_t UPTIME_ATTRIBUTE_ID = 0x0002;
static const size_t UPTIME_ATTRIBUTE_LENGTH = 4;

static const uint16_t SERVICES[] = {SERVICE_ID};
static const uint8_t NUM_SERVICES = 1;

// TODO: make conditional per platform
// Teensy
//static const uint8_t PEBBLE_DATA_PIN = 1;
// Uno
static const uint8_t PEBBLE_DATA_PIN = 2;

static uint8_t buffer[20];


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // TODO: revert to Baud57600 post-debug
  ArduinoPebbleSerial::begin_software(PEBBLE_DATA_PIN, buffer, sizeof(buffer), Baud57600, SERVICES,
                                      NUM_SERVICES);

  // initialize serial (for debug)
  Serial.begin(9600);
  Serial.println("setup complete");
  
  digitalWrite(LED_BUILTIN, false);
}

static void prv_handle_uptime_request(RequestType type, size_t length) {
  if (type != RequestTypeRead) {
    // unexpected request type
    return;
  }
  // write back the current uptime
  const uint32_t uptime = millis() / 1000;
  ArduinoPebbleSerial::write(true, (uint8_t *)&uptime, sizeof(uptime));
}

static void prv_handle_led_request(RequestType type, size_t length) {
  if (type != RequestTypeWrite) {
    // unexpected request type
    Serial.print("unexpected RequestType");
    return;
  } else if (length != LED_ATTRIBUTE_LENGTH) {
    // unexpected request length
    Serial.print("unexpected length");
    return;
  }
  // set the LED
  digitalWrite(LED_BUILTIN, (bool) buffer[0]);
  // ACK that the write request was received
  ArduinoPebbleSerial::write(true, NULL, 0);
  Serial.print("digitalWrite():");
  Serial.println((bool) buffer[0]);
  Serial.println("write(true, NULL, 0)");
}

void loop() {
  if (ArduinoPebbleSerial::is_connected()) {
    Serial.println("IS_connected()");
    static uint32_t last_notify_time = 0;
    const uint32_t current_time = millis() / 1000;
    if (current_time > last_notify_time) {
      ArduinoPebbleSerial::notify(SERVICE_ID, UPTIME_ATTRIBUTE_ID);
      Serial.println("notify()");
      last_notify_time = current_time;
    }
  } else {
    Serial.println("!is_connected()");
  }

  uint16_t service_id;
  uint16_t attribute_id;
  size_t length;
  RequestType type;
  if (ArduinoPebbleSerial::feed(&service_id, &attribute_id, &length, &type)) {
    Serial.println("feed()");
    // process the request
    if (service_id == SERVICE_ID) {
      Serial.println("correct SERVICE_ID");
      switch (attribute_id) {
        case UPTIME_ATTRIBUTE_ID:
          Serial.println("UPTIME_ATTRIBUTE_ID");
          prv_handle_uptime_request(type, length);
          break;
        case LED_ATTRIBUTE_ID:
          Serial.println("LED_ATTRIBUTE_ID");
          prv_handle_led_request(type, length);
          break;
        default:
          Serial.println("unknown attribute_id");
          break;
      }
    }
  }
}
