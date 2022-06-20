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
    void measure(uint32_t &val, double &volts, double &rs);
    bool is_valid(void);
    bool is_valid(const double r0);
    void calibrate(void);
    void calibrate(uint32_t &val, double &volts, double &r0);
    bool check_calibration(const double threshold);
    bool check_calibration(const double threshold, double &precision);
    void clear_calibration(void);
    double R0 = .0;

  private:
    typedef struct {
      uint32_t avalue;
      double volts;
      double RS;
    } ST_MEAS;
    typedef struct {
      uint32_t n;
      double * pAvalues;
      double precision;
    } ST_CALIB;
    uint8_t _ain_pin = -1;
    ST_MEAS _meas = { .avalue = 0, .volts = .0, .RS = .0, };
    ST_CALIB _calib = { .n = 0, .pAvalues = NULL, .precision = DBL_MAX };
};

#endif // _MQ3_H_