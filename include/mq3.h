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