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
	LexItem token = getNextToken(in, line);
	// FIX: remove later
	cout << token;
	return token;
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

// GRAMMAR RULES \\

// Prog ::= PROGRAM IDENT CompStmt
bool Prog(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != PROGRAM) return ParseError(line, "Missing Program keyword");

	token = Parser::GetNextToken(in, line);
	if (token != IDENT) return ParseError(line, "Missing Program name");

	bool comp = CompStmt(in, line);
	if (!comp) return ParseError(line, "Invalid Program");

	return true;
}

// StmtList ::= Stmt { Stmt }
bool StmtList(istream &in, int &line) {
	bool stmt = Stmt(in, line);
	if (!stmt) return ParseError(line, "Syntactic error in statement list");

	// Checks for a right brace, meaning we're at the end of the program
	// (This only works because StmtList is only called by CompStmt, which ends with a })
	LexItem token = Parser::GetNextToken(in, line);
	if (token == RBRACE) {
		Parser::PushBackToken(token);
		return true;
	}

	Parser::PushBackToken(token);
	return StmtList(in, line);
}

// Stmt ::= DeclStmt | ControlStmt | CompStmt
bool Stmt(istream &in, int &line) {
	bool declStmt = DeclStmt(in, line);
	if (declStmt) return true;

	// TODO: add more statements

	/*bool controlstmt = ControlStmt(in, line);*/
	/*if (controlstmt) return true;*/
	/**/
	/*bool compstmt = CompStmt(in, line);*/
	/*if (compstmt) return true;*/
	return ParseError(line, "Unknown statement");

	return ParseError(line, "Invalid statement");
}

// DeclStmt ::= ( INT | FLOAT | BOOL | CHAR | STRING ) VarList ;
bool DeclStmt(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != INT && token != FLOAT && token != BOOL && token != CHAR && token != STRING)
		return ParseError(line, "Invalid type");

	bool varlist = VarList(in, line);
	if (!varlist) return ParseError(line, "Invalid variable list");

	token = Parser::GetNextToken(in, line);
	if (token != SEMICOL) return ParseError(line, "Missing semicolon");

	return true;
}

// VarList ::= Var [= Expr] { ,Var [= Expr] }
bool VarList(istream &in, int &line) {
	bool var = Var(in, line);
	if (!var) return ParseError(line, "Invalid variable");

	LexItem token = Parser::GetNextToken(in, line);
	if (token == ASSOP) {
		bool expr = Expr(in, line);
		if (!expr) return ParseError(line, "Invalid expression");
	} else {
		Parser::PushBackToken(token);
	}

	token = Parser::GetNextToken(in, line);
	if (token == COMMA) {
		bool varlist = VarList(in, line);
		if (!varlist) return ParseError(line, "Invalid expression");
	} else {
		Parser::PushBackToken(token);  // Puts the token back if not used
	}

	return true;
}

// ControlStmt ::= AssgnStmt ; | IfStmt | PrintStmt ;
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
	if (!stmtlist) return ParseError(line, "Incorrect statement list");

	token = Parser::GetNextToken(in, line);
	if (token != RBRACE) return ParseError(line, "Missing Right Brace");

	return true;
}
// IfStmt ::= IF (Expr) Stmt [ ELSE Stmt ]
/*bool IfStmt(istream &in, int &line) {}*/

// AssignStmt ::= Var ( = | += | -= | *= | /= | % = ) Expr
/*bool AssignStmt(istream &in, int &line) {}*/

// Var ::= IDENT
bool Var(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	return token == IDENT;
}

// ExprList ::= Expr { , Expr }
bool ExprList(istream &in, int &line) {
	bool status = false;
	status = Expr(in, line);

	if (!status) return ParseError(line, "Missing Expression");

	LexItem token = Parser::GetNextToken(in, line);

	if (token == COMMA) {
		status = ExprList(in, line);
		if (!status) return ParseError(line, "Missing Expression");

	} else if (token == ERR) {
		// FIX: this can probably be removed
		cout << "(" << token.GetLexeme() << ")" << endl;
		return ParseError(line, "Unrecognized Input Pattern");

	} else {
		Parser::PushBackToken(token);  // Puts the comma back
	}

	return true;
}

// Expr ::= LogANDExpr { || LogANDExpr }
bool Expr(istream &in, int &line) {
	bool logAndExpr = LogANDExpr(in, line);
	if (!logAndExpr) return ParseError(line, "Invalid expression");

	LexItem token = Parser::GetNextToken(in, line);
	if (token == OR) {
		logAndExpr = Expr(in, line);
		if (!logAndExpr) return ParseError(line, "Invalid OR expression");
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// LogANDExpr ::= EqualExpr { && EqualExpr }
bool LogANDExpr(istream &in, int &line) {
	bool equalExpr = EqualExpr(in, line);
	if (!equalExpr) return ParseError(line, "Invalid expression");

	LexItem token = Parser::GetNextToken(in, line);
	if (token == AND) {
		equalExpr = LogANDExpr(in, line);
		if (!equalExpr) return ParseError(line, "Invalid AND expression");
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// EqualExpr ::= RelExpr [ ( == | !== ) RealExpr ]
bool EqualExpr(istream &in, int &line) {
	bool relexpr = RelExpr(in, line);
	if (!relexpr) return ParseError(line, "Invalid expression");

	LexItem token = Parser::GetNextToken(in, line);
	if (token == EQ || token == NEQ) {
		relexpr = EqualExpr(in, line);
		if (!relexpr) return ParseError(line, "Invalid equality expression");
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// RelExpr ::= AddExpr [ ( < | > ) AddExpr ]
bool RelExpr(istream &in, int &line) {
	bool addexpr = AddExpr(in, line);
	if (!addexpr) return ParseError(line, "Invalid expression");

	LexItem token = Parser::GetNextToken(in, line);
	if (token == LTHAN || token == GTHAN) {
		addexpr = RelExpr(in, line);
		if (!addexpr) return ParseError(line, "Invalid relative expression");
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// AddExpr ::= MultExpr { ( + | - ) MultExpr }
bool AddExpr(istream &in, int &line) {
	bool multexpr = MultExpr(in, line);
	if (!multexpr) return ParseError(line, "Invalid expression");

	LexItem token = Parser::GetNextToken(in, line);
	if (token == PLUS || token == MINUS) {
		multexpr = AddExpr(in, line);
		if (!multexpr) return ParseError(line, "Invalid addition expression");
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// MultExpr ::= UnaryExpr { ( * | / | % ) UnaryExpr }
bool MultExpr(istream &in, int &line) {
	bool unaryexpr = UnaryExpr(in, line);
	if (!unaryexpr) return ParseError(line, "Invalid expression");

	LexItem token = Parser::GetNextToken(in, line);
	if (token == MULT || token == DIV || token == REM) {
		unaryexpr = MultExpr(in, line);
		if (!unaryexpr) return ParseError(line, "Invalid multiplication expression");
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// UnaryExpr ::= [ ( - | + | ! ) ] PrimaryExpr
bool UnaryExpr(istream &in, int &line) {
	int sign = 1;  // Default to positive

	LexItem token = Parser::GetNextToken(in, line);
	if (token == MINUS || token == NOT)
		sign = -1;
	else if (token == PLUS)
		sign = 1;
	else
		Parser::PushBackToken(token);

	return PrimaryExpr(in, line, sign);
}

// PrimaryExpr ::= IDENT | ICONST | RCONST | SCONST | BCONST | CCONST | ( Expr )
bool PrimaryExpr(istream &in, int &line, int sign) {
	LexItem token = Parser::GetNextToken(in, line);

	if (token == IDENT || token == ICONST || token == RCONST || token == SCONST || token == BCONST || token == CCONST)
		return true;

	token = Parser::GetNextToken(in, line);
	if (token == LPAREN) {
		bool expr = Expr(in, line);
		if (!expr) ParseError(line, "Invalid expression");

		token = Parser::GetNextToken(in, line);
		if (token != RPAREN) return ParseError(line, "Missing right parenthsis");
	} else {
		Parser::PushBackToken(token);
	}

	return ParseError(line, "Invalid primary expression");
}
