#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "common.h"

// Initializes the lexer with the input file pointer
void init_lexer(FILE *input_file);

// Scans the file and populates the token table. Returns the total number of tokens found.
int tokenize(Lexeme *table);

#endif