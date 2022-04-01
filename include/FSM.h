#ifndef _FSM_H
#define _FSM_H

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

class FSM
{
  public:
    typedef void (*state_fp)(void);
    typedef enum {
      TRANSITION_NONE,
      TRANSITION_SUCCESS,
      TRANSITION_FAILURE
    } E_TRANSITION;
    typedef struct {
      uint32_t cycle;
      int16_t steps;
      int16_t delay;
      uint8_t primary_transition;
      uint8_t alternate_transition;
      state_fp action;
      state_fp delay_cb;
    } ST_STATE;
    FSM(ST_STATE pStates[], size_t size);
    ~FSM();
    void init(void);
    void init(ST_STATE state);
    void transition(void);
    void run(void);
    uint32_t get_current_cycle(void);
    int16_t get_current_steps(void);
    void set_alt_transition(void);

  private:
    ST_STATE *_pStates;
    size_t _n;
    size_t _size;
    ST_STATE _state;
    bool _alt_transition;
};

#endif // _FSM_H