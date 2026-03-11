#ifndef NFA_H
#define NFA_H

#include <stdbool.h>
#include <stdio.h>
#include "vector.h"

typedef char Symbol;
#define MAX_ALPHABET_SIZE 256

#define MAX_STATE_ID_SIZE 3
typedef char StateID[MAX_STATE_ID_SIZE + 1];

typedef struct {
	StateID id;
	Vector transitions;
	Vector epsilon_transitions;
	bool isAccept;
} State;

typedef struct {
	Symbol symbol;
	int to_id;
} Transition;

typedef struct {
	Vector states; // Q

	bool isInAlphabet[MAX_ALPHABET_SIZE]; // Σ

	int initialState_id; // S
} NFA;
void Init_NFA(NFA *nfa);
void Fill_NFA_From_File(NFA *nfa, FILE* file);

bool accepts(NFA *nfa, char *string);

#endif
