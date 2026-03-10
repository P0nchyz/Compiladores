#ifndef NFA_H
#define NFA_H

#include <stdbool.h>
#include "vector.h"

typedef char Symbol;

typedef struct {
	char id[4];
} State;

typedef struct {
	Symbol symbol;
	State from;
	State to;
} Transition;

typedef struct {
	Vector states; // Q

	bool isInAlphabet[sizeof(Symbol)]; // Σ

	State initialState; // S
	Vector finalStates; // F

	Vector transitionTable[sizeof(Symbol)]; // δ
} NFA;

#endif
