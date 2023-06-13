#include "../Parser/syntaxer.h"
#include "../Parser/parser.h"
#include "../Lexer/lexer.h"
#include "../Errors/error.h"

#define next yel_tokens->value[yel_tokens->pointer+1]
#define prev yel_tokens->value[yel_tokens->pointer-1]
#define cur yel_tokens->value[yel_tokens->pointer]

typedef struct __YelToken {
    YelTokenType type;
    char* value;
} YelToken;

YelEntities yel_define_next_entity(YelTokens* yel_tokens) {
    register size_t i = yel_tokens->pointer;
    static YelTokenType last_type = en_end;

    while (i < yel_tokens->length) {
        if (__builtin_strcmp(yel_tokens->value[i], ";") == 0) {
            YelTokenType tmp = last_type;
            last_type = en_end;
            return tmp;
        }

        // tok_name ...
        if (yel_tokens->type[i] == tok_name) {
            // tok_name bin_op
            if (yel_tokens->type[i + 1] == tok_binary_op) {
                // tok_name = ...
                if (__builtin_strcmp(yel_tokens->value[i + 1], "=") == 0) {
                    return en_stmt;
                }

                // tok_name bin_op ...
                last_type = en_expr;
                size_t tmp_i = i;

                ++yel_tokens->pointer;
                if (yel_define_next_entity(yel_tokens) == en_expr) {
                    yel_tokens->pointer = i;
                    return en_expr;
                }

                printf("Syntax Error: <File '%s'>\n--> invalid expression at %lu:%lu\n|\n|", 
                    yel_tokens->file_name, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);
                yel_print_error(yel_tokens->src_ptr, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);
                yel_tokens->error = 1;
                return en_end;
            }

            // a op ...
            else if (yel_tokens->type[i + 1] == tok_op) {

                // a ( ...
                if (__builtin_strcmp(yel_tokens->value[i + 1], "(") == 0) {
                    return last_type = en_expr;
                }

                else if (__builtin_strcmp(yel_tokens->value[i + 1], ";") == 0) {
                    return last_type = en_expr;
                }  
            }
            else {
                printf("Syntax Error: <File '%s'>\n--> expression is expected: '%s' at %lu:%lu\n|\n|", 
                    yel_tokens->file_name, next, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_print_error(yel_tokens->src_ptr, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);
                
                yel_tokens->error = 1;
                return en_end;
            }
        }

        // 1 ...
        else if (yel_tokens->type[i] == tok_number) {
            if (__builtin_strcmp(yel_tokens->value[i + 1], "=") == 0) {
                printf("Syntax Error: <File '%s'>\n--> expression must be non-literal or modifiable at %lu:%lu\n|\n|",
                    yel_tokens->file_name, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_print_error(yel_tokens->src_ptr, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_tokens->error = 1;
                return en_end;
            }
            else if (yel_tokens->type[i+1] != tok_binary_op && yel_tokens->type[i+1] != tok_op && __builtin_strcmp(yel_tokens->value[i+1], ";") != 0) {
                printf("Syntax Error: <File '%s'>\n--> expression is expected after '%s' at %lu:%lu\n|\n|", 
                    yel_tokens->file_name, cur, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_print_error(yel_tokens->src_ptr, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_tokens->error = 1;
                return en_end;
            }

            last_type = en_expr;
            size_t tmp_i = i;
            ++yel_tokens->pointer;

            if (yel_define_next_entity(yel_tokens) == en_expr) {
                yel_tokens->pointer = i;
                last_type = en_end;
                return en_expr;
            }

            printf("Syntax Error: <File '%s'>\n--> invalid expression at %lu:%lu\n|\n|", 
                yel_tokens->file_name, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

            yel_print_error(yel_tokens->src_ptr, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

            yel_tokens->error = 1;
            return en_end;
        }

        else if (yel_tokens->type[i] == tok_word) {

            // return ...
            if (__builtin_strcmp(yel_tokens->value[i], "return") == 0) {
                last_type = en_end;
                return en_stmt;
            }

            // break ...
            else if (__builtin_strcmp(yel_tokens->value[i], "break") == 0) {
                last_type = en_end;
                return en_stmt;
            }
        }
        else if (yel_tokens->type[i] == tok_string) {
            last_type = en_end;
            return en_expr;
        }

        else if(yel_tokens->type[i] == tok_op) {
            if (__builtin_strcmp(yel_tokens->value[i], "(") == 0) {
                last_type = en_expr;
                size_t tmp_i = i;
                ++yel_tokens->pointer;

                if (yel_define_next_entity(yel_tokens) == en_expr) {
                    yel_tokens->pointer = i;
                    last_type = en_end;
                    return en_expr;
                }
                printf("Syntax Error: <File '%s'>\n--> invalid expression at %lu:%lu\n|\n|", 
                    yel_tokens->file_name, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_print_error(yel_tokens->src_ptr, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_tokens->error = 1;
                last_type = en_end;
                return en_end;
            }
            else if (__builtin_strcmp(yel_tokens->value[i], "(") == 0) {
                last_type = en_end;
                return en_expr;
            }
        }
        else if(yel_tokens->type[i] == tok_binary_op) {
            // unary
            /*if (strcmp(yel_tokens->value[i], "-") == 0 || strcmp(yel_tokens->value[i], "+") == 0) {
                last_type = en_expr;
                size_t tmp_i = i;
                ++yel_tokens->pointer;

                if (yel_define_next_entity(yel_tokens) == en_expr) {
                    yel_tokens->pointer = i;

                    yel_tokens->type[i] = tok_unary_op;

                    return en_expr;
                }
                printf("Syntax Error: <File '%s'>\n--> expression is expected at %lu:%lu\n|\n|", 
                    yel_tokens->file_name, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_print_error(yel_tokens->src_ptr, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_tokens->error = 1;
                return en_end;
            }

            else */if (yel_tokens->value[i+1][0] == ';' || (yel_tokens->type[i+1] != tok_name && yel_tokens->type[i+1] != tok_number)) {
                printf("Syntax Error: <File '%s'>\n--> expression is expected after '%s' at %lu:%lu\n|\n|", 
                    yel_tokens->file_name, cur, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_print_error(yel_tokens->src_ptr, yel_tokens->line[yel_tokens->pointer], yel_tokens->start_symbol[yel_tokens->pointer]);

                yel_tokens->error = 1;
                return en_end;
            }

            return en_expr;
        }

        ++i;
    }
}

void yel_gen_parse_tree(YelTokens* yel_tokens) {
    while (yel_tokens->pointer < yel_tokens->length) {
        switch (yel_define_next_entity(yel_tokens)) {
        case (YelEntities)en_stmt: 
            yel_parse_statement(yel_tokens);
            recursive_descent = 0;
            break;
        case (YelEntities)en_decl: 
            break;
        case (YelEntities)en_expr: 
            yel_parse_expression(yel_tokens);
            recursive_descent = 0;
            break;
        case (YelEntities)en_end: 
            break;
        }

        if (yel_tokens->error) {
            return;
        }

        ++yel_tokens->pointer;
    }
}