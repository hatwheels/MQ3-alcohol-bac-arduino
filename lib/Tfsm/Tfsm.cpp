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
    free(this->_pStates);

  this->_n = n;
  this->_size = _n * sizeof(ST_STATE);
  this->_pStates = (ST_STATE *) malloc(_size);
  memcpy(this->_pStates, pStates, _size);
  this->_action_arg_type = _E_ARG_TYPE_NONE;
  memset(this->_str_action_arg, 0, sizeof(this->_str_action_arg));
  this->_alt_transition = false;
  this->_init();
}

TFSM::~TFSM()
{
  if (this->_pStates != NULL)
    free(this->_pStates);
  this->_n = 0;
  this->_size = 0;
  this->_state = {0};
  this->_action_arg_type = _E_ARG_TYPE_NONE;
  memset(this->_str_action_arg, 0, sizeof(this->_str_action_arg));
  this->_alt_transition = false;
}

void TFSM::_init(void)
{
  this->_state = this->_pStates[0];
  this->_alt_transition = false;
}

void TFSM::_init(ST_STATE state)
{
  this->_state = state;
  this->_alt_transition = false;
  if (NULL == this->_state.action_arg)
  {
    switch (this->_action_arg_type)
    {
      case _E_ARG_TYPE_CHAR_ARRAY:
        this->_state.action_arg = (void *) this->_str_action_arg;
        this->_action_arg_type = _E_ARG_TYPE_NONE;
        break;

      default:
        break;
    }
  }
}

void TFSM::run(void)
{
  if (this->_state.steps > 0)
  {
    if (this->_state.action != NULL)
      this->_state.action(this->_state.action_arg);
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

    this->_init(this->_pStates[s]);

    if (this->_state.action != NULL)
      this->_state.action(this->_state.action_arg);
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

void TFSM::force_transition(void)
{
  this->_state.steps = 0;
}

void TFSM::set_alt_transition(void)
{
  this->_alt_transition = true;
}

void TFSM::set_delay(int16_t delay)
{
  if (delay > 0)
  {
    this->_state.delay = delay;
  }
}

void TFSM::set_action_arg(const char * str_action_arg)
{
  if (str_action_arg != NULL && strlen(str_action_arg) > 0 && strlen(str_action_arg) < 34)
  {
    memset(this->_str_action_arg, 0, sizeof(this->_str_action_arg));
    strcpy(this->_str_action_arg, str_action_arg);
    this->_action_arg_type = _E_ARG_TYPE_CHAR_ARRAY;
  }
}

void TFSM::set_all(
  bool alt_transition /*=false*/,
  int16_t delay /*=-1*/,
  bool force_transition /*=false*/,
  const char *str_action_arg /*=NULL*/
)
{
  this->set_action_arg(str_action_arg);

  if (alt_transition)
  {
    this->set_alt_transition();
  }

  this->set_delay(delay);

  if (force_transition)
  {
    this->force_transition();
  }
}
