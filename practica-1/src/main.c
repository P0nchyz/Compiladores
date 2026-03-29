#include "NFA.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s <input>\n\n", argv[0]);
		printf("Arguments:\n");
		printf("\tinput\tPath to the NFA file.\n\n");
		exit(1);
	}

	FILE *afnFile;
	if (!(afnFile = fopen(argv[1], "r"))) {
		fprintf(stderr, "File %s not found\n", argv[1]);
		exit(2);
	}

	NFA nfa;
	NFA_Init(&nfa);

	NFA_Load(&nfa, afnFile);

	while (true) {
		printf("Input: ");
		char *line;
		size_t cap = 0;
		ssize_t n;

		n = getline(&line, &cap, stdin);
		
		line[n - 1] = '\0';

		if (NFA_Accepts(&nfa, line))
			printf("Accepts\n");
		else
			printf("Doesn't accept\n");
		free(line);
	}
}
