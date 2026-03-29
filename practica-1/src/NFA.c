#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "NFA.h"
#include "vector/vector.h"

typedef struct {
	Transition transition;
	ssize_t parent_index;
} PathNode;

static Vector Get_Epsilon_Closure(const NFA *nfa, Vector input_indices, Vector *path_history);
static Vector move(const NFA *nfa, Vector current_indices, Symbol symbol, Vector *path_history);
static bool Print_Path(const NFA *nfa, Vector *path_history, ssize_t current_idx);

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

bool NFA_Accepts(const NFA *nfa, const char *string)
{
	Vector path_history;
	Vector_Init(&path_history, sizeof(PathNode));

	PathNode root = {.transition = {
		.from_id = 0, .symbol = 0, .to_id = nfa->initialState_id},
		.parent_index = -1
	};
	ssize_t inital_idx = (ssize_t)Vector_Push(&path_history, &root);

	Vector initial_indices;
	Vector_Init(&initial_indices, sizeof(ssize_t));
	Vector_Push(&initial_indices, &inital_idx);

	Vector current_indices = Get_Epsilon_Closure(nfa, initial_indices, &path_history);
	Vector_Free(&initial_indices);

	for (int i = 0; string[i]; i++) {
		Vector moved = move(nfa, current_indices, (Symbol)string[i], &path_history);
		Vector_Free(&current_indices);

		if (Vector_IsEmpty(&moved)) {
			Vector_Free(&moved);
			return false;
		}

		current_indices = Get_Epsilon_Closure(nfa, moved, &path_history);
		Vector_Free(&moved);
	}

	bool accepted = false;
	for (size_t i = 0; i < Vector_Size(&current_indices); i++) {
		ssize_t history_idx;
		Vector_Get_Copy(&current_indices, i, &history_idx);

		PathNode *end_node = Vector_Get(&path_history, (size_t)history_idx);

		State s;
		Vector_Get_Copy(&nfa->states, end_node->transition.to_id, &s);

		if (s.isAccept) {
			accepted = true;
			Print_Path(nfa, &path_history, history_idx);
			printf("\n");
		}
	}

	Vector_Free(&current_indices);
	Vector_Free(&path_history);
	return accepted;
}

static bool Print_Path(const NFA *nfa, Vector *path_history, ssize_t current_idx)
{
	if (current_idx == -1)
		return true;

	PathNode *node = Vector_Get(path_history, (size_t)current_idx);

	if (node->parent_index == -1)
		return true;

	bool parent_isDummy = Print_Path(nfa, path_history, node->parent_index);

	State fromState;
	Vector_Get_Copy(&nfa->states, node->transition.from_id, &fromState);
	State toState;
	Vector_Get_Copy(&nfa->states, node->transition.to_id, &toState);

	Symbol symbol = node->transition.symbol;

	if (!parent_isDummy) {
		printf("--'%c'->(%s)", symbol, toState.id);
	} else {
		printf("(%s)--'%c'->(%s)", fromState.id, symbol, toState.id);
	}

	return false;
}

static Vector Get_Epsilon_Closure(const NFA *nfa, const Vector input_indices, Vector *path_history)
{
	Vector closure_indices;
	Vector_Init(&closure_indices, sizeof(ssize_t));

	Vector stack;
	Vector_Init(&stack, sizeof(ssize_t));

	bool *visited = calloc(Vector_Size(&nfa->states), sizeof(*visited));

	// input_indices already part of epsilon_closure
	for (size_t i = 0; i < Vector_Size(&input_indices); i++) {
		ssize_t idx;
		Vector_Get_Copy(&input_indices, i, &idx);

		PathNode *n = Vector_Get(path_history, (size_t)idx);
		visited[n->transition.to_id] = true;

		Vector_Push(&closure_indices, &idx);
		Vector_Push(&stack, &idx);
	}

	while (!Vector_IsEmpty(&stack)) {
		ssize_t curr_idx;
		Vector_Pop(&stack, &curr_idx);

		PathNode *curr_node = Vector_Get(path_history, (size_t)curr_idx);

		State s;
		Vector_Get_Copy(&nfa->states, curr_node->transition.to_id, &s);

		// Find all epsilon transitions and add to history if not already in closure
		for (size_t i = 0; i < Vector_Size(&s.epsilon_transitions); i++) {
			Transition t;
			Vector_Get_Copy(&s.epsilon_transitions, i, &t);

			if (!visited[t.to_id]) {
				visited[t.to_id] = true;

				PathNode eps_node = {.transition = t, .parent_index = curr_idx};
				size_t new_idx = Vector_Push(path_history, &eps_node);
				Vector_Push(&closure_indices, &new_idx);
				Vector_Push(&stack, &new_idx);
			}
		}
	}
	Vector_Free(&stack);
	return closure_indices;
}

static Vector move(const NFA *nfa, const Vector current_indices, Symbol symbol, Vector *pathHistory)
{
	Vector result_indices;
	Vector_Init(&result_indices, sizeof(ssize_t));

	for (size_t i = 0; i < Vector_Size(&current_indices); i++) {
		ssize_t parent_idx;
		Vector_Get_Copy(&current_indices, i, &parent_idx);

		PathNode *parent_node = Vector_Get(pathHistory, (size_t)parent_idx);

		State currentState;
		Vector_Get_Copy(&nfa->states, parent_node->transition.to_id, &currentState);

		for (size_t j = 0; j < Vector_Size(&currentState.transitions); j++) {
			Transition t;
			Vector_Get_Copy(&currentState.transitions, j, &t);

			if (t.symbol == symbol) {
				PathNode new_step = {.transition = t, .parent_index = parent_idx};
				size_t new_idx = Vector_Push(pathHistory, &new_step);
				Vector_Push(&result_indices, &new_idx);
			}
		}
	}

	return result_indices;
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
		if (parseStatus != 0)
			return;
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
		if (parseStatus != 0)
			return;
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

		if (parseStatus != 0)
			return;
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

	Transition newTransition = {
		.from_id = (size_t)from_id, .symbol = symbol, .to_id = (size_t)to_id};

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
