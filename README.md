# LanguageParser

> [!IMPORTANT]
> This repository contains the code used for Project 2 (PA2) of my CS280 class

This is a parser for a "Mini C-Like (MCL)" language, as defined below in EBNF notation.
It takes in a file to perform the parsing on and outputs whether it was correctly parsed, any parsing errors, and other information.

# Building

To build this project, download the project and compile `lex.cpp`, `parser.cpp`, and `prog2.cpp` with C++11.

For example, with `gcc`

```bash
g++ -std=c++11 *.cpp -o a
```

## Usage

Assuming the compiled executable is named `a`

```bash
./a <filename>
```

Where only one `filename` can be provided, and optional flags can be provided

### Test Cases

If using the provided test cases, use the command

```bash
./a tests/<test>
```

Where `test` is the filename of the file you wish to test

> [!NOTE]
> You can compare the test cases to those with the same name but have `.correct` appended to it.
> The output of the program and the text within the correct file should be identical.

## Project Outline

`prog2.cpp`

> The driver of the program which handles the file input and calls the parser.
> It calls `Prog()` from the parser to recursively parse through it.

`lex.h`

> The header file that contains information about the tokens the lexer and parser can use and the functions implemented in `lex.cpp`.

`lex.cpp`

> The lexer part of the compiler, `getNextToken()` takes in a stream of characters and returns the next token.
> It also contains helper functions like `operator<<()` to display tokens and `id_or_kws()` to determine if a given lexeme is a keyword or identifier.

`parser.h`

> The header file that contains functions to be implemented in the parser, each pertaining to a specific line in the EBNF grammar.

`parser.cpp`

> The parser part of the compiler, with implemented functions from `parser.h` to perform the recursive descent parsing.

`tests/`

> A folder that contains files used for testing and grading the parser.
> Files are numbered and have comments for the parts they test with a matching file ending with `.correct` that contains the "correct" response of the parser

# Parser Description

A parser, is the second step in the compilation process.

A parser takes in a stream of tokens and determines whether or not it follows the correct format defined by the grammar. If not, it outputs a syntax error.
This is an implementation of a recursive-descent parser, which recursivly determines if the program followed the grammar format.

## Grammar

Here is a list of all the grammar rules defined for this language

### EBNF

1. `Prog ::= PROGRAM IDENT CompStmt`
2. `StmtList ::= Stmt { Stmt }`
3. `Stmt ::= DeclStmt | ControlStmt | CompStmt`
4. `DeclStmt ::= ( INT | FLOAT | BOOL | CHAR | STRING ) VarList ;`
5. `VarList ::= Var [= Expr] { ,Var [= Expr]}`
6. `ControlStmt ::= AssgnStmt ; | IfStmt | PrintStmt ;`
7. `PrintStmt ::= PRINT (ExprList)`
8. `CompStmt ::= ‘{‘ StmtList ‘}’`
9. `IfStmt ::= IF '(' Expr ')' Stmt [ ELSE Stmt ]`
10. `AssgnStmt ::= Var ( = | += | -= | *= | /= | %= ) Expr`
11. `Var ::= IDENT`
12. `ExprList ::= Expr { , Expr }`
13. `Expr ::= LogANDExpr { || LogANDRxpr }`
14. `LogANDExpr ::= EqualExpr { && EqualExpr }`
15. `EqualExpr ::= RelExpr [ (== | != ) RelExpr ]`
16. `RelExpr ::= AddExpr [ ( < | > ) AddExpr ]`
17. `AddExpr :: MultExpr { ( + | - ) MultExpr }`
18. `MultExpr ::= UnaryExpr { ( * | / | % ) UnaryExpr }`
19. `UnaryExpr ::= [( - | + | ! )] PrimaryExpr`
20. `PrimaryExpr ::= IDENT | ICONST | RCONST | SCONST | BCONST | CCONST | '(' Expr ')'`

### Operator Precedence Levels

| Precedence | Operator            | Description                             | Associativity |
| :--------: | ------------------- | --------------------------------------- | :-----------: |
|     1      | Unary `+`, `-`, `!` | Unary plus, minus and logical NOT       | Right-to-Left |
|     2      | `*`, `/`, `%`       | Multiplication, Division, and Remainder | Left-to-Right |
|     3      | `+`, `-`            | Addition and Subtraction                | Left-to-Right |
|     4      | `<`, `>`            | Relational operators `<` and `>`        | No cascading  |
|     5      | `==`, `!=`          | Equality and nonequality operators      | No cascading  |
|     6      | `&&`                | Logical AND                             | Left-to-Right |
|     7      | `\|\|`              | Logical OR                              | Left-to-Right |

### Variables

Variables must be declared before they can be used. They cannot be redeclared. These both result in parsing errors.

### If-Statement Nesting

The parser count and outputs each if-else-statement nesting level as it is parsing.

## Errors

TODO
Maybe also talk about the edge cases that were missed
