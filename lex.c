#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

/*
    Jose Valencia 
    COP3402 Fall 22 Compiler Project Part 4
    12/04/2022
*/

#define MAX_NUMBER_LENGTH 5
#define MAX_IDENTIFIER_LENGTH 11
#define NUM_KEYWORDS 14

int isWhiteSpace(char c);
int readStatement(const char* src, int srcLength, char* st, int max);
int isAlpha(char c);
int isNumber(char c);
int isSymbol(char c);
int isValidSingleSymbol(char c);
int isInvalidIdentifier(char* id, int len);
int isKeyword(char* id, int len);
void printValidLexeme(const char* lex, int type);
void processStatement(char* st, int len);

char validSingleSymbols[] = { '.', '-', ';', '{', '}', '>', '<', '+', '*', '/', '(', ')' };
char validSymbols[] = { '.', ':', '=','-', ';', '{', '}', '>', '<', '+', '*', '/', '(', ')' };
const char* keywords[NUM_KEYWORDS] = { "const", "var", "procedure", "call", "begin", "end", "if", "then", "else", "while", "do", "read", "write", "def" };
token_type singleSymbolTokens[] = { period, minus, semicolon, left_curly_brace, right_curly_brace, greater_than, less_than, plus, times, division, left_parenthesis, right_parenthesis};
lexeme *lex;
int lexIndex = 0;
int error_flag = 0;
int print_flag = 0;


lexeme* lex_analyze(int list_flag, char* input)
{

    print_flag = list_flag;
    lex = calloc(ARRAY_SIZE, sizeof(lexeme));
    int statementLength = 0;
    char statement[ARRAY_SIZE] = { 0 };
    int currentOffset = 0;
    int inputStrLength = strlen(input);
    if(list_flag) {
        printf("Lexeme List:\n");
        printf("%11s\t%s\n", "lexeme", "token type");
    }
    while ((statementLength = readStatement(input+currentOffset, strlen(input)-currentOffset, statement, ARRAY_SIZE)) > 0)
    {
        statement[statementLength] = 0;
        processStatement(statement, statementLength);
        currentOffset += statementLength;
    }

    if(error_flag == 1)
        return NULL;

    return lex;
}


int isWhiteSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' ? 1 : 0;
}

int readStatement(const char* src, int srcLength, char* st, int max)
{
    char ch;
    int last_index = 0;
    int hasStarted = 0;
    int isComment = 0;
    while (last_index < srcLength && (ch = src[last_index]) != '\0')
    {
        st[last_index] = ch;
        last_index++;

        if (isWhiteSpace(ch) == 0 && hasStarted == 0)
        {
            hasStarted = 1;
            if (ch == '#')
            {
                isComment = 1;
            }
        }

        if (last_index == max)
        {
            return last_index;
        }
        if (ch == '\n' || (ch == ';' && isComment == 0))
        {
            return last_index;
        }
    }
    return last_index;
}

int isAlpha(char c)
{
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' ? 1 : 0;
}

int isNumber(char c)
{
    return c >= '0' && c <= '9' ? 1 : 0;
}

int isSymbol(char c)
{
    char* ptr = strchr(validSymbols, c);
    return ptr != NULL ? (ptr - validSymbols) + 1 : 0;
}

int isValidSingleSymbol(char c)
{
    char* ptr = strchr(validSingleSymbols, c);
    return ptr != NULL ? (ptr - validSingleSymbols) + 1 : 0;
}

int isInvalidIdentifier(char* id, int len)
{
    return strncmp(id, "null", len) == 0 || strncmp(id, "main", len) == 0 ? 1 : 0;
}

int isKeyword(char* id, int len)
{
    for (int i = 0; i < NUM_KEYWORDS; ++i)
    {
        if (strncmp(keywords[i], id, strlen(keywords[i])) == 0)
        {
            return i + 1;
        }
    }
    return 0;
}

void printValidLexeme(const char* lex, int type)
{
    if(print_flag == 1)
        printf("%-11s\t%d\n", lex, type);
}

void processStatement(char* st, int len)
{
    if (len == 0)
    {
        return;
    }

    int index = 0;
    // skip over white space
    while (isWhiteSpace(st[index]))
    {
        ++index;
    }

    char tmpstr[12] = { 0 };

    while (index < len)
    {
        if (isNumber(st[index]) == 1)
        {
            // Special case should be only a number and nothing else
            int start = index;
            int numCount = 0;
            while (isSymbol(st[index]) == 0 && isWhiteSpace(st[index]) == 0)
            {
                if (isNumber(st[index]) == 1)
                {
                    ++numCount;
                }
                ++index;
            }

            if (numCount > MAX_NUMBER_LENGTH)
            {
                // ERROR NUMBER HAS TOO MANY CHARACTERS
                printf("ERROR: NUMBERS CAN BE MAX 5 DIGITS.\n");
                error_flag = 1;
            }
            else if (numCount != index - start)
            {
                // ERROR NUMBER CONTAINS ALPHA CHARACTERS AND IS LIKELY AN INVALID IDENTIFIER
                printf("ERROR: IDENTIFIERS CAN'T START WITH NUMBERS.\n");
                error_flag = 1;
            }
            else {
                //Valid number
                strncpy(tmpstr, st + start, index - start);
                tmpstr[index - start] = '\0';
                printValidLexeme(tmpstr, number);
                lex[lexIndex].type = number;
                lex[lexIndex].number_value = atoi(tmpstr);
                lexIndex++;
            }
        }
        else if (isAlpha(st[index]) == 1)
        {
            // keyword or identifier
            int start = index;
            while (isSymbol(st[index]) == 0 && isWhiteSpace(st[index]) == 0)
            {
                ++index;
            }

            if (index - start > MAX_IDENTIFIER_LENGTH)
            {
                // ERROR IDENTIFIER IS TOO LONG
                printf("ERROR: IDENTIFIER MAX LENGTH IS 11\n");
                error_flag = 1;
            }
            else if (isInvalidIdentifier(st + start, index - start) == 1)
            {
                // ERROR INVALID IDENTIFIER
                printf("ERROR: IDENTIFIER CAN NOT BE NULL OR MAIN\n");
                error_flag = 1;
            }
            else if (isKeyword(st + start, index - start) > 0)
            {
                // IS A VALID KEYWORD
                int keyWordIndex = isKeyword(st + start, index - start);
                printValidLexeme(keywords[keyWordIndex - 1], keyWordIndex + 2);
                lex[lexIndex].type = keyWordIndex+2;
                lexIndex++;
            }
            else
            {
                // IS A VALID IDENTIFIER
                strncpy(tmpstr, st + start, index - start);
                tmpstr[index - start] = '\0';
                printValidLexeme(tmpstr, identifier);
                lex[lexIndex].type = identifier;
                strcpy(lex[lexIndex].identifier_name, tmpstr);
                lexIndex++;
            }
        }
        else if (isWhiteSpace(st[index]) == 1)
        {
            ++index;
        }
        else
        {
            // symbol
            int start = index;
            if (st[index] == ';')
            {
                printValidLexeme(";", semicolon);
                lex[lexIndex].type = semicolon;
                lexIndex++;
            }
            else if (st[index] == '#')
            {
                // THIS ENTIRE STATEMENT IS A COMMENT
                return;
            }
            else if (st[index] == ':' || st[index] == '=')
            {
                // an equal sign is required next
                if (index + 1 == len)
                {
                    // end of line
                    // ERROR INVALID SYMBOL COLON ONLY OR SINGLE =
                    // ++index that follows else if will make index greater than len
                    printf("ERROR:INVALID SYMBOL\n");
                    error_flag = 1;
                }
                else if (st[index + 1] != '=')
                {
                    // ERROR INVALID SYMBOL, ASSIGNMENT OPERATOR MUST BE := OR EQUALITY MUST BE ==
                    printf("ERROR:INVALID SYMBOL\n");
                    error_flag = 1;
                }
                else
                {
                    if (st[index] == ':') {
                        printValidLexeme(":=", assignment_symbol);
                        lex[lexIndex].type = assignment_symbol;
                        lexIndex++;
                    }
                    else {
                        printValidLexeme("==", equal_to);
                        lex[lexIndex].type = equal_to;
                        lexIndex++;
                    }
                    // VALID ASSIGNMENT OPERATOR := OR EQUALITY OPERATOR
                    // need additional increment because we did a look-ahead
                    ++index;
                }
            }
            else if (st[index] == '<')
            {
                if (index + 1 == len)
                {
                    // VALID LESS THAN
                    printValidLexeme("<", less_than);
                    lex[lexIndex].type = less_than;
                    lexIndex++;
                }
                else if (isSymbol(st[index + 1]) > 0)
                {
                    if (st[index + 1] == '=' || st[index + 1] == '>')
                    {
                        // VALID SYMBOL
                        if (st[index + 1] == '=') {
                            printValidLexeme("<=", less_than_or_equal_to);
                            lex[lexIndex].type = less_than_or_equal_to;
                            lexIndex++;
                        }
                        else {
                            printValidLexeme("<>", not_equal_to);
                            lex[lexIndex].type = not_equal_to;
                            lexIndex++;
                        }
                        ++index;
                    }
                    else
                    {
                        // VALID LESS THAN OPERATOR
                        printValidLexeme("<", less_than);
                        lex[lexIndex].type = less_than;
                        lexIndex++;
                    }
                }
                else
                {
                    // VALID LESS THAN OPERATOR
                    printValidLexeme("<", less_than);
                    lex[lexIndex].type = less_than;
                    lexIndex++;
                }
            }
            else if (st[index] == '>')
            {
                if (index + 1 == len)
                {
                    // VALID GREATER THAN OPERATOR
                    printValidLexeme(">", greater_than);
                    lex[lexIndex].type = greater_than;
                    lexIndex++;
                }
                else if (isSymbol(st[index + 1]) > 0)
                {
                    if (st[index + 1] == '=')
                    {
                        // VALID GREATER THAN OR EQUAL SYMBOL
                        printValidLexeme(">=", greater_than_or_equal_to);
                        lex[lexIndex].type = greater_than_or_equal_to;
                        lexIndex++;
                        ++index;
                    }
                    else
                    {
                        // VALID GREATER THAN OPERATOR
                        printValidLexeme(">", greater_than);
                        lex[lexIndex].type = greater_than;
                        lexIndex++;
                    }
                }
                else
                {
                    // VALID GREATER THAN OPERATOR
                    printValidLexeme(">", greater_than);
                    lex[lexIndex].type = greater_than;
                    lexIndex++;
                }
            }
            else if (isValidSingleSymbol(st[index]) > 0)
            {
                // IS A VALID SYMBOL
                int symbolIndex = isValidSingleSymbol(st[index]);
                char temp[2] = { 0 };
                temp[0] = st[index];
                printValidLexeme(temp, singleSymbolTokens[symbolIndex-1]);
                lex[lexIndex].type = singleSymbolTokens[symbolIndex - 1];
                lexIndex++;
            }
            else if(isSymbol(st[index]) == 0)
            {
                // INVALID SYMBOL
                printf("ERROR:INVALID SYMBOL\n");
                error_flag = 1;
            }
            else
            {
                // THIS SHOULD NEVER HAPPEN OR WE DID SOMETHING VERY WRONG
            }

            ++index;
        }
    }
}


