/*******************************************************************************
 * @file    Tfsm.cpp
 * @author  Kostas Markostamos
 * @date    31/03/2022
 * 
 * TODO:    - Add comments to constructor and methods
 *******************************************************************************/

#include "Tfsm.h"

TFSM::TFSM(ST_STATE pStates[], size_t n)
{
  if (this->_pStates != NULL)
    free(_pStates);

  this->_n = n;
  this->_size = _n * sizeof(ST_STATE);
  this->_pStates = (ST_STATE *) malloc(_size);
  memcpy(_pStates, pStates, _size);
  this->init();
}

TFSM::~TFSM()
{
  if (this->_pStates != NULL)
    free(this->_pStates);
  this->_n = 0;
  this->_size = 0;
  this->_state = {0};
  this->_alt_transition = false;
}

void TFSM::init(void)
{
  this->_state = this->_pStates[0];
  this->_alt_transition = false;
}

void TFSM::init(ST_STATE state)
{
  this->_state = state;
  this->_alt_transition = false;
}

void TFSM::run(void)
{
  if (this->_state.steps > 0)
  {
    if (this->_state.action != NULL)
      this->_state.action();
    --this->_state.steps;

    return;
  }

  if (this->_state.delay > 0)
  {
    if (--this->_state.delay == 0 && this->_state.delay_cb != NULL)
    {
      this->_state.delay_cb();
      this->_state.delay_cb = NULL;
    }
  }
  else
  {
    const uint8_t s = this->_alt_transition ? this->_state.alternate_transition : this->_state.primary_transition;

    if (this->_state.delay_cb != NULL)
      this->_state.delay_cb();

    this->init(this->_pStates[s]);

    if (this->_state.action != NULL)
      this->_state.action();
    --this->_state.steps;
  }
}

uint32_t TFSM::get_current_cycle(void)
{
  return this->_state.cycle;
}

int32_t TFSM::get_current_steps(void)
{
  return this->_state.steps;
}

void TFSM::set_alt_transition(void)
{
  this->_alt_transition = true;
}

void TFSM::force_transition(void)
{
  this->_state.steps = 0;
}

void TFSM::set_delay(int16_t delay)
{
  this->_state.delay = delay;
}