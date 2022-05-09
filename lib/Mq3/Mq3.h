/*******************************************************************************
 * @file    Mq3.h
 * @author  Kostas Markostamos
 * @date    31/03/2022
 * @brief   Defines a class for the use of MQ3 alcohol sensors.
 *          A MQ3 sensor can be initialized with the pin connected to,
 *          calibrated for valid measurements
 *          and measure the Breadth Alcohol Concentration (BAC) in the air.
 *
 *          R0 is the calibrated value of MQ3 for valid measurements.
 *          If pre-calibrated, then R0 can be assigned with the value directly
 *          and do measurements without calibrating first.
 *
 *          Calibration is an iterative process and calibrate() should be
 *          called numerous times. After completing the calibration, the
 *          calibration results must be validated with check_calibration().
 *          check_calibration() returns "true" for a valid calibration and the
 *          calibrated R0. In case the calibration failed, then it must be
 *          cleared with clear_calibration() first before retrying calibration.
*******************************************************************************/

#ifndef _MQ3_H
#define _MQ3_H

#include <stdint.h>
#include <float.h>

class MQ3
{
  public:
    MQ3(uint8_t ain_pin);
    ~MQ3();
    static const uint16_t R = 4700U;
    void init(void);
    void measure(void);
    void measure(float &val, float &volts, float &ratio);
    void calibrate(void);
    void calibrate(float &val, float &volts, float &r0);
    bool check_calibration(const float threshold);
    bool check_calibration(const float threshold, float &precision);
    void clear_calibration(void);
    float R0 = 0.0f;

  private:
    typedef struct {
      float avalue;
      float volts;
      float RS;
    } ST_MEAS;
    typedef struct {
      uint32_t n;
      float * pAvalues;
      float precision;
    } ST_CALIB;
    uint8_t _ain_pin = -1;
    ST_MEAS _meas = { .avalue = 0.0f, .volts = 0.0f, .RS = 0.0f, };
    ST_CALIB _calib = { .n = 0, .pAvalues = NULL, .precision = FLT_MAX };
};

#endif // _MQ3_H_