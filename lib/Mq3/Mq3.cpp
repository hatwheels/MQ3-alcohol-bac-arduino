/*******************************************************************************
 * @file    Mq3.cpp
 * @author  Kostas Markostamos
 * @date    31/03/2022
 * 
 * TODO:    - Add comments to constructor and methods
 *******************************************************************************/

#include <Arduino.h>
#include "Mq3.h"

MQ3::MQ3(uint8_t ain_pin)
{
  this->_ain_pin = ain_pin;
}

MQ3::~MQ3()
{
  this->_ain_pin = -1;

  this->_meas = { 0 };

  this->clear_calibration();

  this->_calib.precision = FLT_MAX;

  this->R0 = 0;
}

void MQ3::init(void)
{
  pinMode(this->_ain_pin, INPUT);
}

bool MQ3::measure(void)
{
  if (this->_ain_pin == -1)
  {
    return false;
  }

  uint32_t sum = 0;

  for (uint16_t x = 0 ; x < 1000 ; x++)
    sum += analogRead(this->_ain_pin);

  if (0 == sum)
  {
    return false;
  }

  this->_meas.avalue = sum / 1000;
  this->_meas.volts = this->_meas.avalue / 1024.0 * 5.0;
  this->_meas.RS = ((5.0 * R) / this->_meas.volts) - R;

  return true;
}

bool MQ3::measure(uint32_t &val, double &volts, double &rs)
{
  if (this->measure())
  {
    val = this->_meas.avalue;
    volts = this->_meas.volts;
    rs = this->_meas.RS;

    return true;
  }

  return false;
}

bool MQ3::is_valid(const double r0)
{
  // corresponds to ~[1.0V ... 0.1V]
  return (r0 > 300.0 && r0 < 4000.0) ? true : false;
}

bool MQ3::is_valid(void)
{
  return is_valid(this->R0);
}

bool MQ3::calibrate(void)
{
  if (this->measure())
  {
    const double R0 = this->_meas.RS / 60.0;

    this->_calib.n++;
    if (this->_calib.pAvalues == NULL)
      this->_calib.pAvalues = (double *) malloc(sizeof(double) * this->_calib.n);
    else
      this->_calib.pAvalues = (double *) realloc(this->_calib.pAvalues, sizeof(double) * this->_calib.n);
    this->_calib.pAvalues[this->_calib.n - 1] = R0;

    return true;
  }

  return false;
}

bool MQ3::calibrate(uint32_t &val, double &volts, double &r0)
{
  if (this->calibrate())
  {
    val = this->_meas.avalue;
    volts = this->_meas.volts;
    r0 = this->_calib.pAvalues[this->_calib.n - 1];

    return true;
  }

  return false;
}

bool MQ3::check_calibration(const double threshold)
{
  if (this->_calib.n == 0 || this->_calib.pAvalues == NULL)
    return false;

  double mean = .0, sd = .0;

  for (uint16_t i = 0; i < this->_calib.n; i++)
    mean += this->_calib.pAvalues[i];
  mean /= this->_calib.n;

  if (!this->is_valid(mean))
    return false;

  for (uint16_t i = 0; i < this->_calib.n; i++)
    sd += pow(this->_calib.pAvalues[i] - mean, 2);
  sd = sqrt(sd / this->_calib.n);

  // "Gauss" curve, 99.7% of data falls within 3 standard deviations.
  // So calculate the error of 99.7% of the data.
  this->_calib.precision = ((3 * sd) / mean) * 100;

  if (this->_calib.precision > threshold)
    return false;

  this->R0 = mean;

  return true;
}

bool MQ3::check_calibration(const double threshold, double &precision)
{
  bool result = this->check_calibration(threshold);

  precision = this->_calib.precision;

  return result;
}

void MQ3::clear_calibration(void)
{
  if (this->_calib.pAvalues != NULL)
  {
    memset(this->_calib.pAvalues, 0, sizeof(double) * this->_calib.n);
    free(this->_calib.pAvalues);
  }
  this->_calib.n = 0;
}
