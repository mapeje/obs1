#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/onewire_temperature.h"
#include "signalk/signalk_output.h"
#include "transforms/linear.h"
#include "wiring_helpers.h"
#include "sensors/bmp280.h"

ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  sensesp_app = new SensESPApp();

  /*
     Find all the sensors and their unique addresses. Then, each new instance
     of OneWireTemperature will use one of those addresses. You can't specify
     which address will initially be assigned to a particular sensor, so if you
     have more than one sensor, you may have to swap the addresses around on
     the configuration page for the device. (You get to the configuration page
     by entering the IP address of the device into a browser.)
  */

  /*
     Tell SensESP where the sensor is connected to the board
     ESP8266 pins are specified as DX
     ESP32 pins are specified as just the X in GPIOX
  */

  //uint8_t pin = 4;

  DallasTemperatureSensors* dts = new DallasTemperatureSensors(4);

  // Define how often SensESP should read the sensor(s) in milliseconds [updates every 5s]
  uint engine_read_delay = 5000;

  // Below are temperatures sampled and sent to Signal K server
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html

  // Measure coolant temperature
  auto* coolant_temp =
      new OneWireTemperature(dts, engine_read_delay, "/coolantTemperature/oneWire");

  coolant_temp->connect_to(new Linear(1.0, 0.0, "/coolantTemperature/linear"))
      ->connect_to(
          new SKOutputNumber("propulsion.mainEngine.coolantTemperature",
                             "/coolantTemperature/skPath"));


// Environment
  // Create a BMP280, which represents the physical sensor.
  // 0x77 is the default address. Some chips use 0x76, which is shown here.
  auto* bmp280 = new BMP280(0x76);

  // If you want to change any of the settings that are set by
  // Adafruit_BMP280::setSampling(), do that here, like this:
  // bmp280->adafruit_bmp280->setSampling(); // pass in the parameters you want

  // Define the read_delays you're going to use:
  const uint env_read_delay = 1000;            // once per second
  const uint pressure_read_delay = 60000;  // once per minute

  // Create a BMP280Value, which is used to read a specific value from the
  // BMP280, and send its output to Signal K as a number (float). This one is for
  // the temperature reading.
  auto* bmp_temperature =
      new BMP280Value(bmp280, temperature, env_read_delay, "/Outside/Temperature");

  bmp_temperature->connect_to(
      new SKOutputNumber("environment.outside.temperature"));

  // Do the same for the barometric pressure value. Its read_delay is longer,
  // since barometric pressure can't change all that quickly. It could be much
  // longer for that reason.
  auto* bmp_pressure = new BMP280Value(bmp280, pressure, pressure_read_delay,
                                       "/Outside/Pressure");

  bmp_pressure->connect_to(new SKOutputNumber("environment.outside.pressure"));

  // Configuration is done, lets start the readings of the sensors!
  sensesp_app->enable();
});
