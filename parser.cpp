// =======================================================
// Name        : parser.cpp
// Description : Parses an input of tokens
// =======================================================

#include "parser.h"

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

/*bool Prog(istream &in, int &line) {}*/

/*bool StmtList(istream &in, int &line) {}*/

/*bool Stmt(istream &in, int &line) {}*/

/*bool DeclStmt(istream &in, int &line) {}*/

/*bool VarList(istream &in, int &line) {}*/

/*bool ControlStmt(istream &in, int &line) {}*/

bool PrintStmt(istream &in, int &line) {
	LexItem t = Parser::GetNextToken(in, line);
	if (t != LPAREN) return ParseError(line, "Missing Left Parenthesis");

	bool ex = ExprList(in, line);

	if (!ex) return ParseError(line, "Missing expression list after Print");

	t = Parser::GetNextToken(in, line);
	if (t != RPAREN) return ParseError(line, "Missing Right Parenthesis");

	// Evaluate: print out the list of expressions values

	return true;
}
/*bool CompStmt(istream &in, int &line) {}*/

/*bool IfStmt(istream &in, int &line) {}*/

/*bool AssignStmt(istream &in, int &line) {}*/

/*bool Var(istream &in, int &line) {}*/

bool ExprList(istream &in, int &line) {
	bool status = false;
	/*status = Expr(in, line);*/

	if (!status) return ParseError(line, "Missing Expression");

	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == COMMA) {
		status = ExprList(in, line);

	} else if (tok.GetToken() == ERR) {
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return ParseError(line, "Unrecognized Input Pattern");

	} else {
		Parser::PushBackToken(tok);
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
