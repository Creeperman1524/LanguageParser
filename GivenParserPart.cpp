/* Implementation of Recursive-Descent Parser
 * for Mini C-Like Language
 * Programming Assignment 2
 * Fall 2024
 */

#include "parser.h"

map<string, bool> defVar;
map<string, Token> SymTable;

namespace Parser {
bool pushed_back = false;
LexItem pushed_token;

static LexItem GetNextToken(istream &in, int &line) {
	if (pushed_back) {
		pushed_back = false;
		return pushed_token;
	}
	return getNextToken(in, line);
}

static void PushBackToken(LexItem &t) {
	if (pushed_back) { abort(); }
	pushed_back = true;
	pushed_token = t;
}

}  // namespace Parser

static int error_count = 0;

int ErrCount() { return error_count; }

void ParseError(int line, string msg) {
	++error_count;
	cout << line << ": " << msg << endl;
}

bool IdentList(istream &in, int &line);

// PrintStmt:= PRINT (ExpreList)
bool PrintStmt(istream &in, int &line) {
	LexItem t;
	// cout << "in PrintStmt" << endl;

	t = Parser::GetNextToken(in, line);
	if (t != LPAREN) {
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}

	bool ex = ExprList(in, line);

	if (!ex) {
		ParseError(line, "Missing expression list after Print");
		return false;
	}

	t = Parser::GetNextToken(in, line);
	if (t != RPAREN) {
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	// Evaluate: print out the list of expressions values

	return true;
}  // End of PrintStmt

// ExprList:= Expr {,Expr}
bool ExprList(istream &in, int &line) {
	bool status = false;
	// cout << "in ExprList and before calling Expr" << endl;
	status = Expr(in, line);
	if (!status) {
		ParseError(line, "Missing Expression");
		return false;
	}

	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == COMMA) {
		// cout << "before calling ExprList" << endl;
		status = ExprList(in, line);
		// cout << "after calling ExprList" << endl;
	} else if (tok.GetToken() == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	} else {
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}  // End of ExprList
