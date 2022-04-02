/********************************************************************************
 * @file    main.ino
 * @author  Kostas Markostamos
 * @date    31/03/2022
 * @brief   Main Arduino program file.
 *          Declares MQ3, TFSM and LiquidCrystal_I2C instances.
 *          Declares the states of the TFSM of the MQ3 sensor.
 *          Defines and declares the state action and delay callbacks of the MQ3
 *          sensor.
 *          Serial print at every state and delayed transition for internal
 *          info.
 *          LCD display at every state and delayed transition for user info.
 *          Program loop checks if cycle time of the TFSM elapsed and runs its
 *          current state action.
 *          Information about the project will be written in the README.
 * 
 *          TODO: - Finish LCD display process for most states.
 *                - Comment state action functions.
 *                - Eventually keep only core programm process in main file and
 *                  move the rest in appropriate header, source files.
********************************************************************************/


/**************************************
 * Includes
 **************************************/
#include <Arduino.h>
#include <Wire.h>
#include <avr/wdt.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Tfsm.h>
#include <Mq3.h>

/**************************************
 * Defines
 **************************************/
#define EEPROM_VALID_CONFIG ((byte)'C')

/**************************************
 * Typedefs
 **************************************/
typedef enum {
  STATE_INIT_WARMUP = 0,
  STATE_RUN_WARMUP,
  STATE_CONFIG,
  STATE_CALIBRATE,
  STATE_VERIFY,
  STATE_MAIN,
  STATE_RESET,
} E_STATE;

/**************************************
 * State action functions prototypes
 **************************************/
void delay_cb(void);
void state_initWarmUp(void);
void state_runWarmUp(void);
void state_config(void);
void state_calibrate(void);
void state_verify(void);
void state_main(void);
void state_reset(void);

/**************************************
 * Variables
 **************************************/
TFSM::ST_STATE state_table[] = { // cycle, steps, delay, primary_transition, alternate_transition, action, delay_cb
  // STATE_INIT_WARMUP
  {0, 1, 0, STATE_RUN_WARMUP, STATE_RESET, state_initWarmUp, NULL},
  // STATE_RUN_WARMUP
  {1000, 11, 1, STATE_CONFIG, STATE_RESET, state_runWarmUp, delay_cb},
  // STATE_CONFIG
  {1000, 1, 4, STATE_MAIN, STATE_CALIBRATE, state_config, delay_cb},
  // STATE_CALIBRATE
  {1000, 200, 1, STATE_VERIFY, STATE_RESET, state_calibrate, NULL},
  // STATE_VERIFY
  {1000, 1, 1, STATE_MAIN, STATE_INIT_WARMUP, state_verify, delay_cb},
  // STATE_MAIN
  {1000, 1, 0, STATE_MAIN, STATE_RESET, state_main, NULL},
  // STATE_RESET
  {UINT32_MAX, 1, 0, STATE_RESET, STATE_RESET, state_reset, NULL}
};
uint32_t time = 0;

/**************************************
 * Objects
 **************************************/
LiquidCrystal_I2C display(0x27, 20, 4);
TFSM Fsm(state_table, sizeof(state_table) / sizeof(TFSM::ST_STATE));
MQ3 Mq3(A3);


/***************************/
/* State actions Functions */
/***************************/

void delay_cb(void)
{
  Serial.print(String(millis()/1000) + "  |  ");
  Serial.println("display cleared");
  display.clear();
}

void state_initWarmUp(void)
{
  Serial.print(String(millis()/1000) + "  |  ");
  Serial.println("Warming up...");

  display.setCursor(1,0);
  display.print("Warming up...");
}

void state_runWarmUp(void)
{
  const int16_t timer = Fsm.get_current_steps() - 1;
  const unsigned int seconds = timer % 60;
  const unsigned int minutes = ((timer - seconds) / 60) % 60;
  char str_minutes[3] = {'\0'};
  char str_seconds[3] = {'\0'};

  sprintf(str_minutes, "%02d", minutes);
  sprintf(str_seconds, "%02d", seconds);

  Serial.print(String(millis()/1000) + "  |  ");
  Serial.print(str_minutes);
  Serial.print(':');
  Serial.println(str_seconds);

  display.setCursor(6,1);
  display.print(str_minutes);
  display.setCursor(7,1);
  display.print(":");
  display.setCursor(8,1);
  display.print(str_seconds);
}

void state_config(void)
{
  if (EEPROM.read(0) == EEPROM_VALID_CONFIG)
  {
    EEPROM.get(1, Mq3.R0);
    Serial.print(String(millis()/1000) + "  |  ");
    Serial.println("Loaded Configuration  |  [R0 = " + String(Mq3.R0, 2) + "]");
  }
  else
  {
    Mq3.clear_calibration();
    Serial.print(String(millis()/1000) + "  |  ");
    Serial.println("No configuration found");
    Fsm.set_alt_transition();
  }
}

void state_calibrate(void)
{
  float val, volts, r0;

  Mq3.calibrate(val, volts, r0);

  Serial.print(String(millis()/1000) + "  |  ");
  Serial.println("Calibrating... Keep MQ3 in clean air!");
  Serial.print("Sensor value = ");
  Serial.print(val);
  Serial.print("  |  ");
  Serial.print("sensor volts = ");
  Serial.print(volts);
  Serial.print("V  |  calib R0 = ");
  Serial.println(r0);
}

void state_verify(void)
{
  float precision;

  Serial.print(String(millis()/1000) + "  |  ");

  if (Mq3.check_calibration(1.0f, precision))
  {
    EEPROM.write(0, EEPROM_VALID_CONFIG);
    EEPROM.put(1, Mq3.R0);
    Serial.print("Calibrated " + String(precision, 2) + "%  |  ");
    Serial.println("[R0 = " + String(Mq3.R0, 2) + "]");
  }
  else
  {
    Serial.println("Error too high!");
    Serial.println("Error: " + String(precision, 2) + "%");
    Fsm.set_alt_transition();
  }
}

void state_main(void)
{
  float val, volts, ratio;

  Mq3.measure(val, volts, ratio);

  const float mgL = pow(0.4 * ratio, -1.431);
  const float gdL = mgL * 0.0001;

  Serial.print(String(millis()/1000) + "  |  ");
  Serial.print("Sensor value = ");
  Serial.print(val);
  Serial.print("  |  sensor_volt = ");
  Serial.print(volts);
  Serial.print("  |  mg/L = ");
  Serial.print(mgL);
  Serial.print("  |  ppm = ");
  Serial.println(gdL);
}

void state_reset(void)
{
  Serial.print(String(millis()/1000) + "  |  ");
  Serial.println("Unexpected error occured, resetting when watchdog expires...");
}


/**************************************
 * System functions
 **************************************/

void setup(void)
{
  Serial.begin(9600);

  display.init();
  display.backlight();

  Mq3.init();

  wdt_disable();
  delay(3000);
  wdt_enable(WDTO_8S); // 8s Watchdog
}

void loop(void)
{
  if (millis() - time > Fsm.get_current_cycle())
  {
    time = millis();

    Fsm.run();

    wdt_reset();
  }
}