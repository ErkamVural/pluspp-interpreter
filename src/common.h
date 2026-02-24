#ifndef COMMON_H
#define COMMON_H

#define MAX_TOKENS 2000     // Increased limit for larger scripts
#define MAX_DIGITS 100      // Max digits for Big Integer support
#define MAX_VARIABLES 100   // Max number of declared variables

// Token structure used by both Lexer and Interpreter
typedef struct {
    char *token;    
    char *value;   
} Lexeme;

#endif