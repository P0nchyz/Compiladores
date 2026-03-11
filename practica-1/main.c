#include "NFA.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *afnFile;
	if (!(afnFile = fopen(argv[1], "r"))) {
		fprintf(stderr, "File %s not found\n", argv[1]);
		exit(2);
	}

	NFA nfa;
	Init_NFA(&nfa);

	Fill_NFA_From_File(&nfa, afnFile);

	while (true) {
		printf("Input: ");
		char *line;
		size_t cap = 0;
		ssize_t n;

		n = getline(&line, &cap, stdin);
		
		line[n - 1] = '\0';

		if (accepts(&nfa, line))
			printf("Accepts\n");
		else
			printf("Doesn't accept\n");

		free(line);
	}



}
