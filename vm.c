#include <stdio.h>
#include <string.h>
#include "compiler.h"

/*
    Jose Valencia 
    COP3402 Fall 22 Compiler Project Part 4
    12/04/2022
*/

//GLOBALS
int pas[ARRAY_SIZE] = {0};
int bars[ARRAY_SIZE] = {0};
int bp, sp, pc, gp;
instruction ir;
int halt = 1;

//FUNCTION DECLARATION
int base(int *pas, int BP, int L);
void print_stack(int pc, int bp, int sp, int gp, int * pas, int * bars);
void print_instruction(int PC, instruction IR);

//MAIN FUNCTION
void execute(int trace_flag, instruction *code){

    int index = 0;
    int pas_index = 0;
    while(code[index].m != -1) {

        pas[pas_index] = code[index].op;
        pas[pas_index+1] = code[index].l;
        pas[pas_index+2] = code[index].m;

        pas_index += 3;
        index++;
    }

    //FILLING IN POINTERS
    bp = pas_index;
    gp = pas_index;
    sp = bp-1;
    pc = 0;

    if(trace_flag) {
    //HEADER FOR PRINT FORMAT
    printf("                \tPC\tBP\tSP\tStack\n");
    printf("Initial values: \t%d\t%d\t%d\n\n", pc, bp, sp);
    }

    //MAIN WHILE 
    while(halt) {

        //FILLING IN IR
        ir.op = pas[pc];
        ir.l = pas[pc+1];
        ir.m = pas[pc+2];

        pc = pc + 3;

        //MAIN SWITCH FOR INSTRUCTIONS
        switch(ir.op) {
        
            //LIT INSTRUCTION
            case 1:
                sp++;
                pas[sp] = ir.m;
                break; 
            //OPR INSTRUCTION           
            case 2:
                switch(ir.m){
                    //RTN
                    case 0:
                        sp = bp - 1;
                        bp = pas[sp + 2];
                        pc = pas[sp + 3];
                        break;
                    //ADD
                    case 1:
                        pas[sp - 1] = pas[sp - 1] + pas[sp];
                        sp = sp - 1;
                        break;
                    //SUB
                    case 2:
                        pas[sp - 1] = pas[sp - 1] - pas[sp];
                        sp = sp - 1;
                        break;
                    //MUL
                    case 3:
                        pas[sp - 1] = pas[sp - 1] * pas[sp];
                        sp = sp - 1;
                        break;
                    //DIV
                    case 4:
                        pas[sp - 1] = pas[sp - 1] / pas[sp];
                        sp = sp - 1;
                        break;
                    //EQL
                    case 5:
                        pas[sp - 1] = pas[sp - 1] == pas[sp];
                        sp = sp - 1;
                        break;
                    //NEQ
                    case 6:
                        pas[sp - 1] = pas[sp - 1] != pas[sp];
                        sp = sp - 1;
                        break;
                    //LSS
                    case 7:
                        pas[sp - 1] = pas[sp - 1] < pas[sp];
                        sp = sp - 1;
                        break;
                    //LEQ
                    case 8:
                        pas[sp - 1] = pas[sp - 1] <= pas[sp];
                        sp = sp - 1;
                        break;
                    //GTR
                    case 9:
                        pas[sp - 1] = pas[sp - 1] > pas[sp];
                        sp = sp - 1;
                        break;
                    //GEQ
                    case 10:
                        pas[sp - 1] = pas[sp - 1] >= pas[sp];
                        sp = sp - 1;
                        break;
                }
                break;
            //LOD INSTRUCTION
            case 3:
                sp++;
                pas[sp] = pas[base(pas, bp, ir.l) + ir.m];
                break;
            //STO INSTRUCTION
            case 4:
                pas[base(pas, bp, ir.l) + ir.m] = pas[sp];
                sp--;
                break;
            //CAL INSTRUCTION
            case 5:
                bars[sp+1] = 1;
                pas[sp+1] = base(pas, bp, ir.l);
                pas[sp+2] = bp;
                pas[sp+3] = pc;
                bp = sp+1;
                pc = ir.m;
                break;
            //INC INSTRUCTION
            case 6:
                sp = sp + ir.m;
                break;
            //JMP INSTRUCTION            
            case 7:
                pc = ir.m;
                break;
            //JPC INSTRUCTION           
            case 8: 
                if(pas[sp] == 0)
                    pc = ir.m;
                sp = sp - 1;
                break;
            //SYSTEM INSTRUCTIONS
            case 9:
                switch(ir.m) {
                    case 1:
                        printf("Output result is: %d\n", pas[sp]);
                        sp = sp - 1;
                        break;

                    case 2: 
                        sp = sp + 1;
                        printf("Please enter an integer: ");
                        scanf("%d", &pas[sp]);
                        break;

                    case 3:
                        halt = 0;
                        break;
                }

        }

        //PRINT THE INSTRUCTIONS AND STACK 
        if(trace_flag == 1) {
            print_instruction(pc, ir);
            print_stack(pc, bp, sp, gp, pas, bars);
        }
        

    }
    
      

  
}



int base(int *pas, int BP, int L) {
    int arb = BP; // arb = activation record base
    while ( L > 0)     //find base L levels down
    {
        arb = pas[arb];
        L--;
    }
    return arb;
}

//FUNCTION TO PRINT THE STACK
void print_stack(int pc, int bp, int sp, int gp, int * pas, int * bars){
    
    //PRINT THE  CURRENT pc, bp, sp
    int i;
    printf("%d\t%d\t%d\t", pc, bp, sp);
    
    //PRINT VALUES AND BARS IN THE STACK 
    for (i = gp; i <= sp; i++) {
        if (bars[i] == 1)
            printf("| %d ", pas[i]);
    else
        printf("%d ", pas[i]);
    }
    printf("\n");
}

//PRINT THE INSTRUCTION
void print_instruction(int PC, instruction IR){
    //VARIABLE FOR INSTRUCTION NAME
    char opname[4];
    
    //DETERMINE AND SAVE INSTRUCTION
    switch (IR.op) {
        case 1 : 
            strcpy(opname, "LIT"); 
            break;

        case 2 :
            switch (IR.m) {
                case 0 : 
                    strcpy(opname, "RTN"); 
                    break;
                case 1 : 
                    strcpy(opname, "ADD"); 
                    break;
                case 2 : 
                    strcpy(opname, "SUB"); 
                    break;
                case 3 : 
                    strcpy(opname, "MUL"); 
                    break;
                case 4 : 
                    strcpy(opname, "DIV"); 
                    break;
                case 5 : 
                    strcpy(opname, "EQL"); 
                    break;
                case 6 : 
                    strcpy(opname, "NEQ"); 
                    break;
                case 7 : 
                    strcpy(opname, "LSS"); 
                    break;
                case 8 : 
                    strcpy(opname, "LEQ"); 
                    break;
                case 9 : 
                    strcpy(opname, "GTR"); 
                    break;
                case 10 : 
                    strcpy(opname, "GEQ"); 
                    break;
                default : 
                    strcpy(opname, "err"); 
                    break;
            }
            break;

        case 3 : 
            strcpy(opname, "LOD"); 
            break;

        case 4 : 
            strcpy(opname, "STO"); 
            break;

        case 5 : 
            strcpy(opname, "CAL"); 
            break;

        case 6 : 
            strcpy(opname, "INC"); 
            break;

        case 7 : 
            strcpy(opname, "JMP"); 
            break;

        case 8 : 
            strcpy(opname, "JPC"); 
            break;

        case 9 :
            switch (IR.m) {
                case 1 : 
                    strcpy(opname, "WRT"); 
                    break;

                case 2 : 
                    strcpy(opname, "RED"); 
                    break;

                case 3 : 
                    strcpy(opname, "HLT"); 
                    break;

                default : 
                    strcpy(opname, "err"); 
                    break;
            }
            break;

        default : strcpy(opname, "err"); break;
    }
    //PRINT INSTRUCTION WITH ir.l and ir.m
    printf("%s\t%d\t%d\t", opname, IR.l, IR.m);
    
}