// =======================================================
// Name        : parser.cpp
// Description : Parses an input of tokens
// =======================================================

#include "parser.h"
#include "lex.h"

map<string, bool> defVar;
map<string, Token> SymTable;

namespace Parser {

bool pushed_back = false;
LexItem pushed_token;

// A wrapper for the getNextToken function, used to also pushback tokens
static LexItem GetNextToken(istream &in, int &line) {
	if (pushed_back) {
		pushed_back = false;
		return pushed_token;
	}
	// TODO: maybe deal with errors in here?
	// (I don't think they're checked for)
	return getNextToken(in, line);
}

// Pushes back a token to be used later
static void PushBackToken(LexItem &t) {
	if (pushed_back) { abort(); }
	pushed_back = true;
	pushed_token = t;
}

}  // namespace Parser

static int error_count = 0;
int ErrCount() { return error_count; }

bool ParseError(int line, string msg) {
	++error_count;
	cout << line << ": " << msg << endl;
	return false;
}

// Prog ::= PROGRAM IDENT CompStmt
bool Prog(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != PROGRAM) return ParseError(line, "Missing Program keyword");

	token = Parser::GetNextToken(in, line);
	if (token != IDENT) return ParseError(line, "Missing Program name");

	bool comp = CompStmt(in, line);
	if (!comp) return ParseError(line, "Cannot have an empty program");

	return true;
}

// StmtList ::= Stmt { Stmt }
bool StmtList(istream &in, int &line) {
	bool stmt = Stmt(in, line);
	if (!stmt) return ParseError(line, "Invalid statement");

	// Checks for a right brace, meaning we're at the end of the program
	// (This only works because StmtList is only called by CompStmt, which ends with a })
	LexItem token = Parser::GetNextToken(in, line);
	if (token == RBRACE) {
		Parser::PushBackToken(token);
		return true;
	}

	return StmtList(in, line);
}

// Stmt ::= DeclStmt | ControlStmt | CompStmt
bool Stmt(istream &in, int &line) {
	// TODO: fix
	return true;
}

/*bool DeclStmt(istream &in, int &line) {}*/

/*bool VarList(istream &in, int &line) {}*/

/*bool ControlStmt(istream &in, int &line) {}*/

// PrintStmt ::= PRINT (ExprList)
bool PrintStmt(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != LPAREN) return ParseError(line, "Missing Left Parenthesis");

	bool ex = ExprList(in, line);

	if (!ex) return ParseError(line, "Missing expression list after Print");

	token = Parser::GetNextToken(in, line);
	if (token != RPAREN) return ParseError(line, "Missing Right Parenthesis");

	return true;
}

// CompStmt ::= '{' StmtList '}'
bool CompStmt(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != LBRACE) return ParseError(line, "Missing Left Brace");

	bool stmtlist = StmtList(in, line);

	if (!stmtlist) return ParseError(line, "Missing statements in program");

	token = Parser::GetNextToken(in, line);
	if (token != RBRACE) return ParseError(line, "Missing Right Brace");

	return true;
}

/*bool IfStmt(istream &in, int &line) {}*/

/*bool AssignStmt(istream &in, int &line) {}*/

/*bool Var(istream &in, int &line) {}*/

// ExprList ::= Expr { , Expr }
bool ExprList(istream &in, int &line) {
	bool status = false;
	/*status = Expr(in, line);*/

	if (!status) return ParseError(line, "Missing Expression");

	LexItem token = Parser::GetNextToken(in, line);

	if (token == COMMA) {
		status = ExprList(in, line);

	} else if (token == ERR) {
		// FIX: this can probably be removed
		cout << "(" << token.GetLexeme() << ")" << endl;
		return ParseError(line, "Unrecognized Input Pattern");

	} else {
		Parser::PushBackToken(token);
		return true;
	}
	return true;
}

/*bool Expr(istream &in, int &line) {}*/

/*bool LogANDExpr(istream &in, int &line) {}*/

/*bool EqualExpr(istream &in, int &line) {}*/

/*bool RelExpr(istream &in, int &line) {}*/

/*bool AddExpr(istream &in, int &line) {}*/

/*bool MultExpr(istream &in, int &line) {}*/

/*bool UnaryExpr(istream &in, int &line) {}*/

/*bool PrimaryExpr(istream &in, int &line, int sign) {}*/
