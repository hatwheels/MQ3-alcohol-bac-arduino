#include "FSM.h"

FSM::FSM(ST_STATE pStates[], size_t n)
{
  if (this->_pStates != NULL)
    free(_pStates);

  this->_n = n;
  this->_size = _n * sizeof(ST_STATE);
  this->_pStates = (ST_STATE *) malloc(_size);
  memcpy(_pStates, pStates, _size);
  for (uint16_t i = 0; i < this->_n; ++i)
  {
    if (this->_pStates[i].cycle > 7000)
      this->_pStates[i].cycle = 7000;
  }
  this->init();
}

FSM::~FSM()
{
  if (this->_pStates != NULL)
    free(this->_pStates);
  this->_n = 0;
  this->_size = 0;
  this->_state = {0};
  this->_alt_transition = false;
}

void FSM::init(void)
{
  this->_state = this->_pStates[0];
  this->_alt_transition = false;
}

void FSM::init(ST_STATE state)
{
  this->_state = state;
  this->_alt_transition = false;
}

void FSM::run(void)
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

uint32_t FSM::get_current_cycle(void)
{
  return this->_state.cycle;
}

int16_t FSM::get_current_steps(void)
{
  return this->_state.steps;
}

void FSM::set_alt_transition(void)
{
  this->_alt_transition = true;
}