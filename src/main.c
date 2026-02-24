#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "lexer.h"

// Data Structures 
typedef struct {
    char name[21];
    char value[MAX_DIGITS + 1];
    int declared;
} Variable;

typedef struct {
    char digits[MAX_DIGITS + 1];
    int sign; // 1 for positive, -1 for negative
    int length;
} Decimal;

// Global State
Lexeme lexeme_table[MAX_TOKENS];
int lexeme_index = 0;
int current_token_index = 0;
Variable variables[MAX_VARIABLES];
int variable_count = 0;

// Function Prototypes
void execute_repeat();
void execute_statement();

// Variable Logic 
int find_variable(char *name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void declare_variable(char *name) {
    if (find_variable(name) != -1) {
        printf("Runtime Error: Variable '%s' already declared!\n", name);
        exit(1);
    }
    strcpy(variables[variable_count].name, name);
    strcpy(variables[variable_count].value, "0");
    variables[variable_count].declared = 1;
    variable_count++;
}

void assign_variable(char *name, char *value) {
    int index = find_variable(name);
    if (index == -1) {
        printf("Runtime Error: Variable '%s' not declared!\n", name);
        exit(1);
    }
    strcpy(variables[index].value, value);
}

char *get_variable_value(char *name) {
    int index = find_variable(name);
    if (index == -1) {
        printf("Runtime Error: Variable '%s' not declared!\n", name);
        exit(1);
    }
    return variables[index].value;
}

// Big Int Logic
Decimal create_decimal(char *str) {
    Decimal num;
    num.sign = 1;
    num.length = 0;
    int start = 0;

    if (str[0] == '-') {
        num.sign = -1;
        start = 1;
    }

    int len = strlen(str);
    for (int i = start; i < len; i++) {
        num.digits[num.length++] = str[i];
    }
    num.digits[num.length] = '\0';
    
    return num;
}

int compare_magnitude(Decimal a, Decimal b) {
    if (a.length > b.length) return 1;
    if (a.length < b.length) return -1;

    for (int i = 0; i < a.length; i++) {
        if (a.digits[i] > b.digits[i]) return 1;
        if (a.digits[i] < b.digits[i]) return -1;
    }
    return 0;
}

Decimal subtract_positive(Decimal a, Decimal b) {
    Decimal result;
    result.sign = 1;
    result.length = 0;
    
    int borrow = 0;
    int k = 0;
    int i = a.length - 1;
    int j = b.length - 1;

    while (i >= 0) {
        int digit_a = a.digits[i] - '0';
        int digit_b = (j >= 0) ? (b.digits[j] - '0') : 0;
        int diff = digit_a - digit_b - borrow;

        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.digits[k++] = diff + '0';
        i--;
        j--;
    }

    result.length = k;

    // Reverse logic
    for (int x = 0; x < result.length / 2; x++) {
        char temp = result.digits[x];
        result.digits[x] = result.digits[result.length - 1 - x];
        result.digits[result.length - 1 - x] = temp;
    }

    // Remove leading zeros
    int start = 0;
    while (start < result.length - 1 && result.digits[start] == '0') {
        start++;
    }

    if (start > 0) {
        for (int m = 0; m < result.length - start; m++) {
            result.digits[m] = result.digits[m + start];
        }
        result.length -= start;
    }

    result.digits[result.length] = '\0';
    
    if (result.length == 0 || (result.length == 1 && result.digits[0] == '0')) {
        result.digits[0] = '0';
        result.length = 1;
        result.sign = 1;
    }
    return result;
}

Decimal add_positive(Decimal a, Decimal b) {
    Decimal result;
    result.sign = 1;
    result.length = 0;
    
    int carry = 0;
    int k = 0;
    int i = a.length - 1;
    int j = b.length - 1;

    while (i >= 0 || j >= 0 || carry > 0) {
        int digit_a = (i >= 0) ? (a.digits[i] - '0') : 0;
        int digit_b = (j >= 0) ? (b.digits[j] - '0') : 0;
        int sum = digit_a + digit_b + carry;
        
        result.digits[k++] = (sum % 10) + '0';
        carry = sum / 10;
        
        i--;
        j--;
    }

    result.length = k;

    for (int x = 0; x < result.length / 2; x++) {
        char temp = result.digits[x];
        result.digits[x] = result.digits[result.length - 1 - x];
        result.digits[result.length - 1 - x] = temp;
    }
    result.digits[result.length] = '\0';
    
    return result;
}

Decimal add_integers(Decimal a, Decimal b) {
    Decimal result;

    if (a.sign == 1 && b.sign == 1) {
        result = add_positive(a, b);
        result.sign = 1;
    } else if (a.sign == -1 && b.sign == -1) {
        Decimal ta = a, tb = b;
        ta.sign = 1;
        tb.sign = 1;
        result = add_positive(ta, tb);
        result.sign = -1;
    } else if (a.sign == 1 && b.sign == -1) {
        Decimal tb = b;
        tb.sign = 1;
        if (compare_magnitude(a, tb) >= 0) {
            result = subtract_positive(a, tb);
            result.sign = 1;
        } else {
            result = subtract_positive(tb, a);
            result.sign = -1;
        }
    } else {
        Decimal ta = a;
        ta.sign = 1;
        if (compare_magnitude(b, ta) >= 0) {
            result = subtract_positive(b, ta);
            result.sign = 1;
        } else {
            result = subtract_positive(ta, b);
            result.sign = -1;
        }
    }

    if (result.length == 1 && result.digits[0] == '0') {
        result.sign = 1;
    }
    return result;
}

Decimal subtract_integers(Decimal a, Decimal b) {
    Decimal result;

    if (a.sign == 1 && b.sign == 1) {
        if (compare_magnitude(a, b) >= 0) {
            result = subtract_positive(a, b);
            result.sign = 1;
        } else {
            result = subtract_positive(b, a);
            result.sign = -1;
        }
    } else if (a.sign == -1 && b.sign == -1) {
        Decimal ta = a, tb = b;
        ta.sign = 1;
        tb.sign = 1;
        if (compare_magnitude(tb, ta) >= 0) {
            result = subtract_positive(tb, ta);
            result.sign = 1;
        } else {
            result = subtract_positive(ta, tb);
            result.sign = -1;
        }
    } else if (a.sign == 1 && b.sign == -1) {
        Decimal tb = b;
        tb.sign = 1;
        result = add_positive(a, tb);
        result.sign = 1;
    } else {
        Decimal ta = a;
        ta.sign = 1;
        result = add_positive(ta, b);
        result.sign = -1;
    }

    if (result.length == 1 && result.digits[0] == '0') {
        result.sign = 1;
    }
    return result;
}

// Interpreter Logic
void execute_assignment() {
    if (current_token_index >= lexeme_index) return;

    char *var_name = lexeme_table[current_token_index].value;
    current_token_index++;

    // Check for ':='
    if (current_token_index >= lexeme_index || 
        strcmp(lexeme_table[current_token_index].value, ":=") != 0) {
        printf("Syntax Error: Expected ':=' after variable name '%s'!\n", var_name);
        exit(1);
    }
    current_token_index++;

    if (current_token_index >= lexeme_index) {
        printf("Syntax Error: Expected value after ':='!\n");
        exit(1);
    }

    char *value;
    if (strcmp(lexeme_table[current_token_index].token, "IntConstant") == 0) {
        value = lexeme_table[current_token_index].value;
    } else if (strcmp(lexeme_table[current_token_index].token, "Identifier") == 0) {
        value = get_variable_value(lexeme_table[current_token_index].value);
    } else {
        printf("Syntax Error: Expected integer or variable for assignment!\n");
        exit(1);
    }

    assign_variable(var_name, value);
    current_token_index++;
}

void execute_increment() {
    if (current_token_index >= lexeme_index) return;

    char *var_name = lexeme_table[current_token_index].value;
    current_token_index++;

    if (current_token_index >= lexeme_index || 
        strcmp(lexeme_table[current_token_index].value, "+=") != 0) {
        printf("Syntax Error: Expected '+=' after variable name!\n");
        exit(1);
    }
    current_token_index++;

    if (current_token_index >= lexeme_index) {
        printf("Syntax Error: Expected value after '+='!\n");
        exit(1);
    }

    char *increment_value;
    if (strcmp(lexeme_table[current_token_index].token, "IntConstant") == 0) {
        increment_value = lexeme_table[current_token_index].value;
    } else if (strcmp(lexeme_table[current_token_index].token, "Identifier") == 0) {
        increment_value = get_variable_value(lexeme_table[current_token_index].value);
    } else {
        printf("Syntax Error: Expected integer or variable for increment!\n");
        exit(1);
    }

    char *current_value = get_variable_value(var_name);
    Decimal current_num = create_decimal(current_value);
    Decimal increment_num = create_decimal(increment_value);
    Decimal result = add_integers(current_num, increment_num);

    char result_str[MAX_DIGITS + 2];
    if (result.sign == -1) {
        sprintf(result_str, "-%s", result.digits);
    } else {
        strcpy(result_str, result.digits);
    }

    assign_variable(var_name, result_str);
    current_token_index++;
}

void execute_decrement() {
    if (current_token_index >= lexeme_index) return;

    char *var_name = lexeme_table[current_token_index].value;
    current_token_index++;

    if (current_token_index >= lexeme_index || 
        strcmp(lexeme_table[current_token_index].value, "-=") != 0) {
        printf("Syntax Error: Expected '-=' after variable name!\n");
        exit(1);
    }
    current_token_index++;

    if (current_token_index >= lexeme_index) {
        printf("Syntax Error: Expected value after '-='!\n");
        exit(1);
    }

    char *decrement_value;
    if (strcmp(lexeme_table[current_token_index].token, "IntConstant") == 0) {
        decrement_value = lexeme_table[current_token_index].value;
    } else if (strcmp(lexeme_table[current_token_index].token, "Identifier") == 0) {
        decrement_value = get_variable_value(lexeme_table[current_token_index].value);
    } else {
        printf("Syntax Error: Expected integer or variable for decrement!\n");
        exit(1);
    }

    char *current_value = get_variable_value(var_name);
    Decimal current_num = create_decimal(current_value);
    Decimal decrement_num = create_decimal(decrement_value);
    Decimal result = subtract_integers(current_num, decrement_num);

    char result_str[MAX_DIGITS + 2];
    if (result.sign == -1) {
        sprintf(result_str, "-%s", result.digits);
    } else {
        strcpy(result_str, result.digits);
    }

    assign_variable(var_name, result_str);
    current_token_index++;
}

void execute_write() {
    current_token_index++;
    while (current_token_index < lexeme_index && 
           strcmp(lexeme_table[current_token_index].token, "EndOfLine") != 0) {
        
        char *type = lexeme_table[current_token_index].token;
        char *val = lexeme_table[current_token_index].value;

        if (strcmp(type, "StringConstant") == 0) {
            // Remove quotes
            for (int i = 1; i < strlen(val) - 1; i++) {
                printf("%c", val[i]);
            }
        } else if (strcmp(type, "Identifier") == 0) {
            printf("%s", get_variable_value(val));
        } else if (strcmp(type, "IntConstant") == 0) {
            printf("%s", val);
        } else if (strcmp(type, "Keyword") == 0) {
            if (strcmp(val, "newline") == 0) {
                printf("\n");
            } else if (strcmp(val, "and") == 0) {
                // Do nothing, just a separator
            } else {
                printf("Syntax Error: Invalid keyword '%s' in write statement!\n", val);
                exit(1);
            }
        } else {
            printf("Syntax Error: Unexpected token type '%s' in write statement!\n", type);
            exit(1);
        }
        current_token_index++;
    }
}

void execute_repeat() {
    current_token_index++; // Skip 'repeat'

    if (current_token_index >= lexeme_index) {
        printf("Syntax Error: Expected token after repeat!\n");
        exit(1);
    }

    int repeat_count;
    char *var_name = NULL;
    int is_variable = 0;

    // Get count (Constant or Variable)
    if (strcmp(lexeme_table[current_token_index].token, "IntConstant") == 0) {
        repeat_count = atoi(lexeme_table[current_token_index].value);
    } else if (strcmp(lexeme_table[current_token_index].token, "Identifier") == 0) {
        var_name = lexeme_table[current_token_index].value;
        repeat_count = atoi(get_variable_value(var_name));
        is_variable = 1;
    } else {
        printf("Syntax Error: Expected integer or variable after 'repeat'!\n");
        exit(1);
    }

    current_token_index++;

    // Check for 'times'
    if (current_token_index >= lexeme_index || 
        strcmp(lexeme_table[current_token_index].value, "times") != 0) {
        printf("Syntax Error: Expected 'times' keyword!\n");
        exit(1);
    }
    current_token_index++;

    int loop_start_index = current_token_index;
    int is_block = 0;

    // Check for block '{'
    if (current_token_index < lexeme_index && 
        strcmp(lexeme_table[current_token_index].token, "OpenBlock") == 0) {
        is_block = 1;
        current_token_index++;
    }

    // Execution Loop
    for (int i = repeat_count; i >= 1; i--) {
        current_token_index = loop_start_index;

        if (is_variable) {
            char temp[12];
            sprintf(temp, "%d", i);
            assign_variable(var_name, temp);
        }

        if (is_block) {
            current_token_index++; // Enter block
            while (current_token_index < lexeme_index && 
                   strcmp(lexeme_table[current_token_index].token, "CloseBlock") != 0) {
                execute_statement();
            }
        } else {
            execute_statement();
        }
    }

    // Reset loop variable if used
    if (is_variable) {
        assign_variable(var_name, "0");
    }

    // Move index past the loop body
    if (is_block && current_token_index < lexeme_index && 
        strcmp(lexeme_table[current_token_index].token, "CloseBlock") == 0) {
        current_token_index++;
    }
}

void execute_statement() {
    if (current_token_index >= lexeme_index) return;

    char *type = lexeme_table[current_token_index].token;
    char *val = lexeme_table[current_token_index].value;

    if (strcmp(type, "Keyword") == 0) {
        if (strcmp(val, "number") == 0) {
            current_token_index++;
            if (current_token_index >= lexeme_index || 
                strcmp(lexeme_table[current_token_index].token, "Identifier") != 0) {
                printf("Syntax Error: Expected variable name after 'number'!\n");
                exit(1);
            }
            declare_variable(lexeme_table[current_token_index].value);
            current_token_index++;
        } else if (strcmp(val, "write") == 0) {
            execute_write();
        } else if (strcmp(val, "repeat") == 0) {
            execute_repeat();
        } else {
            printf("Syntax Error: Unexpected keyword '%s' at start of statement!\n", val);
            exit(1);
        }
    } else if (strcmp(type, "Identifier") == 0) {
        // Look ahead for operator
        if (current_token_index + 1 < lexeme_index && 
            strcmp(lexeme_table[current_token_index + 1].token, "Operator") == 0) {
            
            char *op = lexeme_table[current_token_index + 1].value;
            if (strcmp(op, ":=") == 0) execute_assignment();
            else if (strcmp(op, "+=") == 0) execute_increment();
            else if (strcmp(op, "-=") == 0) execute_decrement();
            else {
                printf("Syntax Error: Unknown operator '%s'!\n", op);
                exit(1);
            }
        } else {
            printf("Syntax Error: Identifier '%s' without assignment/operation!\n", val);
            exit(1);
        }
    }

    // Skip EndOfLine (;)
    if (current_token_index < lexeme_index && 
        strcmp(lexeme_table[current_token_index].token, "EndOfLine") == 0) {
        current_token_index++;
    }
}

//  Main Program 
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ppp <filename>\n");
        return 1;
    }

    char filename[100];
    sprintf(filename, "%s.ppp", argv[1]);

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file '%s'\n", filename);
        return 1;
    }

    // 1. Lexer Phase
    init_lexer(file);
    lexeme_index = tokenize(lexeme_table);
    fclose(file);

    // 2. Interpreter Phase
    current_token_index = 0;
    while (current_token_index < lexeme_index) {
        execute_statement();
    }

    return 0;
}