// ///////////////////////////////////////////////////////////////////////////
// StateMachine.h by Victor Dods, created 2006/03/19
// Copyright Victor Dods, licensed for unlimited use by Leap Motion Inc.
// ///////////////////////////////////////////////////////////////////////////

#if !defined(_STATEMACHINE_H_)
#define _STATEMACHINE_H_

#include <ostream>
#include <string>

typedef unsigned int StateMachineInput;

// StateMachine mechanism inputs
enum
{
    SM_ENTER                    = static_cast<StateMachineInput>(-1),
    SM_EXIT                     = static_cast<StateMachineInput>(-2),
    SM_INVALID                  = static_cast<StateMachineInput>(-3),
    SM_HIGHEST_USER_INPUT_VALUE = static_cast<StateMachineInput>(-4)
};

// This is a simple state machine class, which is driven by inputs.  An SM_ENTER or SM_EXIT input
// is sent when the state is entered (transitioned-to) or exited (transitioned-from).  Additional
// inputs can be user defined, as long as they are numerically less-than-or-equal-to
// SM_HIGHEST_USER_INPUT_VALUE.
//
// Each state corresponds to a single state-handling method which MUST handle all inputs.  The
// handler method should return true when it handles an input, and have a catch-all return of
// false, so that the state machine can properly warn/error on unhandled input.
//
// A state can transition-to any other state, but doesn't have to.  If a state transitions to
// another, that state handler will be immediately called.  This happens within a while-loop,
// whose termination condition is that the current state did not transition.  Thus it is possible
// to enter an infinite loop of mutually-transitioning-to states (so watch out).
//
// The way this class is used is as follows:
// - The class, call it Widget, which needs state machine functionality defines a member instance
//   of type StateMachine<Widget>.  This sets up all the method types and so forth.
// - A "state" in this state machine is a Widget method pointer.
// - The state machine must be Initialize()'ed before use -- you must specify the initial state.
// - To avoid a lot of repetetive typing, it is useful to make a preprocessor macro of the form
//   #define TRANSITION_TO(x) m_state_machine.SetNextState(&Widget::x, #x)
// - RunCurrentState(input) will initiate the while-loop process described above, using the
//   given input.
// - It is possible to set a "transition logger" to generate some debug spew for diagnostic
//   purposes.  This prints when states are entered and exited, and prints what inputs are
//   received.  The default status is no logging.  One input may be specified to ignore
//   if/when the logger is initialized, for example a "process frame" input that happens
//   many times per second, and would be considered debug spam.
template <typename OwnerClass>
class StateMachine
{
public:

    typedef bool (OwnerClass::*State)(StateMachineInput);

    StateMachine ();
    ~StateMachine ();

    /// @brief Indicates the owner class.
    /// @details The name of the owner class for purposes transition_logger_name varThe name variable
    /// name is used for transition logging.  NULL indicates that no logging should be done.
    void SetOwnerClass (OwnerClass* owner_class, std::string const &owner_class_name);
    bool IsInitialized () const { return m_current_state != NULL; }
    bool IsTransitionLoggerEnabled () const { return m_transition_logger != NULL; }
    std::ostream *TransitionLogger () const { return m_transition_logger; }
    StateMachineInput TransitionLoggerIgnoreInput () const { return m_transition_logger_ignore_input; }
    State CurrentState () const { return m_current_state; }
    /// Will return the empty string if CurrentState returns NULL.
    std::string const &CurrentStateName () const { return m_current_state_name; }
    State NextState () const { return m_next_state; }
    /// Will return the empty string if NextState returns NULL.
    std::string const &NextStateName () const { return m_next_state_name; }

    /// initial_state_name is used for transition logging.
    void Initialize (State initial_state, std::string const &initial_state_name);
    /// Specifying NULL for transition_logger indicates that logging will be disabled.
    void SetTransitionLogger (std::ostream *transition_logger, StateMachineInput transition_logger_ignore_input = SM_INVALID);
    void RunCurrentState (StateMachineInput input);
    void Shutdown ();

    /// state_name is used for transition logging.
    void SetNextState (State state, std::string const &state_name);

private:

    void LogTransition (StateMachineInput input) const;
    void RunCurrentStatePrivate (StateMachineInput input);

    //OwnerClass &m_owner_class;
    OwnerClass* m_owner_class;
    std::string m_owner_class_name;
    std::ostream *m_transition_logger;
    StateMachineInput m_transition_logger_ignore_input;
    bool m_is_running_a_state;
    State m_current_state;
    std::string m_current_state_name;
    State m_next_state;
    std::string m_next_state_name;
}; // end of class StateMachine

template <typename OwnerClass>
StateMachine<OwnerClass>::StateMachine ()
  :
  m_owner_class(NULL),
  m_transition_logger(NULL),
  m_transition_logger_ignore_input(SM_INVALID),
  m_is_running_a_state(false),
  m_current_state(NULL),
  m_next_state(NULL)
{ }

template <typename OwnerClass>
StateMachine<OwnerClass>::~StateMachine ()
{
    Shutdown();
}

template <typename OwnerClass>
void StateMachine<OwnerClass>::SetOwnerClass(OwnerClass* owner_class, std::string const &owner_class_name)
{
  m_owner_class = owner_class;
  m_owner_class_name = owner_class_name;
}

template <typename OwnerClass>
void StateMachine<OwnerClass>::Initialize (State initial_state, std::string const &initial_state_name)
{
    // just make sure this happens only once at the beginning
    assert(!m_is_running_a_state && "This method should not be used from inside a state");
    assert(m_current_state == NULL && "This state machine is already initialized");

    // set the current state and run it with SM_ENTER.
    m_current_state = initial_state;
    m_current_state_name = initial_state_name;
    this->RunCurrentStatePrivate(SM_ENTER);
}

template <typename OwnerClass>
void StateMachine<OwnerClass>::SetTransitionLogger (std::ostream *transition_logger, StateMachineInput transition_logger_ignore_input)
{
    assert(transition_logger_ignore_input == SM_INVALID || transition_logger_ignore_input <= SM_HIGHEST_USER_INPUT_VALUE);
    m_transition_logger = transition_logger;
    m_transition_logger_ignore_input = transition_logger_ignore_input;
}

template <typename OwnerClass>
void StateMachine<OwnerClass>::RunCurrentState (StateMachineInput input)
{
    assert(input <= SM_HIGHEST_USER_INPUT_VALUE && "Users are not allowed to send state-machine-defined input");
    this->RunCurrentStatePrivate(input);
}

template <typename OwnerClass>
void StateMachine<OwnerClass>::Shutdown ()
{
    assert(!m_is_running_a_state);

    // only actually shutdown if we're not already shutdown.
    if (m_current_state != NULL)
    {
        // run the current state with SM_EXIT and nullify it.
        this->RunCurrentStatePrivate(SM_EXIT);
        m_current_state = NULL;
        m_current_state_name.clear();
    }
}

template <typename OwnerClass>
void StateMachine<OwnerClass>::SetNextState (State state, std::string const &state_name)
{
    assert(m_is_running_a_state && "This method should only be used from inside a state");
    assert(m_current_state != NULL && "This state machine has not been initialized");

    // note: a NULL value for state is acceptable.  it is
    // effectively canceling an earlier requested transition.
    m_next_state = state;
    m_next_state_name = state_name;
}

template <typename OwnerClass>
void StateMachine<OwnerClass>::LogTransition (StateMachineInput input) const
{
    assert(input != SM_INVALID);

    if (m_transition_logger == NULL || input == m_transition_logger_ignore_input)
        return; // logging disabled or we want to ignore this particular input

    *m_transition_logger << m_owner_class_name << ": ";
    if (input == SM_ENTER)
        *m_transition_logger << "--> " << m_current_state_name;
    else if (input == SM_EXIT)
        *m_transition_logger << "<-- " << m_current_state_name;
    else
        *m_transition_logger << "input: " << input;
    *m_transition_logger << std::endl;
}

template <typename OwnerClass>
void StateMachine<OwnerClass>::RunCurrentStatePrivate (StateMachineInput input)
{
    assert(!m_is_running_a_state && "This method should not be used from inside a state");
    assert(m_current_state != NULL && "This state machine has not been initialized");

    // NULL is a sentinel value so we know if the state has transitioned
    m_next_state = NULL;
    m_next_state_name.clear();

    // if the state return true, the input was handled.  otherwise not.
    m_is_running_a_state = true;
    LogTransition(input);
#ifndef NDEBUG
    bool state_handled_the_input =
#endif
    ((*m_owner_class).*m_current_state)(input);
    m_is_running_a_state = false;
    // make sure that states always handle all input (with the exception of
    // the StateMachine mechanism inputs)
    if (input <= SM_HIGHEST_USER_INPUT_VALUE)
        assert(state_handled_the_input && "All user-defined state machine input must be handled");

    if (input == SM_EXIT && m_next_state != NULL)
        assert(false && "You must not transition while exiting a state");

    // if a transition was requested, perform the necessary exit/enter machinery.
    // this is a while-loop because you can transition during SM_ENTER.
    while (m_next_state != NULL)
    {
        // we have to save off the next state, because the forthcoming SM_EXIT
        // call to the current state might change m_next_state, and we need to
        // detect if they try to transition on SM_EXIT (which is not allowed).
        State real_next_state = m_next_state;
        std::string real_next_state_name = m_next_state_name;
        m_next_state = NULL;
        m_next_state_name.clear();
        // call the current state with SM_EXIT, ignoring the return value
        m_is_running_a_state = true;
        LogTransition(SM_EXIT);
        ((*m_owner_class).*m_current_state)(SM_EXIT);
        m_is_running_a_state = false;
        // if they requested a transition, assert
        assert(m_next_state == NULL && "You must not transition while exiting a state");

        // set the current state to the new state
        m_current_state = real_next_state;
        m_current_state_name = real_next_state_name;
        // call the current state with SM_ENTER, ignoring the return value
        m_is_running_a_state = true;
        LogTransition(SM_ENTER);
        ((*m_owner_class).*m_current_state)(SM_ENTER);
        m_is_running_a_state = false;
    }

    // since m_next_state is NULL, clear the next state name
    m_next_state_name.clear();
}

#endif // !defined(_STATEMACHINE_H_)

