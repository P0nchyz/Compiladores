#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "NFA.h"
#include "vector/vector.h"

static Vector Get_Epsilon_Closure(const NFA *nfa, Vector states_id);
static Vector move(const NFA *nfa, Vector states_id, Symbol symbol);

static void parseQLine(NFA *nfa, const char *line);
static void parseAlphabetLine(NFA *nfa, const char *line);
static void parseSLine(NFA *nfa, const char *line);
static void parseFLine(const NFA *nfa, const char *line);
static void parseDeltaLine(const NFA *nfa, const char *line);

static State State_Init(StateID id);
static void copy_StateID(const char *src, int end, StateID *out);

static bool eq_StateID(const StateID A, const StateID B);
static bool eq_State(const void *a, const void *b);
static bool eq_Int(const void *a, const void *b);

void NFA_Init(NFA *nfa)
{
	// Initialize states vector
	Vector_Init(&nfa->states, sizeof(State));

	// Initialize alphabet flags
	for (int i = 0; i < MAX_ALPHABET_SIZE; i++) {
		nfa->isInAlphabet[i] = false;
	}

	// Initialize initial state
	nfa->initialState_id = 0;
}

bool NFA_Accepts(const NFA *nfa, char *string)
{
	Vector initialState_vector;
	Vector_Init(&initialState_vector, sizeof(size_t));
	Vector_Push(&initialState_vector, &nfa->initialState_id);

	Vector currentStates_id = Get_Epsilon_Closure(nfa, initialState_vector);

	Vector_Free(&initialState_vector);

	for (int i = 0; string[i]; i++) {
		Vector moveStates = move(nfa, currentStates_id, (Symbol)string[i]);
		Vector_Free(&currentStates_id);
		currentStates_id = Get_Epsilon_Closure(nfa, moveStates);

		Vector_Free(&moveStates);

		if (Vector_IsEmpty(&currentStates_id)) {
			Vector_Free(&currentStates_id);
			return false;
		}
	}

	for (size_t i = 0; i < Vector_Size(&currentStates_id); i++) {
		size_t currentState_id;
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

static Vector Get_Epsilon_Closure(const NFA *nfa, Vector states_id)
{
	Vector epsilon_closure;
	Vector_Init(&epsilon_closure, sizeof(size_t));
	Vector stack;
	Vector_Init(&stack, sizeof(size_t));

	for (size_t i = 0; i < Vector_Size(&states_id); i++) {
		size_t current_id;
		Vector_Get_Copy(&states_id, i, &current_id);

		Vector_Push(&epsilon_closure, &current_id);
		Vector_Push(&stack, &current_id);
	}

	while (!Vector_IsEmpty(&stack)) {
		size_t reachableStack_id;
		Vector_Pop(&stack, &reachableStack_id);

		State currentState;
		Vector_Get_Copy(&nfa->states, reachableStack_id, &currentState);
		for (size_t i = 0; i < Vector_Size(&currentState.epsilon_transitions); i++) {
			Transition tempTransition;
			Vector_Get_Copy(&currentState.epsilon_transitions, i, &tempTransition);

			ssize_t tempTransitionIndex =
				Vector_Find(&epsilon_closure, &tempTransition.to_id, eq_Int);
			if (tempTransitionIndex == -1) {
				Vector_Push(&epsilon_closure, &tempTransition.to_id);
				Vector_Push(&stack, &tempTransition.to_id);
			}
		}
	}

	Vector_Free(&stack);

	return epsilon_closure;
}

static Vector move(const NFA *nfa, Vector states_id, Symbol symbol)
{
	Vector result;
	Vector_Init(&result, sizeof(int));

	for (size_t i = 0; i < Vector_Size(&states_id); i++) {
		size_t state_id;
		Vector_Get_Copy(&states_id, i, &state_id);

		State currentState;
		Vector_Get_Copy(&nfa->states, state_id, &currentState);

		for (size_t j = 0; j < Vector_Size(&currentState.transitions); j++) {
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

void NFA_Load(NFA *nfa, FILE *file)
{
	char *line = NULL;
	size_t cap = 0;
	ssize_t n;

	int line_id = 0;

	while ((n = getline(&line, &cap, file)) != -1) {
		line[n - 1] = '\0';
		switch (line_id) {
		case 0:
			parseQLine(nfa, line);
			break;
		case 1:
			parseAlphabetLine(nfa, line);
			break;
		case 2:
			parseSLine(nfa, line);
			break;
		case 3:
			parseFLine(nfa, line);
			break;
		default:
			parseDeltaLine(nfa, line);
			break;
		}
		line_id++;
	}
}

static void parseQLine(NFA *nfa, const char *line)
{
	int start = 0;
	int end = 0;
	int i = 0;

	while (true) {
		if (line[i] == ',' || line[i] == '\0') {
			StateID s = {0};
			copy_StateID(&line[start], end, &s);

			State newState = State_Init(s);

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

static void parseAlphabetLine(NFA *nfa, const char *line)
{
	int i = 0;

	while (line[i] != '\0') {
		Symbol c = (Symbol)line[i];
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

static void parseSLine(NFA *nfa, const char *line)
{
	State initialStateDummy;

	size_t i = 0;

	for (; i < MAX_STATE_ID_SIZE && line[i] != '\0'; i++) {
		initialStateDummy.id[i] = line[i];
	}

	initialStateDummy.id[i] = line[i];

	ssize_t initialState_id = Vector_Find(&nfa->states, &initialStateDummy, eq_State);
	if (initialState_id == -1) {
		fprintf(stderr, "Initial State not in States\n");
		exit(2);
	}

	nfa->initialState_id = (size_t)initialState_id;
}

static void parseFLine(const NFA *nfa, const char *line)
{
	int start = 0;
	int i = 0;

	while (true) {
		if (line[i] == ',' || line[i] == '\0') {
			State dummyState;
			copy_StateID(&line[start], i, &dummyState.id);

			ssize_t finalState_id = Vector_Find(&nfa->states, &dummyState, eq_State);
			if (finalState_id == -1) {
				fprintf(stderr, "Final state %s is not in Q\n", dummyState.id);
				exit(1);
			}

			State *stateRef = Vector_Get(&nfa->states, (size_t)finalState_id);

			stateRef->isAccept = true;

			if (line[i] == '\0')
				break;

			start = i + i;
		}
		i++;
	}
}

static void parseDeltaLine(const NFA *nfa, const char *line)
{
	State fromDummy, toDummy;
	char symbolBuffer[2];
	Symbol symbol;

	sscanf(line, "%3[^,],%1[^.],%3s", fromDummy.id, symbolBuffer, toDummy.id);

	symbol = (Symbol)symbolBuffer[0];

	ssize_t from_id = Vector_Find(&nfa->states, &fromDummy, eq_State);
	if (from_id == -1) {
		fprintf(stderr, "State %s in delta was not defined\n", fromDummy.id);
		exit(2);
	}
	ssize_t to_id = Vector_Find(&nfa->states, &toDummy, eq_State);
	if (to_id == -1) {
		fprintf(stderr, "State %s in delta was not defined\n", toDummy.id);
		exit(2);
	}

	Transition newTransition = {.to_id = (size_t)to_id, .symbol = symbol};

	State *fromState = Vector_Get(&nfa->states, (size_t)from_id);

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

static void copy_StateID(const char *src, int end, StateID *out)
{
	int i = 0;
	for (; i < end && i < MAX_STATE_ID_SIZE; i++)
		(*out)[i] = src[i];
	(*out)[i] = '\0';
}

static bool eq_StateID(const StateID A, const StateID B)
{
	for (int i = 0; i < MAX_STATE_ID_SIZE; i++)
		if (A[i] != B[i])
			return false;
	return true;
}

static bool eq_State(const void *a, const void *b)
{
	const State StateA = *(const State *)a;

	const State StateB = *(const State *)b;

	return eq_StateID(StateA.id, StateB.id);
}

static bool eq_Int(const void *a, const void *b)
{
	const int intA = *(const int *)a;

	const int intB = *(const int *)b;

	return intA == intB;
}

static State State_Init(StateID id)
{
	State retState;
	for (int i = 0; i < MAX_STATE_ID_SIZE; i++)
		retState.id[i] = id[i];

	retState.isAccept = false;

	Vector_Init(&retState.epsilon_transitions, sizeof(Transition));
	Vector_Init(&retState.transitions, sizeof(Transition));

	return retState;
}
