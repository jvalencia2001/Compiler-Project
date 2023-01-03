# Compiler-Project

### _This project was created for my system software class at UCF for Fall 2022. The compiler as a whole is designed for a small, but turing-complete, langauge called mini PL/0. It is fully coded in c and it includes 3 main files, which are the following:_

- ### _lex.c - this file contains a lexographical analyzer which scans the program that the compiler is trying to interpret and translates instructions into token representations that the parser is able to understand. Any lexographical erros will be cought by lex.c._

- ### _parser.c - if there were no lexographical errors in the program, the token representation of the program is then passed here. In this part of the process the token list is parsed into machine code for our VM and a symbol table is created. Any other erros will be cought here._

- ### _vm.c - if there were no mistakes of any kind in the program then the machine code for the program that was produced in parser.c will be passed here. vm.c will run this code and give us the final output for our program._

### \_The file driver.c should take care of running this files all together. To run the compiler make sure that you have all the files (lex.c, parser.c, vm.c, driver.c, compiler.h, input.txt) in the same directory, then follow the following steps:

- ### _Using your terminal in a computer that can run c, use:_

  ```
  gcc driver.c lex.c parser.c vm.c
  ```

- ### _and then:_

  ```
  /a.out input.txt -v -s -l -c
  ```
