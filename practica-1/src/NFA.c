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
static int parseNextToken(const char **cursor, StateID *ret);
static int parseNextAlphabet(const char **cursor, Symbol *ret);

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
	// Puts initial state in array to match Get_Epsilon_Closure
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
	const char *cursor = line;
	StateID s;
	while (true) {
		int parseStatus = parseNextToken(&cursor, &s);
		State newState = State_Init(s);

		if (Vector_Find(&nfa->states, &newState, eq_State) != -1) {
			fprintf(stderr, "Repeated states in Q\n");
			exit(1);
		}

		Vector_Push(&nfa->states, &newState);
		if (parseStatus != 0) return;
	}
}

static void parseAlphabetLine(NFA *nfa, const char *line)
{
	const char *cursor = line;
	Symbol s;
	while (true) {
		int parseStatus = parseNextAlphabet(&cursor, &s);
		if (nfa->isInAlphabet[s]) {
			fprintf(stderr, "Duplicate symbols in alphabet\n");
			exit(2);
		}
		nfa->isInAlphabet[s] = true;
		if (parseStatus != 0) return;
	}
}

static void parseSLine(NFA *nfa, const char *line)
{
	const char *cursor = line;
	State initialStateDummy;
	parseNextToken(&cursor, &initialStateDummy.id);

	ssize_t initialState_id = Vector_Find(&nfa->states, &initialStateDummy, eq_State);
	if (initialState_id == -1) {
		fprintf(stderr, "Initial State not in States\n");
		exit(2);
	}

	nfa->initialState_id = (size_t)initialState_id;
}

static void parseFLine(const NFA *nfa, const char *line)
{
	const char *cursor = line;
	State dummyState;
	while (true) {
		int parseStatus = parseNextToken(&cursor, &dummyState.id);
		ssize_t finalState_id = Vector_Find(&nfa->states, &dummyState, eq_State);
		if (finalState_id == -1) {
			fprintf(stderr, "Final state %s is not in Q\n", dummyState.id);
			exit(2);
		}

		State *stateRef = Vector_Get(&nfa->states, (size_t)finalState_id);

		stateRef->isAccept = true;

		if (parseStatus != 0) return;
	}
}

static void parseDeltaLine(const NFA *nfa, const char *line)
{
	const char *cursor = line;
	State fromDummy, toDummy;
	Symbol symbol;

	parseNextToken(&cursor, &fromDummy.id);
	parseNextAlphabet(&cursor, &symbol);
	parseNextToken(&cursor, &toDummy.id);

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

/**
 * @brief Parse the next token from a string into a StateID.
 *
 * A token is a string of 1-MAX_STATE_ID_SIZE characters after unescaping.
 * Escapable characters are ',' and '\'.
 *
 * Tokens are separated by commas. A comma terminates the current token (unless escaped).
 * '\0' and '\n' terminate the input.
 *
 * Advances the input cursor past the parsed token and its delimiter.
 *
 * @param cursor Pointer to the current position in the input string. Updated to point past the
 * parsed token.
 * @param ret Output parameter where the parsed StateID is stored.
 *
 * @return Parsing status.
 * @retval 0 Token successfully parsed, more tokens may follow.
 * @retval 1 Token successfully parsed, and it was the last token.
 */
static int parseNextToken(const char **cursor, StateID *ret)
{
	size_t usable_count = 0;
	bool escaped_flag = false;
	for (;; (*cursor)++) {
		char readChar = **cursor;

		if (readChar == '\0' || readChar == '\n') {
			if (usable_count == 0) {
				fprintf(stderr, "parseNextToken: Couldn't parse token\n");
				exit(1);
			} else if (escaped_flag) {
				fprintf(stderr, "parseNextToken: Dangling escape\n");
				exit(1);
			} else {
				(*ret)[usable_count] = '\0';
				(*cursor)++;
				return 1;
			}
		}

		if (escaped_flag) {
			if (readChar == ',' || readChar == '\\') {
				(*ret)[usable_count] = readChar;
				usable_count++;
				escaped_flag = false;

				if (usable_count > MAX_STATE_ID_SIZE) {
					fprintf(stderr, "parseNextToken: Token longer than %d\n",
						MAX_STATE_ID_SIZE);
					exit(1);
				}
			} else {
				fprintf(stderr,
					"parseNextToken: Escaped non-escapable character\n");
				exit(1);
			}
		} else if (readChar == ',') {
			if (usable_count == 0) {
				fprintf(stderr, "parseNextToken: Couldn't parse token\n");
				exit(1);
			} else {
				(*ret)[usable_count] = '\0';
				(*cursor)++;
				return 0;
			}
		} else if (readChar == '\\') {
			escaped_flag = true;
		} else {
			(*ret)[usable_count] = readChar;
			usable_count++;
			if (usable_count > MAX_STATE_ID_SIZE) {
				fprintf(stderr, "parseNextToken: Token longer than %d\n",
					MAX_STATE_ID_SIZE);
				exit(1);
			}
		}
	}
}

static int parseNextAlphabet(const char **cursor, Symbol *ret)
{
	bool escaped = false;
	bool haveChar = false;
	char value = '\0';

	for (;; (*cursor)++) {
		char c = **cursor;

		if (c == '\0' || c == '\n') {
			if (escaped) {
				fprintf(stderr, "parseNextAlphabet: Dangling escape\n");
				exit(1);
			}

			if (!haveChar) {
				*ret = '\0';
			} else {
				*ret = (Symbol)value;
			}
			return 1;
		}

		if (escaped) {
			if (haveChar) {
				fprintf(stderr, "parseNextAlphabet: Too many characters\n");
				exit(1);
			}

			if (c == ',' || c == '\\') {
				value = c;
			} else if (c == 'n') {
				value = '\n';
			} else {
				fprintf(stderr,
					"parseNextToken: Escaped non-escapable character\n");
				exit(1);
			}

			haveChar = true;
			escaped = false;
		} else if (c == '\\') {
			escaped = true;
		} else if (c == ',') {
			if (!haveChar) {
				*ret = '\0';
			} else {
				*ret = (Symbol)value;
			}
			(*cursor)++;
			return 0;
		} else {
			if (haveChar) {
				fprintf(stderr, "parseNextToken: Too many characters\n");
				exit(1);
			}

			value = c;
			haveChar = true;
		}
	}
}
