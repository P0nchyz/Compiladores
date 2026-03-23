#ifndef NFA_H
#define NFA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "vector/vector.h"

typedef unsigned char Symbol;
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
	size_t to_id;
} Transition;

typedef struct {
	Vector states; // Q

	bool isInAlphabet[MAX_ALPHABET_SIZE]; // Σ

	size_t initialState_id; // S
} NFA;

void NFA_Init(NFA *nfa);
void NFA_Load(NFA *nfa, FILE* file);

bool NFA_Accepts(const NFA *nfa, char *string);

#endif
