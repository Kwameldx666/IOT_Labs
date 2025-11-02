#include "fsm.hpp"

FiniteStateMachine::FiniteStateMachine(const State* states,
                                       uint8_t numStates,
                                       const Transition* transitions,
                                       uint8_t numTransitions,
                                       StateId initialState)
    : m_states(states)
    , m_numStates(numStates)
    , m_transitions(transitions)
    , m_numTransitions(numTransitions)
    , m_currentState(initialState)
    , m_previousState(initialState)
    , m_transitionCount(0) {
}

void FiniteStateMachine::begin() {
  // Execute entry action for initial state
  const State* state = getState(m_currentState);
  if (state != nullptr && state->onEntry != nullptr) {
    state->onEntry();
  }
}

void FiniteStateMachine::processEvent(EventId event) {
  const State* currentState = getState(m_currentState);
  if (currentState == nullptr) {
    return;
  }
  
  StateId nextState = m_currentState;
  
  // Option 1: Use transition table if available
  if (m_transitions != nullptr) {
    StateId transitionState = findTransition(m_currentState, event);
    if (transitionState != m_currentState) {
      nextState = transitionState;
    }
  }
  
  // Option 2: Use state handler
  if (nextState == m_currentState && currentState->handler != nullptr) {
    nextState = currentState->handler(event);
  }
  
  // Perform transition if state changed
  if (nextState != m_currentState) {
    transitionTo(nextState);
  }
}

const char* FiniteStateMachine::getCurrentStateName() const {
  const State* state = getState(m_currentState);
  return (state != nullptr) ? state->name : "UNKNOWN";
}

void FiniteStateMachine::forceState(StateId newState) {
  if (newState < m_numStates) {
    transitionTo(newState);
  }
}

const State* FiniteStateMachine::getState(StateId id) const {
  for (uint8_t i = 0; i < m_numStates; i++) {
    if (m_states[i].id == id) {
      return &m_states[i];
    }
  }
  return nullptr;
}

StateId FiniteStateMachine::findTransition(StateId fromState, EventId event) const {
  for (uint8_t i = 0; i < m_numTransitions; i++) {
    if (m_transitions[i].fromState == fromState && 
        m_transitions[i].event == event) {
      return m_transitions[i].toState;
    }
  }
  return fromState;  // No transition found, stay in current state
}

void FiniteStateMachine::transitionTo(StateId newState) {
  const State* oldState = getState(m_currentState);
  const State* nextState = getState(newState);
  
  if (nextState == nullptr) {
    return;  // Invalid state
  }
  
  // Execute exit action of current state
  if (oldState != nullptr && oldState->onExit != nullptr) {
    oldState->onExit();
  }
  
  // Update state
  m_previousState = m_currentState;
  m_currentState = newState;
  m_transitionCount++;
  
  // Execute entry action of new state
  if (nextState->onEntry != nullptr) {
    nextState->onEntry();
  }
}
#ifndef FSM_HPP
#define FSM_HPP

#include <stdint.h>

/**
 * @file fsm.hpp
 * @brief Finite State Machine (FSM) Framework
 * 
 * Generic FSM implementation with:
 * - State transitions based on events
 * - State entry/exit actions
 * - Transition table support
 * - State history tracking
 */

/**
 * @brief State identifier type
 */
typedef uint8_t StateId;

/**
 * @brief Event identifier type
 */
typedef uint8_t EventId;

/**
 * @brief State handler function signature
 * @param event Event that triggered the handler
 * @return Next state ID (or same state to stay)
 */
typedef StateId (*StateHandler)(EventId event);

/**
 * @brief State entry/exit action signature
 */
typedef void (*StateAction)();

/**
 * @struct State
 * @brief State definition with handlers and actions
 */
struct State {
  StateId id;
  const char* name;
  StateHandler handler;
  StateAction onEntry;
  StateAction onExit;
};

/**
 * @struct Transition
 * @brief State transition definition
 */
struct Transition {
  StateId fromState;
  EventId event;
  StateId toState;
};

/**
 * @class FiniteStateMachine
 * @brief Generic FSM implementation
 */
class FiniteStateMachine {
public:
  /**
   * @brief Constructor
   * @param states Array of state definitions
   * @param numStates Number of states
   * @param transitions Array of transitions (optional)
   * @param numTransitions Number of transitions
   * @param initialState Initial state ID
   */
  FiniteStateMachine(const State* states, 
                     uint8_t numStates,
                     const Transition* transitions = nullptr,
                     uint8_t numTransitions = 0,
                     StateId initialState = 0);
  
  /**
   * @brief Initialize FSM
   */
  void begin();
  
  /**
   * @brief Process event and transition if needed
   * @param event Event to process
   */
  void processEvent(EventId event);
  
  /**
   * @brief Get current state ID
   * @return Current state ID
   */
  StateId getCurrentState() const { return m_currentState; }
  
  /**
   * @brief Get current state name
   * @return State name string
   */
  const char* getCurrentStateName() const;
  
  /**
   * @brief Get previous state ID
   * @return Previous state ID
   */
  StateId getPreviousState() const { return m_previousState; }
  
  /**
   * @brief Get transition count
   * @return Number of transitions performed
   */
  uint32_t getTransitionCount() const { return m_transitionCount; }
  
  /**
   * @brief Force state change (bypass transitions)
   * @param newState Target state ID
   */
  void forceState(StateId newState);

private:
  const State* m_states;
  uint8_t m_numStates;
  const Transition* m_transitions;
  uint8_t m_numTransitions;
  
  StateId m_currentState;
  StateId m_previousState;
  uint32_t m_transitionCount;
  
  const State* getState(StateId id) const;
  StateId findTransition(StateId fromState, EventId event) const;
  void transitionTo(StateId newState);
};

#endif // FSM_HPP

