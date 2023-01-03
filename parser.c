#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

/*
    Jose Valencia 
    COP3402 Fall 22 Compiler Project Part 4
    12/04/2022
*/

lexeme *tokens;
int token_index = 0;
symbol *table;
int table_index = 0;
instruction *code;
int code_index = 0;

int error = 0;
int level;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void emit(int op, int l, int m);
void add_symbol(int kind, char name[], int value, int level, int address);
void mark();
int multiple_declaration_check(char name[]);
int find_symbol(char name[], int kind);

void print_parser_error(int error_code, int case_code);
void print_assembly_code();
void print_symbol_table();

void program();
void block();
int declarations(); 
void constants();
void variables(int numVars);
void procedures();
void statment();
void condition();
void expression();
void term();
void factor();

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

instruction *parse(int code_flag, int table_flag, lexeme *list)
{
	// variable setup
	int i;
	tokens = calloc(ARRAY_SIZE, sizeof(lexeme));
	table = calloc(ARRAY_SIZE, sizeof(symbol));
	code = calloc(ARRAY_SIZE, sizeof(instruction));
	FILE *ifp;
	int buffer;
	

	for(int i = 0; i < ARRAY_SIZE; i++) {
		tokens[i] = list[i];
	}

	token_index = 0;
	
	
	program();
	if(code_flag)
		print_assembly_code();
	if(table_flag)
		print_symbol_table();
	
	emit(-1, -1, -1);

	free(tokens);
	free(table);
	//free(code);

	if(error == 1)
		return NULL;
		
	return code;
}

//Function to start parsing a program, it emits the first and last instruction and calls block in between. 
void program()
{
	add_symbol(3, "main", 0, 0, 0);
	level = -1;
	emit(JMP, 0, 0);
	block();

	if(error == 1)
	{
		return;
	}
	if(tokens[token_index].type !=  period)
	{
		print_parser_error(1, 0);
		error = 1;
		return; 
	}
	
	for(int i = 0; i < code_index; i++)
	{
		if(code[i].op == CAL)
		{
			code[i].m = table[code[i].m].address;
		}
	}
	code[0].m = table[0].address;
	
	emit(SYS, 0, HLT);

}

//Function where the 3 main parts of a program or procedure parse. Block can be call in a block call within procedures for subprocedures.
void block()
{
	int last_proc = table_index-1;
	level++;
	int inc_m_value = declarations();
	if(error == 1)
	{
		return;
	}
	procedures();
	if(error == 1)
	{
		return;
	}
	emit(INC, 0, inc_m_value);
	int proc_address = (code_index - 1)*3;
	table[last_proc].address = proc_address;
	statment();
	if(error == 1)
	{
		return;
	}
	mark();
	level--;
	return;
}

//Checks for the declaration of any variables at the start of a procedure. Returns number of consts and vars found.
int declarations() 
{
	int number_declared = 0;
	while(tokens[token_index].type == keyword_const || tokens[token_index].type == keyword_var)
	{
		if(tokens[token_index].type == keyword_const)
		{
			constants();
			if(error == 1)
			{
				return -1;
			}
		} else 
		{
			variables(number_declared);
			if(error == 1)
			{
				return -1;
			}
			number_declared++;
		}
	}
	return number_declared + 3;
}

//Parses any constant declared and adds it to the symbol table.
void constants() 
{
	int minus_flag = 0;
	token_index++;
	if(tokens[token_index].type != identifier)
	{
		print_parser_error(2, 1);
		error = 1;
		return;
	}
	if(multiple_declaration_check(tokens[token_index].identifier_name) != -1)
	{
		print_parser_error(3, 0);
		error = 1;
		return;
	}
	char identifier_name[12]; 
	strcpy(identifier_name, tokens[token_index].identifier_name);
	token_index++;
	if(tokens[token_index].type != assignment_symbol)
	{
		print_parser_error(4, 1);
		error = 1;
		return;
	}
	token_index++;
	if(tokens[token_index].type == minus)
	{
		minus_flag = 1;
		token_index++;
	}
	if(tokens[token_index].type != number)
	{
		print_parser_error(5, 0);
		error = 1;
		return;
	}
	int number_value = tokens[token_index].number_value;
	token_index++;
	if(minus_flag == 1)
	{
		number_value *= -1;
	}
	add_symbol(1, identifier_name, number_value, level, 0);
	if(tokens[token_index].type != semicolon)
	{
		print_parser_error(6, 1);
		error = 1;
		return;
	}
	token_index++;
	return;
}

//Parses any variable declared and adds it to the symbol table.
void variables(int numVars) 
{
	token_index++;;
	if(tokens[token_index].type != identifier) 
	{
		print_parser_error(2, 2);
		error = 1;
		return;
	}
	if(multiple_declaration_check(tokens[token_index].identifier_name) != -1)
	{
		print_parser_error(3, 0);
		error = 1;
		return;
	}
	char identifier_name[12]; 
	strcpy(identifier_name, tokens[token_index].identifier_name);
	token_index++;
	add_symbol(2, identifier_name, 0, level, numVars+3);
	if(tokens[token_index].type != semicolon)
	{
		print_parser_error(6, 2);
		error = 1;
		return;
	}
	token_index++;
	return;
}

//Parses proceudres. The inside of a procedure is just a whole other block up a lex level.
void procedures()
{
	while(tokens[token_index].type == keyword_procedure)
	{
		token_index++;
		if(tokens[token_index].type != identifier)
		{
			print_parser_error(2, 3);
			error = 1;
			return;
		}
		if(multiple_declaration_check(tokens[token_index].identifier_name) != -1)
		{
			print_parser_error(3, 0);
			error = 1;
			return;
		}
		char identifier_name[12]; 
		strcpy(identifier_name, tokens[token_index].identifier_name);
		token_index++;
		add_symbol(3, identifier_name, 0, level, 0);
		if(tokens[token_index].type != left_curly_brace)
		{
			print_parser_error(14, 0);
			error = 1;
			return;
		}
		token_index++;
		block();
		if(error == 1)
		{
			return;
		}
		emit(OPR, 0, RTN);
		if(tokens[token_index].type != right_curly_brace)
		{
			print_parser_error(15, 0);
			error = 1;
			return;
		}
		token_index++;
		
	}

	return;
}

//Parses individual statments whichs start with keywords.
void statment()

{
	if(tokens[token_index].type == keyword_def)
	{
		token_index++;

		if(tokens[token_index].type != identifier)
		{
			print_parser_error(2, 6);
			error = 1;
			return;
		}
		
		int symbol_index = find_symbol(tokens[token_index].identifier_name, 2);
		if(symbol_index == -1) 
		{
			if(find_symbol(tokens[token_index].identifier_name, 1) == find_symbol(tokens[token_index].identifier_name, 3))
			{
				print_parser_error(8, 1);
				error = 1;
				return;
			} else
			{
				print_parser_error(7, 0);
				error = 1;
				return;
			}
		}
		
		token_index++;
		if(tokens[token_index].type != assignment_symbol) 
		{
			print_parser_error(4, 2);
			error = 1;
			return;
		}
		token_index++;
		expression();
		if(error == 1)
		{
			return;
		}
		emit(STO, level - table[symbol_index].level, table[symbol_index].address);

	} else if (tokens[token_index].type == keyword_call)
	{
		token_index++;
		if(tokens[token_index].type != identifier)
		{
			print_parser_error(2, 4);
			error = 1;
			return;
		}
		int symbol_index = find_symbol(tokens[token_index].identifier_name, 3);
		if(symbol_index == -1) 
		{
			if(find_symbol(tokens[token_index].identifier_name, 1) == find_symbol(tokens[token_index].identifier_name, 2))
			{
				print_parser_error(8, 2);
				error = 1;
				return;
			} else 
			{
				print_parser_error(9, 0);
				error = 1;
				return;
			}
		}
		token_index++;
		emit(CAL, level - table[table_index].level, symbol_index);
	} else if (tokens[token_index].type == keyword_begin)
	{
		do {
			token_index++;
			statment();
			if(error == 1)
			{
				return;
			}
		} while (tokens[token_index].type == semicolon);
		
		if(tokens[token_index].type != keyword_end)
		{
			if(tokens[token_index].type == identifier || tokens[token_index].type == keyword_call ||
			   tokens[token_index].type == keyword_begin || tokens[token_index].type == keyword_read || tokens[token_index].type == keyword_def ||
			   tokens[token_index].type == keyword_if || tokens[token_index].type == keyword_while || tokens[token_index].type == keyword_write)
			{
				print_parser_error(6, 3);
				error = 1;
				return;

			} else 
			{
				print_parser_error(10, 0);
				error = 1;
				return;
			}
		}
		token_index++;		
	} else if(tokens[token_index].type == keyword_read)
	{
		token_index++;
		if(tokens[token_index].type != identifier)
		{
			print_parser_error(2, 5);
			error = 1;
			return;
		}	

		int symbol_index = find_symbol(tokens[token_index].identifier_name, 2);
		if(symbol_index == -1)
		{
			if(find_symbol(tokens[token_index].identifier_name, 1) == find_symbol(tokens[token_index].identifier_name, 3))
			{
				print_parser_error(8, 3);
				error = 1;
				return;

			} else 
			{
				print_parser_error(13, 0);
				error = 1;
				return;
			}
		}
		token_index++;
		emit(SYS, 0, RED);
		emit(STO, level - table[symbol_index].level, table[symbol_index].address);

	} else if(tokens[token_index].type == keyword_if)
	{
		token_index++;
		condition();
		if(error == 1)
		{
			return;
		}
		emit(JPC, 0, 0);
		int jpc_index = code_index - 1;
		if(tokens[token_index].type != keyword_then)
		{
			print_parser_error(11, 0);
			error = 1;
			return;
		}
		token_index++;
		statment();
		if(error == 1)
		{
			return;
		}
		if(tokens[token_index].type == keyword_else)
		{
			token_index++;
			emit(JMP, 0, 0);
			int jmp_index = code_index - 1;
			code[jpc_index].m = code_index * 3;
			
			statment();
			if(error == 1)
			{
				return;
			}
			code[jmp_index].m = code_index * 3;
		} else
		{
			code[jpc_index].m = code_index * 3;
		}

	} else if(tokens[token_index].type == keyword_while)
	{
		token_index++;

		int loop_index = code_index;
		condition();
		if(error == 1)
		{
			return;
		}
		if(tokens[token_index].type != keyword_do)
		{
			print_parser_error(12, 0);
			error = 1;
			return;
		}
		token_index++;
		emit(JPC, 0, 0);
		int jpc_index = code_index - 1;
		statment();
		if(error == 1)
		{
			return;
		}
		emit(JMP, 0, loop_index * 3);
		code[jpc_index].m = code_index * 3;
	} else if(tokens[token_index].type == keyword_write)
	{
		token_index++;
		expression();
		if(error == 1)
		{
			return;
		}
		emit(SYS, 0, WRT);
	}
	return;
}

void condition()
{
	expression();
	if(error == 1)
	{
		return;
	}
	if(tokens[token_index].type != equal_to && tokens[token_index].type != not_equal_to &&
		tokens[token_index].type != less_than && tokens[token_index].type != less_than_or_equal_to &&
		tokens[token_index].type != greater_than && tokens[token_index].type != greater_than_or_equal_to)
	{
		print_parser_error(16, 0);
		error = 1;
		return;
	}
	int symbol  = tokens[token_index].type;
	token_index++;
	expression();
	if(error == 1)
	{
		return;
	}
	if(symbol == equal_to) emit(OPR, 0, EQL);
	else if(symbol == not_equal_to) emit(OPR, 0, NEQ);
	else if(symbol == less_than) emit(OPR, 0, LSS);
	else if(symbol == less_than_or_equal_to) emit(OPR, 0, LEQ);
	else if(symbol == greater_than) emit(OPR, 0, GTR);
	else if(symbol == greater_than_or_equal_to) emit(OPR, 0, GEQ);

	return;
}

void expression()
{
	term();
	if(error == 1) 
	{
		return;
	}
	while(tokens[token_index].type == plus || tokens[token_index].type == minus)
	{
		int symbol = (tokens[token_index].type == plus ? 0 : 1);
		token_index++;
		term();
		if(error == 1)
		{
			return;
		}
		if(symbol == 0)
		{
			emit(OPR, 0, ADD);

		}
		else
		{
			emit(OPR, 0, SUB);
		}
		
	}

	return;
}

void term()
{
	factor();
	if(error == 1) 
	{
		return;
	}
	while(tokens[token_index].type == times || tokens[token_index].type == division)
	{
		int symbol = (tokens[token_index].type == times ? 0 : 1);
		token_index++;
		factor();
		if(error == 1)
		{
			return;
		}
		if(symbol == 0)
		{
			emit(OPR, 0, MUL);

		}
		else
		{
			emit(OPR, 0, DIV);
		}
		
	}

	return;
}

//Parses the identifiers and numbers.
void factor() 
{
    if(tokens[token_index].type == identifier) 
    {
        int constant_index = find_symbol(tokens[token_index].identifier_name, 1);
        int variable_index = find_symbol(tokens[token_index].identifier_name, 2);

        if(constant_index == variable_index) 
        {
            if(find_symbol(tokens[token_index].identifier_name, 3) == -1)
            {
                print_parser_error(8, 4); 
                error = 1; 
                return;
            } 
            else
            {
                print_parser_error(17, 0);
                error = 1;
                return;
            }
        }

        if(constant_index == -1) 
        {
            emit(LOD, level - table[variable_index].level, table[variable_index].address);
        }
        else if(variable_index == -1)
        {
            emit(LIT, 0, table[constant_index].value);
        }
        else if(table[constant_index].level > table[variable_index].level)
        {
            emit(LIT, 0, table[constant_index].value);
        }
        else
        {
            emit(LOD, level - table[variable_index].level, table[variable_index].address);
        }

        token_index++;
    }
    else if(tokens[token_index].type == number)
    {
        emit(LIT, 0, tokens[token_index].number_value);
        token_index++;
    }
	else if(tokens[token_index].type == left_parenthesis) {

		token_index++;

		expression();
		if(error == 1)
		{
			return;
		}
		if(tokens[token_index].type != right_parenthesis)
		{
			print_parser_error(18, 0);
			error = 1;
			return;
		}

		token_index++;
	}
    else 
    {
        print_parser_error(19, 0);
        error = 1;
        return;
    }

    return;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
// adds a new instruction to the end of the code
void emit(int op, int l, int m)
{
	code[code_index].op = op;
	code[code_index].l = l;
	code[code_index].m = m;
	code_index++;
}

// adds a new symbol to the end of the table
void add_symbol(int kind, char name[], int value, int level, int address)
{
	table[table_index].kind = kind;
	strcpy(table[table_index].name, name);
	table[table_index].value = value;
	table[table_index].level = level;
	table[table_index].address = address;
	table[table_index].mark = 0;
	table_index++;
}

// marks all of the current procedure's symbols
void mark()
{
	int i;
	for (i = table_index - 1; i >= 0; i--)
	{
		if (table[i].mark == 1)
			continue;
		if (table[i].level < level)
			return;
		table[i].mark = 1;
	}
}

// returns -1 if there are no other symbols with the same name within this procedure
int multiple_declaration_check(char name[])
{
	int i;
	for (i = 0; i < table_index; i++)
		if (table[i].mark == 0 && table[i].level == level && strcmp(name, table[i].name) == 0)
			return i;
	return -1;
}

// returns the index of the symbol with the desired name and kind, prioritizing 
// 		symbols with level closer to the current level
int find_symbol(char name[], int kind)
{
	int i;
	int max_idx = -1;
	int max_lvl = -1;
	for (i = 0; i < table_index; i++)
	{
		if (table[i].mark == 0 && table[i].kind == kind && strcmp(name, table[i].name) == 0)
		{
			if (max_idx == -1 || table[i].level > max_lvl)
			{
				max_idx = i;
				max_lvl = table[i].level;
			}
		}
	}
	return max_idx;
}

void print_parser_error(int error_code, int case_code)
{
	switch (error_code)
	{
		case 1 :
			printf("Parser Error 1: missing . \n");
			break;
		case 2 :
			switch (case_code)
			{
				case 1 :
					printf("Parser Error 2: missing identifier after keyword const\n");
					break;
				case 2 :
					printf("Parser Error 2: missing identifier after keyword var\n");
					break;
				case 3 :
					printf("Parser Error 2: missing identifier after keyword procedure\n");
					break;
				case 4 :
					printf("Parser Error 2: missing identifier after keyword call\n");
					break;
				case 5 :
					printf("Parser Error 2: missing identifier after keyword read\n");
					break;
				case 6 :
					printf("Parser Error 2: missing identifier after keyword def\n");
					break;
				default :
					printf("Implementation Error: unrecognized error code\n");
			}
			break;
		case 3 :
			printf("Parser Error 3: identifier is declared multiple times by a procedure\n");
			break;
		case 4 :
			switch (case_code)
			{
				case 1 :
					printf("Parser Error 4: missing := in constant declaration\n");
					break;
				case 2 :
					printf("Parser Error 4: missing := in assignment statement\n");
					break;
				default :				
					printf("Implementation Error: unrecognized error code\n");
			}
			break;
		case 5 :
			printf("Parser Error 5: missing number in constant declaration\n");
			break;
		case 6 :
			switch (case_code)
			{
				case 1 :
					printf("Parser Error 6: missing ; after constant declaration\n");
					break;
				case 2 :
					printf("Parser Error 6: missing ; after variable declaration\n");
					break;
				case 3 :
					printf("Parser Error 6: missing ; after statement in begin-end\n");
					break;
				default :				
					printf("Implementation Error: unrecognized error code\n");
			}
			break;
		case 7 :
			printf("Parser Error 7: procedures and constants cannot be assigned to\n");
			break;
		case 8 :
			switch (case_code)
			{
				case 1 :
					printf("Parser Error 8: undeclared identifier used in assignment statement\n");
					break;
				case 2 :
					printf("Parser Error 8: undeclared identifier used in call statement\n");
					break;
				case 3 :
					printf("Parser Error 8: undeclared identifier used in read statement\n");
					break;
				case 4 :
					printf("Parser Error 8: undeclared identifier used in arithmetic expression\n");
					break;
				default :				
					printf("Implementation Error: unrecognized error code\n");
			}
			break;
		case 9 :
			printf("Parser Error 9: variables and constants cannot be called\n");
			break;
		case 10 :
			printf("Parser Error 10: begin must be followed by end\n");
			break;
		case 11 :
			printf("Parser Error 11: if must be followed by then\n");
			break;
		case 12 :
			printf("Parser Error 12: while must be followed by do\n");
			break;
		case 13 :
			printf("Parser Error 13: procedures and constants cannot be read\n");
			break;
		case 14 :
			printf("Parser Error 14: missing {\n");
			break;
		case 15 :
			printf("Parser Error 15: { must be followed by }\n");
			break;
		case 16 :
			printf("Parser Error 16: missing relational operator\n");
			break;
		case 17 :
			printf("Parser Error 17: procedures cannot be used in arithmetic\n");
			break;
		case 18 :
			printf("Parser Error 18: ( must be followed by )\n");
			break;
		case 19 :
			printf("Parser Error 19: invalid expression\n");
			break;
		default:
			printf("Implementation Error: unrecognized error code\n");

	}
}

void print_assembly_code()
{
	int i;
	printf("Assembly Code:\n");
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < code_index; i++)
	{
		printf("%d\t%d\t", i, code[i].op);
		switch(code[i].op)
		{
			case LIT :
				printf("LIT\t");
				break;
			case OPR :
				switch (code[i].m)
				{
					case RTN :
						printf("RTN\t");
						break;
					case ADD : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("ADD\t");
						break;
					case SUB : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("SUB\t");
						break;
					case MUL : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("MUL\t");
						break;
					case DIV : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("DIV\t");
						break;
					case EQL : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("EQL\t");
						break;
					case NEQ : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("NEQ\t");
						break;
					case LSS : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("LSS\t");
						break;
					case LEQ : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("LEQ\t");
						break;
					case GTR : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("GTR\t");
						break;
					case GEQ : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("GEQ\t");
						break;
					default :
						printf("err\t");
						break;
				}
				break;
			case LOD :
				printf("LOD\t");
				break;
			case STO :
				printf("STO\t");
				break;
			case CAL :
				printf("CAL\t");
				break;
			case INC :
				printf("INC\t");
				break;
			case JMP :
				printf("JMP\t");
				break;
			case JPC : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
				printf("JPC\t");
				break;
			case SYS :
				switch (code[i].m)
				{
					case WRT : // DO NOT ATTEMPT TO IMPLEMENT THIS, YOU WILL GET A ZERO IF YOU DO
						printf("WRT\t");
						break;
					case RED :
						printf("RED\t");
						break;
					case HLT :
						printf("HLT\t");
						break;
					default :
						printf("err\t");
						break;
				}
				break;
			default :
				printf("err\t");
				break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
	}
	printf("\n");
}

void print_symbol_table()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i < table_index; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].value, table[i].level, table[i].address, table[i].mark); 
	printf("\n");
}
