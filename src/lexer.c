#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

#define KEYWORD_NUMBER 6
#define OPERATOR_NUMBER 3

const char *keywords[KEYWORD_NUMBER] = {"number", "write", "repeat", "times", "and", "newline"};
const char *operators[OPERATOR_NUMBER] = {":=", "+=", "-="};

// Global variables (Hidden within this file)
static FILE *input;
static int current_char;
static int current_char_index = 0;
static int current_line = 1;
static Lexeme *target_table;
static int lexeme_count = 0;

// Helper: Safe string duplication
char *safe_strdup(const char *s) {
    if (!s) return NULL;
    char *d = malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}

// Advances the file cursor
void get_char() {
    current_char = fgetc(input);
    if (current_char == '\n') {
        current_line++;
        current_char_index = 0;
    } else {
        current_char_index++;
    }
}

// Adds to memory instead of file
void add_token(char *token, char *value) {
    if (lexeme_count >= MAX_TOKENS) {
        printf("Error: Token limit exceeded!\n");
        exit(1);
    }
    target_table[lexeme_count].token = safe_strdup(token); // Use .token instead of .type to match your struct
    target_table[lexeme_count].value = value ? safe_strdup(value) : NULL;
    lexeme_count++;
}


void skip_comment() {
    get_char();
    while (current_char != EOF) {
        if (current_char == '*') {
            get_char();
            return;
        }
        get_char();
    }
    printf("The comment could not be closed!\n");
    exit(1);
}

int is_keyword(char *str) {
    for (int i = 0; i < KEYWORD_NUMBER; i++) {
        if (strcmp(str, keywords[i]) == 0) return 1;
    }
    return 0;
}

int is_operator() {
    int first_char = current_char;
    int second_char = fgetc(input);
    current_char_index++;

    if (second_char == EOF) return 0;

    char op[3];
    op[0] = first_char;
    op[1] = second_char;
    op[2] = '\0';

    for (int i = 0; i < OPERATOR_NUMBER; i++) {
        if (strcmp(op, operators[i]) == 0) {
            add_token("Operator", op); // Changed write_token to add_token
            return 1;
        }
    }
    ungetc(second_char, input); 
    current_char_index--;
    return 0;
}

void get_string() {
    char string[21];
    int i = 0;
    string[i++] = current_char;
    get_char();

    while (isalnum(current_char) || current_char == '_') {
        if (i < 20) {
            string[i++] = current_char;
        } else {
            printf("String cannot be greater than 20 characters at line %d!\n", current_line);
            exit(1);
        }
        get_char();
    }

    ungetc(current_char, input);
    current_char_index--;
    string[i] = '\0';

    if (is_keyword(string)) {
        add_token("Keyword", string);
    } else {
        add_token("Identifier", string);
    }
}

void get_number() {
    char number[101]; // Increased size slightly for safety
    int i = 0;

    if (current_char == '-') {
        number[i++] = current_char;
        get_char();

        if (!isdigit(current_char)) {
            printf("Invalid number format: expected digit after '-' at line %d, char %d\n",
                    current_line, current_char_index);
            exit(1);
        }
    }

    while (isdigit(current_char)) {
        if (i >= 100) {
            printf("Number too long (max 100 digits) at line %d\n", current_line);
            exit(1);
        }
        number[i++] = current_char;
        get_char();
    }

    if (!(isspace(current_char) || current_char == ';' || current_char == '}' || current_char == EOF)) {
        printf("Invalid number format: number cannot contain '%c' at line %d, char %d\n",
                current_char, current_line, current_char_index);
        exit(1);
    }

    ungetc(current_char, input); 
    current_char_index--;
    number[i] = '\0';
    add_token("IntConstant", number);
}

void get_string_constant() {
    int current_line_copy = current_line;
    int current_index_copy = current_char_index;
    char string[101];
    int i = 0;

    string[i++] = current_char;
    get_char();

    while (current_char != EOF && current_char != '"') {
        if (i >= 100) {
            printf("String too long at line %d\n", current_line_copy);
            exit(1);
        }
        string[i++] = current_char;
        get_char();
    }

    if (current_char != '"') {
        printf("The String starting at line %d, char %d was not closed!\n",
                current_line_copy, current_index_copy);
        exit(1);
    }

    string[i++] = current_char;
    string[i] = '\0';
    add_token("StringConstant", string);
}


void init_lexer(FILE *input_file) {
    input = input_file;
    current_line = 1;
    current_char_index = 0;
    lexeme_count = 0;
    get_char(); // Start reading
}

int tokenize(Lexeme *table) {
    target_table = table;

    while (current_char != EOF) {
        if (isspace(current_char)) {
             // Just skip, loop handles next get_char
        } else if (current_char == '*') {
            skip_comment();
        } else if (isalpha(current_char)) {
            get_string();
        } else if (is_operator()) {
            // handled inside
        } else if (isdigit(current_char) || current_char == '-') {
            get_number();
        } else if (current_char == '"') {
            get_string_constant();
        } else if (current_char == ';') {
            add_token("EndOfLine", NULL);
        } else if (current_char == '{') {
            add_token("OpenBlock", NULL);
        } else if (current_char == '}') {
            add_token("CloseBlock", NULL);
        } else {
            printf("Invalid character '%c' detected at line %d, char %d!\n",
                    current_char, current_line, current_char_index);
            exit(1);
        }
        get_char(); // YOUR LOGIC: Always get next char at end of loop
    }
    return lexeme_count;
}