#include "NFA.h"
#include "vector.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

Vector getNextStates_id(State state, Symbol symbol);

void Init_NFA(NFA *nfa)
{
	// Initialize states vector
	Vector_Init(&nfa->states, sizeof(State));
	
	// Initialize alphabet flags
	for (int i = 0; i < MAX_ALPHABET_SIZE; i++) {
		nfa->isInAlphabet[i] = false;
	}

	// Initialize initial state
	nfa->initialState_id = 0;

	// Initialize final states
	// Vector_Init(&nfa->finalStates_id, sizeof(int));
}

void copy_StateID(char *src, int end, StateID *out)
{
	int i = 0;
	for (; i < end && i < MAX_STATE_ID_SIZE; i++)
		(*out)[i] = src[i];
	(*out)[i] = '\0';
}

bool eq_StateID(StateID A, StateID B)
{
	for (int i = 0; i < MAX_STATE_ID_SIZE; i++)
		if (A[i] != B[i])
			return false;
	return true;
}

bool eq_State(const void *a, const void *b)
{
	const State StateA = *(const State *)a;

	const State StateB = *(const State *)b;

	return eq_StateID(StateA.id, StateB.id);
}

bool eq_Int(const void *a, const void *b)
{
	const int intA = *(const int *)a;

	const int intB = *(const int *)b;

	return intA == intB;

}

State Init_State(StateID id)
{
	State retState;
	bool hasEnded = false;
	for (int i = 0; i < MAX_STATE_ID_SIZE; i++)
		retState.id[i] = id[i];

	retState.isAccept = false;

	Vector_Init(&retState.epsilon_transitions, sizeof(Transition));
	Vector_Init(&retState.transitions, sizeof(Transition));

	return retState;
}

Vector Get_Epsilon_Closure(NFA *nfa, Vector states_id)
{
	Vector epsilon_closure;
	Vector_Init(&epsilon_closure, sizeof(int));
	Vector stack;
	Vector_Init(&stack, sizeof(int));
	
	for (int i = 0; i < Vector_Size(&states_id); i++) {
		int current_id;
		Vector_Get_Copy(&states_id, i, &current_id);

		Vector_Push(&epsilon_closure, &current_id);
		Vector_Push(&stack, &current_id);
	}

	while (!Vector_IsEmpty(&stack)) {
		int reachableStack_id;
		Vector_Pop(&stack, &reachableStack_id);

		State currentState;
		Vector_Get_Copy(&nfa->states, reachableStack_id, &currentState);
		for (int i = 0; i < Vector_Size(&currentState.epsilon_transitions); i++) {
			Transition tempTransition;
			Vector_Get_Copy(&currentState.epsilon_transitions, i, &tempTransition);

			bool is_in_epsilon_closure = false;
			for (int j = 0; j < Vector_Size(&epsilon_closure); j++) { // linear search
				int compIndex;
				Vector_Get_Copy(&epsilon_closure, j, &compIndex);
				if (tempTransition.to_id == compIndex) {
					is_in_epsilon_closure = true;
					break;
				}
			}

			if (!is_in_epsilon_closure) {
				int reachable_id = tempTransition.to_id;
				Vector_Push(&epsilon_closure, &reachable_id);
				Vector_Push(&stack, &reachable_id);
			}
		}
	}

	Vector_Free(&stack);

	return epsilon_closure;
}

Vector move(NFA *nfa, Vector states_id, Symbol symbol)
{
	Vector result;
	Vector_Init(&result, sizeof(int));
	
	for (int i = 0; i < states_id.size; i++) {
		int state_id;
		Vector_Get_Copy(&states_id, i, &state_id);

		State currentState;
		Vector_Get_Copy(&nfa->states, state_id, &currentState);

		for (int j = 0; j < Vector_Size(&currentState.transitions); j++) {
			Transition t;
			Vector_Get_Copy(&currentState.transitions, j, &t);
			if (t.symbol == symbol) {
				if (Vector_Find(&result, &t.to_id, eq_Int) == -1) {
					Vector_Push(&result, &t.to_id);
				}
			}
		}
	}

	return result;
}

Vector getNextStates_id(State state, Symbol symbol)
{
	Vector nextStates_id;
	Vector_Init(&nextStates_id, sizeof(int));
	for (int i = 0; i < state.transitions.size; i++) {
		Transition tempTransition;
		Vector_Get_Copy(&state.transitions, i, &tempTransition);

		if (tempTransition.symbol == symbol) {
			int nextState_id = tempTransition.to_id;
			Vector_Push(&nextStates_id, &nextState_id);
		}
	}
	return nextStates_id;
}

bool accepts(NFA *nfa, char *string)
{
	Vector initialState_vector;
	Vector_Init(&initialState_vector, sizeof(int));
	Vector_Push(&initialState_vector, &nfa->initialState_id);

	Vector currentStates_id = Get_Epsilon_Closure(nfa, initialState_vector);

	Vector_Free(&initialState_vector);

	for (int i = 0; string[i]; i++) {
		Vector moveStates = move(nfa, currentStates_id, string[i]);
		Vector_Free(&currentStates_id);
		currentStates_id = Get_Epsilon_Closure(nfa, moveStates);

		Vector_Free(&moveStates);

		if (Vector_IsEmpty(&currentStates_id)) {
			Vector_Free(&currentStates_id);
			return false;
		}
	}

	for (int i = 0; i < Vector_Size(&currentStates_id); i++) {
		int currentState_id;
		Vector_Get_Copy(&currentStates_id, i, &currentState_id);
		State currentState;
		Vector_Get_Copy(&nfa->states, currentState_id, &currentState);

		if (currentState.isAccept) {
			Vector_Free(&currentStates_id);
			return true;
		}
	}

	Vector_Free(&currentStates_id);
	return false;
}


void parseQLine(char *line, NFA *nfa)
{
	int start = 0;
	int end = 0;
	int i = 0;

	while (true) {
		if (line[i] == ',' || line[i] == '\0') {
			StateID s = {0};
			copy_StateID(&line[start], end, &s);

			State newState = Init_State(s);

			if (Vector_Find(&nfa->states, &newState, eq_State) != -1) {
				fprintf(stderr, "Repeated states in Q\n");
				exit(1);
			}

			Vector_Push(&nfa->states, &newState);

			if (line[i] == '\0')
				break;
			
			start = i + 1;
			end = -1;
		}
		end++;
		i++;
	}
}

void parseAlphabetLine(char *line, NFA *nfa)
{
	int i = 0;

	while (line[i] != '\0') {
		char c = line[i];
		if (i % 2 == 0) {
			if (nfa->isInAlphabet[c]) {
				fprintf(stderr, "Duplicate symbols in alphabet\n");
				exit(2);
			}
			nfa->isInAlphabet[c] = true;
		} else {
			if (c != ',') {
				fprintf(stderr, "Alphabet formated incorrectly\n");
				exit(2);
			}
		}
		i++;
	}
}

void parseSLine(char *line, NFA *nfa)
{
	StateID s = {0};
	State initialStateDummy;
	
	int i = 0;

	for(;i < MAX_STATE_ID_SIZE && line[i] != '\0'; i++) {
		s[i] = line[i];
		initialStateDummy.id[i] = line[i];
	}
	
	s[i] = '\0';
	initialStateDummy.id[i] = line[i];

	int initialState_id = Vector_Find(&nfa->states, &initialStateDummy, eq_State);
	if (initialState_id == -1) {
		fprintf(stderr, "Initial State not in States\n");
		exit(2);
	}

	nfa->initialState_id = initialState_id;

}

void parseFLine(char *line, NFA *nfa)
{
	int start = 0;
	int i = 0;

	while (true) {
		if (line[i] == ',' || line[i] == '\0') {
			State dummyState;
			copy_StateID(&line[start], i, &dummyState.id);

			int finalState_id = Vector_Find(&nfa->states, &dummyState, eq_State);
			if (finalState_id == -1) {
				fprintf(stderr, "Final state %s is not in Q\n", dummyState.id);
				exit(1);
			}
			
			State *stateRef = Vector_Get(&nfa->states, finalState_id);

			stateRef->isAccept = true;
			

			if (line[i] == '\0')
				break;
			
			start = i + i;
		}
		i++;
	}
}
void parseDeltaLine(char *line, NFA *nfa)
{
	State fromDummy, toDummy;
	Symbol symbol;

	sscanf(line, "%3[^,],%1[^.],%3s", &fromDummy.id, &symbol, &toDummy.id);

	int from_id = Vector_Find(&nfa->states, &fromDummy, eq_State);
	if (from_id == -1) {
		fprintf(stderr, "State %s in delta was not defined\n", fromDummy.id);
		exit(2);
	}
	int to_id = Vector_Find(&nfa->states, &toDummy, eq_State);
	if (to_id == -1) {
		fprintf(stderr, "State %s in delta was not defined\n", toDummy.id);
		exit(2);
	}

	Transition newTransition = { .to_id = to_id, .symbol = symbol };

	State *fromState = Vector_Get(&nfa->states, from_id);

	if (symbol == '\0')
		Vector_Push(&fromState->epsilon_transitions, &newTransition);
	else {
		if (!nfa->isInAlphabet[symbol]) {
			fprintf(stderr, "Symbol %c not in alphabet\n", symbol);
			exit(2);
		}
		Vector_Push(&fromState->transitions, &newTransition);
	}
}

void Fill_NFA_From_File(NFA *nfa, FILE* file)
{
	char *line = NULL;
	size_t cap = 0;
	ssize_t n;

	int line_id = 0;

	while ((n = getline(&line, &cap, file)) != -1) {
		line[n - 1] = '\0';
		switch (line_id) {
			case 0:
				parseQLine(line, nfa);
				break;
			case 1:
				parseAlphabetLine(line, nfa);
				break;
			case 2:
				parseSLine(line, nfa);
				break;
			case 3:
				parseFLine(line, nfa);
				break;
			default:
				parseDeltaLine(line, nfa);
				break;
		}
		line_id++;
	}
}
