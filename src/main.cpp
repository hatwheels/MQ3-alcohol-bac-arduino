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
 *          TODO: - Comment state action functions.
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
#define WARMUP_PERIOD_SEC (60*60)
#define CALIBRATION_STEPS 200
#define DYNAMIC_CONFIG false
#if !DYNAMIC_CONFIG
  #define PREFCONFIG_R0 120.0
#endif

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
  {1000, WARMUP_PERIOD_SEC+1, 1, STATE_CONFIG, STATE_RESET, state_runWarmUp, delay_cb},
  // STATE_CONFIG
  {1000, 1, 4, STATE_MAIN, STATE_CALIBRATE, state_config, delay_cb},
  // STATE_CALIBRATE
  {1000, CALIBRATION_STEPS, 1, STATE_VERIFY, STATE_RESET, state_calibrate, delay_cb},
  // STATE_VERIFY
  {1000, 1, 1, STATE_MAIN, STATE_CONFIG, state_verify, delay_cb},
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
  const int16_t hours = timer / 3600;
  const int16_t minutes = (timer - hours * 3600) / 60;
  const int16_t seconds = timer - hours * 3600 - minutes * 60;
  char str_hours[3] = {'\0'};
  char str_minutes[3] = {'\0'};
  char str_seconds[3] = {'\0'};

  sprintf(str_hours, "%02d", hours);
  sprintf(str_minutes, "%02d", minutes);
  sprintf(str_seconds, "%02d", seconds);

  Serial.print(String(millis()/1000) + "  |  ");
  Serial.print(str_hours);
  Serial.print(':');
  Serial.print(str_minutes);
  Serial.print(':');
  Serial.println(str_seconds);

  display.setCursor(4,1);
  display.print(str_hours);
  display.setCursor(6,1);
  display.print(":");
  display.setCursor(7,1);
  display.print(str_minutes);
  display.setCursor(9,1);
  display.print(":");
  display.setCursor(10,1);
  display.print(str_seconds);
}

void state_config(void)
{
  if (EEPROM.read(0) == EEPROM_VALID_CONFIG)
  {
    double precision;

    EEPROM.get(1, Mq3.R0);
    EEPROM.get(1 + sizeof(Mq3.R0), precision);

    Serial.print(String(millis()/1000) + "  |  ");
    Serial.println("Loaded Configuration  |  [R0 = " + String(Mq3.R0, 2) + "] with precision " + String(precision, 2));

    display.setCursor(1,0);
    display.print("Loaded Config.");
    display.setCursor(0,1);
    display.print("R0: " + String(round(Mq3.R0)) + " E: " + String(precision, 2) + "%");
  }
  else
  {
#if DYNAMIC_CONFIG
    Serial.print(String(millis()/1000) + "  |  ");
    Serial.println("No configuration found");

    display.setCursor(0,0);
    display.print("No config. found");

    Fsm.set_alt_transition();
#else
    Mq3.R0 = PREFCONFIG_R0;

    EEPROM.write(0, EEPROM_VALID_CONFIG);
    EEPROM.put(1, Mq3.R0);
    EEPROM.put(1 + sizeof(Mq3.R0), 0.0f);

    Serial.print(String(millis()/1000) + "  |  ");
    Serial.println("Set static configuration  | [R0 = " + String(Mq3.R0, 2) + "]");
    display.setCursor(2,0);
    display.print("Set config.:");
    display.setCursor(4,1);
    display.print("R0: " + String(round(Mq3.R0)));
#endif // #if DYNAMIC_CONFIG
  }
  Mq3.clear_calibration();
}

void state_calibrate(void)
{
  const char msg[] = "Calibrating... Keep MQ3 in clean air! ";
  char str_buf[17] = {0};
  const uint16_t step = CALIBRATION_STEPS - Fsm.get_current_steps() + 1;
  const uint8_t id = (step - 1) % sizeof(msg);
  const uint8_t split_len = sizeof(msg) - id - 1;
  uint32_t val;
  double volts, r0;

  Mq3.calibrate(val, volts, r0);

  Serial.print(String(millis()/1000) + "  |  ");
  Serial.println(msg);
  Serial.print("Sensor value = ");
  Serial.print(val);
  Serial.print("  |  ");
  Serial.print("sensor volts = ");
  Serial.print(volts);
  Serial.print("V  |  calib R0 = ");
  Serial.print(r0);
  Serial.print(" | Step = ");
  Serial.println(step);

  display.setCursor(0,0);
  if (split_len > 15)
  {
    strncpy(str_buf, &msg[id], 16);
    display.print(str_buf);
  }
  else
  {
    strncpy(str_buf, &msg[id], split_len);
    display.print(str_buf);
    display.setCursor(split_len, 0);
    strncpy(str_buf, msg, 16 - split_len);
    str_buf[16 - split_len] = '\0';
    display.print(str_buf);
  }
  sprintf(str_buf, "R0: %ld", round(r0));
  memset(&str_buf[strlen(str_buf)], (int)' ', 9 - strlen(str_buf));
  sprintf(&str_buf[9], "%3d/200", step);
  display.setCursor(0,1);
  display.print(str_buf);
}

void state_verify(void)
{
  double precision;

  Serial.print(String(millis()/1000) + "  |  ");

  if (Mq3.check_calibration(1.0f, precision))
  {
    EEPROM.write(0, EEPROM_VALID_CONFIG);
    EEPROM.put(1, Mq3.R0);
    EEPROM.put(2, precision);

    Serial.print("Calibrated " + String(precision, 2) + "%  |  ");
    Serial.println("[R0 = " + String(Mq3.R0, 2) + "]");

    display.setCursor(0,0);
    display.print("Calibrated " + String(precision, 1) + "%");
    display.setCursor(0,1);
    display.print("R0: " + String(Mq3.R0, 2));

    Mq3.clear_calibration();
  }
  else
  {
    Serial.println("Error too high!");
    Serial.println("Error: " + String(precision, 2) + "%");

    display.setCursor(0,0);
    display.print("Error too high!");
    display.setCursor(2,1);
    display.println("Error: " + String(precision, 2) + "%");

    Fsm.set_alt_transition();
  }
}

void state_main(void)
{
  uint32_t val;
  double volts, ratio;

  Mq3.measure(val, volts, ratio);

  const double mgL = pow(0.4 * ratio, -1.431);
  char str_buf[16] = {0};

  Serial.print(String(millis()/1000) + "  |  ");
  Serial.print("Sensor value = ");
  Serial.print(val);
  Serial.print("  |  sensor_volt = ");
  Serial.print(volts);
  Serial.print("  |  mg/L = ");
  Serial.println(mgL);

  dtostrf(mgL, 5, 2, str_buf);
  Serial.println(strlen(str_buf));
  display.setCursor(8 - strlen(str_buf), 0);
  display.print(strcat(str_buf, " mg/L"));
}

void state_reset(void)
{
  Serial.print(String(millis()/1000) + "  |  ");
  Serial.println("Unexpected error occured, resetting when watchdog expires...");

  display.setCursor(0,0);
  display.print("Unexpected error");
  display.setCursor(2,1);
  display.print("Resetting...");
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