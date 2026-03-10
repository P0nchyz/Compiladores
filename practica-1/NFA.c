#include "NFA.h"
#include "memory.h"
#include "vector.h"

void Init_NFA(NFA *nfa)
{
	// Initialize states vector
	Vector_Init(&nfa->states, sizeof(State));
	
	// Initialize alphabet flags
	for (int i = 0; i < sizeof(Symbol); i++) {
		Vector_Init(&nfa->transitionTable[i], sizeof(Transition));
	}

	// Initialize initial state
	proj_memset(nfa->initialState.id, 0, sizeof(nfa->initialState.id));

	// Initialize final states
	Vector_Init(&nfa->finalStates, sizeof(State));

	// Intialize transition tables
	for (int i = 0; i < sizeof(Symbol); i++) {
		Vector_Init(&nfa->transitionTable[i], sizeof(Transition));
	}

}

void Add_State(NFA *nfa, State state)
{
	return Vector_Push(&nfa->states, &state);
}

void Add_Final_State(NFA *nfa, State state)
{
	return Vector_Push(&nfa->finalStates, &state);
}

void Add_Transition(NFA *nfa, Transition transition)
{
	return Vector_Push(&nfa->transitionTable[transition.symbol], &transition);
}

