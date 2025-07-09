// Signal K application template file.
//
// This application demonstrates core SensESP concepts in a very
// concise manner. You can build and upload the application as is
// and observe the value changes on the serial port monitor.
//
// You can use this source file as a basis for your own projects.
// Remove the parts that are not relevant to you, and add your own code
// for external hardware libraries.

#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "n2k_senders.h"
#include "sensesp/net/discovery.h"
#include "sensesp/sensors/analog_input.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/system/system_status_led.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/ui/config_item.h"

#include "sensesp/sensors/digital_output.h"
#include "sensesp/signalk/signalk_listener.h"
#include "sensesp/signalk/signalk_value_listener.h"
#include "sensesp/transforms/debounce.h"
#include "sensesp/transforms/moving_average.h"

#include "sensesp_app_builder.h"
#define BUILDER_CLASS SensESPAppBuilder

#include "halmet_analog.h"
#include "halmet_const.h"
#include "halmet_digital.h"
#include "halmet_display.h"
#include "halmet_serial.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/networking.h"

using namespace sensesp;
using namespace halmet;

/////////////////////////////////////////////////////////////////////
// Declare some global variables required for the firmware operation.

#define WINDLASS_GO_DOWN +1
#define WINDLASS_GO_OFF 0
#define WINDLASS_GO_UP -1

TwoWire* i2c;
Adafruit_SSD1306* display;

// Actuate the relay for going up
const uint8_t goUpPin = 17;

// Actuate the relay for going down
const uint8_t goDownPin = 16;

// Counter for chain events
int chainCounter = 0;  

// 1 =  Chain down / count up, -1 = Chain up / count backwards
int upDown = WINDLASS_GO_OFF;  

// Stores last ChainCounter value to allow storage to nonvolatile storage in case of value changes
int lastSavedCounter = 0;

#define ENABLE_DEMO 1                // Set to 1 to enable Demo Mode with up/down counter
#define SAFETY_STOP 0                // Defines safety stop for chain up. Stops defined number of events before reaching zero
#define MAX_CHAIN_LENGTH 40          // Define maximum chan length. Relay off after the value is reached


// Translates counter impuls to meter 0,33 m per pulse
float chainCalibrationValue = 0.33f; 
float chainCalibrationOffset = 0.00f;

unsigned long lastChainSpeedMills = millis();



// Set the ADS1115 GAIN to adjust the analog input voltage range.
// On HALMET, this refers to the voltage range of the ADS1115 input
// AFTER the 33.3/3.3 voltage divider.

// GAIN_TWOTHIRDS: 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
// GAIN_ONE:       1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
// GAIN_TWO:       2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
// GAIN_FOUR:      4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
// GAIN_EIGHT:     8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
// GAIN_SIXTEEN:   16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

const adsGain_t kADS1115Gain = GAIN_ONE;


/////////////////////////////////////////////////////////////////////
// The setup function performs one-time application initialization.
void setup() {
  SetupLogging(ESP_LOG_DEBUG);

  // These calls can be used for fine-grained control over the logging level.
  // esp_log_level_set("*", esp_log_level_t::ESP_LOG_DEBUG);

  Serial.begin(115200);

  /////////////////////////////////////////////////////////////////////
  // Initialize the application framework

  // Construct the global SensESPApp() object
  BUILDER_CLASS builder;
  sensesp_app = (&builder)
                    // EDIT: Set a custom hostname for the app.
                    ->set_hostname("halmet-windlass-controller")
                    // EDIT: Optionally, hard-code the WiFi and Signal K server
                    // settings. This is normally not needed.
                    //->set_wifi("My WiFi SSID", "my_wifi_password")
                    //->set_sk_server("192.168.10.3", 80)
                    // EDIT: Enable OTA updates with a password.
                    //->enable_ota("my_ota_password")
                    ->get_app();

  // initialize the I2C bus
  i2c = new TwoWire(0);
  i2c->begin(kSDAPin, kSCLPin);

  // Initialize ADS1115
  auto ads1115 = new Adafruit_ADS1115();

  ads1115->setGain(kADS1115Gain);
  bool ads_initialized = ads1115->begin(kADS1115Address, i2c);
  debugD("ADS1115 initialized: %d", ads_initialized);

  // Initialize the OLED display
  bool display_present = InitializeSSD1306(sensesp_app->get(), &display, i2c);


    // Configuraion paths
  String goingUpDownSensorReadDelayConfigPath = "/sensor_going_up_down/read_delay";
  String windlassStatusSKPathConfigPath = "/windlass_status/sk";
  String goingUpDownSensorDebounceDelayConfigPath = "/sensor_going_up_down/debounce_delay";
  String chainCounterSKPathConfigPath = "/rodeDeployed/sk";
  String chainCounterSensorDebounceDelayConfigPath = "/chain_counter_sensor/debounce_delay";
  String chainSpeedSKPathConfigPath = "/chainSpeed/sk";
  String chainCalibrationSKPathConfigPath = "/chain_counter_sensor/calibration_value";

  // Signal K paths
  String windlassStatusSKPath = "navigation.anchor.windlass.status";
  String chainCounterSKPath = "navigation.anchor.rodeDeployed";
  String chainSpeedSKPath = "navigation.anchor.windlass.speed";

  // Chain counter metadata
  SKMetadata* chainCounterMetadata = new SKMetadata();
  chainCounterMetadata->units_ = "m";
  chainCounterMetadata->description_ = "Anchor Chain Deployed";
  chainCounterMetadata->display_name_ = "Chain Deployed";
  chainCounterMetadata->short_name_ = "Chain Out";

  auto chainCounterSKOutput = new SKOutputFloat(chainCounterSKPath, chainCounterSKPathConfigPath, chainCounterMetadata);

  // Chain counter speed metadata
  SKMetadata* chainSpeedMetadata = new SKMetadata();
  chainSpeedMetadata->units_ = "m/s";
  chainSpeedMetadata->description_ = "Windlass chain speed";
  chainSpeedMetadata->display_name_ = "Windlass speed";
  chainSpeedMetadata->short_name_ = "Chain speed";

  auto windlassStatusSKOutput = new SKOutputString(windlassStatusSKPath, windlassStatusSKPathConfigPath);
  windlassStatusSKOutput->emit("off");



  ///////////////////////////////////////////////////////////////////
  // Digitakl outputs

  // Set the relay pins as output
  pinMode(goUpPin, OUTPUT);
  pinMode(goDownPin, OUTPUT);

  // Set the relay pins as low
  digitalWrite(goUpPin, LOW );
  digitalWrite(goDownPin, LOW );
  
  ///////////////////////////////////////////////////////////////////
  // Digital inputs

  auto chainCounterSensor = ConnectAlarmSender(kDigitalInputPin1, "D1");
  auto goingUpSensor = ConnectAlarmSender(kDigitalInputPin2, "D2");
  auto goingDownSensor = ConnectAlarmSender(kDigitalInputPin3, "D3");
  auto resetSensor = ConnectAlarmSender(kDigitalInputPin4, "D4");
  
  chainCounterSensor->connect_to(

      // Create a lambda transform function
      new LambdaTransform<int,int>(

        // Catch the counter and the status output instances, input is HIGH when the pin status changes
        [windlassStatusSKOutput](int input) {
        
          // Check if it is the rising front of the pin status change
          if (input == HIGH) {

            // Increase or decrease the couter
            chainCounter = chainCounter + upDown;
            
            // Safety stop counter reached while chain is going up
            if (
              // If the windlass is going up...
              digitalRead(kDigitalInputPin2) == HIGH
              
              // ...and the chain counter reached the safety limit, stop the windlass
              && (chainCounter <= SAFETY_STOP) 
            ) {  
              // Shutdown the relay on up
              digitalWrite(goUpPin, LOW );

              // Update windlass status
              windlassStatusSKOutput->emit("off");

            }
            // Maximum chain lenght reached   
            else if (
              // If the windlass is going up...
              digitalRead(kDigitalInputPin3) == HIGH

              // ...and the rod is deployed, stop the windlass
              && (chainCounter >= MAX_CHAIN_LENGTH) 
            ) {  
              // Shutdown the relay on up
              digitalWrite(goUpPin, LOW );

              // Update windlass status
              windlassStatusSKOutput->emit("off");
            }
            // Check if the chain if free falling
            else if (
              // If the windlass is going down...
              upDown == WINDLASS_GO_DOWN

              // ...but the going down sensor is not sensing it, the windlass is in free fall status
              && digitalRead(kDigitalInputPin3)==LOW) {

              // Update windlass status
              windlassStatusSKOutput->emit("freeFall");  
            }
            // Check if the chain is free rising (for example manually)
            else if (
              // If the windlass is going up...
              upDown == WINDLASS_GO_UP
              
              // ...but the going up sensor is not sensing it, the windlass is in free up status
              && digitalRead(kDigitalInputPin2)==LOW) {
              
              // Update windlass status
              windlassStatusSKOutput->emit("freeUp");  
            }
          }

          // Returm thr chain counter value
          return chainCounter;
        }
        )
      )
    ->connect_to(new Linear(chainCalibrationValue, chainCalibrationOffset, chainCalibrationSKPathConfigPath))
    ->connect_to(chainCounterSKOutput);
    
  

  goingUpSensor->connect_to(new LambdaConsumer<int>([windlassStatusSKOutput](int input) {

      // Check if the windlass is going up (press)
      if (input == HIGH) {

        // The windlass is going up, update the upDown variable
        upDown = WINDLASS_GO_UP;

        // Update the windlass status
        windlassStatusSKOutput->emit("up");
      } else {

        // The windlass is not going up anymore (release), update the windlass status
        windlassStatusSKOutput->emit("off");
      }
    }));
  
  goingDownSensor->connect_to(new LambdaConsumer<int>([windlassStatusSKOutput](int input) {

      // Check if the windlass is going down (press)
      if (input == HIGH) {

        // The windlass is going down, update the upDown variable
        upDown = WINDLASS_GO_DOWN;

        // Update the windlass status
        windlassStatusSKOutput->emit("down");
      } else {

        // Set th up/down value
        upDown = WINDLASS_GO_OFF;

        // The windlass is not going down anymore (release), update the windlass status
        windlassStatusSKOutput->emit("off");
      }
    }));


  resetSensor->connect_to(new LambdaConsumer<int>([chainCounterSKOutput](int input) {

      // Check if the windlass is going down (press)
      if (input == HIGH) {

        chainCounter = 0;
        chainCounterSKOutput->emit(chainCounter);
      } 
    }));
  


  // Sense the chain speed
  auto chainSpeedSensor = ConnectTachoSender(kDigitalInputPin1, "windlass");


  // Manage the chain speed sensor
  chainSpeedSensor
      ->connect_to(new Linear(chainCalibrationValue, chainCalibrationOffset, chainCalibrationSKPathConfigPath))
      ->connect_to(new MovingAverage(4, 4))                                          
      ->connect_to(new SKOutputFloat(chainSpeedSKPath, chainSpeedSKPathConfigPath, chainSpeedMetadata));  


  // Create a linstener for the SignalK windlass status path
  auto windlassStatusListener = new StringSKListener(windlassStatusSKPath);

  // Connect the listener
  windlassStatusListener->connect_to(

      // Create a lambda consumer function
      new LambdaConsumer<String>(
        [chainCounterSKOutput](String input) {

          // Check if a chain counter reset is needed
          if (input == "reset") {

            // Reset the counter
            chainCounter = 0;

            // Publish the counter
            chainCounterSKOutput->emit(chainCounter);
          }  
          // Check if the status is up and the windlass is not going up
          else if (input == "up" && digitalRead(kDigitalInputPin2) == LOW) {

            // Activate the relay up
            digitalWrite(goUpPin,HIGH); 
          }
          // Check if the status is down and the windlass is not going down
          else if (input == "down" && digitalRead(kDigitalInputPin3) == LOW) {

            // Activate the relay down
            digitalWrite(goDownPin,HIGH); 
          }
          // Check if the status is off and the ralay is up or down
          else if (input == "off" && (digitalRead(kDigitalInputPin2) == HIGH || digitalRead(kDigitalInputPin3) == HIGH)) {

            // Switch off the relay on up
            digitalWrite(goUpPin,LOW); 

            // Switch off the relay on down
            digitalWrite(goDownPin,LOW); 
          }
        }));


  


  ///////////////////////////////////////////////////////////////////
  // Display setup

  // Connect the outputs to the display
  
  if (display_present) {
    /*
    event_loop()->onRepeat(1000, []() {
      PrintValue(display, 1, "IP:", WiFi.localIP().toString());
    });
    */
  }

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
