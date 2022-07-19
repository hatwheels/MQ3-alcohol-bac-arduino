/********************************************************************************
 * @file    Tfsm.h
 * @author  Kostas Markostamos
 * @date    31/03/2022
 * @brief   Defines a class modeling a Timed Finite State Machine. Originally
 *          modeled independently as a modification of a Finite State Machine
 *          with time as input. In search of theoretical background, the term
 *          Timed Finite State Machine was found. More information can be found
 *          in the following links:
 *            - https://arxiv.org/pdf/1408.5967.pdf
 *            - https://en.wikipedia.org/wiki/Timed_automaton
 * 
 *          NOTE: The Tfsm class is not modelling the theory 1:1. It was
 *          originally created as an abstraction for processes in embedded apps
 *          that are implemented as state machines that are often dependent on
 *          cycle times. The drive for its modeling came up during the MQ3 
 *          alcohol sensor calibration and BAC measurement.
 *          The class model is new and, depending on time availability, will see
 *          incremental additions and harmonization to its theoretical model.
 *          The end goal, is to create a deterministic Timed Finite State Machine
 *          abstraction model for real life, mainly embedded, apps.
 *
 *          States: The state table is an input to the object and is copied to
 *          a private dynamic array "_pStates"
 *          Time Inputs:
 *            - cycle: The cycle time of the state in ms. Its periodicity. The
 *                     action of the state is executed every cycle. Currently
 *                     elapsing of the cycle time must be done externally. If
 *                     the cycle time has elapsed, the state action must be
 *                     called.
 *            - steps: The number of cycles left for the machine to stay in its
 *                     current state. It is decremented at each "cycle" of the
 *                     current state.
 *            - delay: A transition delay in number of cycles. It starts being
 *                     decremented every cycle after "steps" reached 0. The
 *                     purpose it serves is a practical one, such as waiting
 *                     for a driver to initialize after the current step, but
 *                     before the next one.
 *          Inputs: Currently there aren't any variables for inputs. External
 *                  variables can be used inside the state action as inputs.
 *          Transitions: There's a limit of two transitions. A primary one and
 *                       an alternate one. The primary one is by default used
 *                       for state transitioning. The alternate one must be 
 *                       manually set with the set_alt_transition() method, for
 *                       example in the action callback.
 *                       The "primary" transition is by default triggered at the
 *          State action: "action" is a callback, where the logic/action
 *                        of the state is assigned to. It is run at every cycle.
 *          State action argument: Parameter passed to a state action before a
 *                        transition. It is meant for specific handling of a
 *                        a state dependent on the previous state, instead
 *                        of a general handling of the state no matter what the
 *                        previous state was. The argument is generic (void *
 *                        pointer) that shall be casted in the state action
 *                        callback. Currently, only char array arguments are
 *                        implemented.
 *          Delay action: It is a special callback of the transition delay and
 *                        serves a practical purpose, for instance clearing a
 *                        LCD display during a state transition. It is run at
 *                        the end of the delayed transition.
 ********************************************************************************/


#ifndef _TFSM_H
#define _TFSM_H

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

class TFSM
{
  public:
    typedef void (*state_action_fp)(void * action_arg);
    typedef void (*state_delay_fp)(void);
    typedef struct {
      uint32_t cycle;
      int32_t steps;
      int16_t delay;
      uint8_t primary_transition;
      uint8_t alternate_transition;
      state_action_fp action;
      void * action_arg;
      state_delay_fp delay_cb;
    } ST_STATE;
    TFSM(ST_STATE pStates[], size_t size);
    ~TFSM();
    void run(void);
    uint32_t get_current_cycle(void);
    int32_t get_current_steps(void);
    void force_transition(void);
    void set_alt_transition(void);
    void set_delay(int16_t delay);
    void set_action_arg(const char str_action_arg[33]);
    void set_all(
      bool alt_transition=false,
      int16_t delay=-1,
      bool force_transition=false,
      const char *str_action_arg=NULL
    );

  private:
    typedef enum {
      _E_ARG_TYPE_NONE = 0,
      _E_ARG_TYPE_CHAR_ARRAY = 1
    } _E_ARG_TYPE;
    void _init(void);
    void _init(ST_STATE state);
    ST_STATE *_pStates;
    size_t _n;
    size_t _size;
    ST_STATE _state;
    _E_ARG_TYPE _action_arg_type;
    char _str_action_arg[33];
    bool _alt_transition;
};

#endif // _TFSM_H