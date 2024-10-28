// =======================================================
// Name        : parser.cpp
// Description : Parses an input of tokens
// =======================================================

#include "parser.h"
#include "lex.h"

map<string, bool> defVar;  // A map of variables, where true means it is assigned

// There is probably a better way to do this
// A variable to keep track of whether we are assigning or not
bool assigning = false;

// A variable to keep track of the current variable being parsed
// (Used for the "initialization" print statements)
// FIX: there is prob a better way to do this
string currVar = "";

int nestingLevel = 0;

/*map<string, Token> SymTable;*/

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
	if (token != PROGRAM) return ParseError(line, "Missing PROGRAM keyword");

	token = Parser::GetNextToken(in, line);
	if (token != IDENT) return ParseError(line, "Missing Program name");

	bool comp = CompStmt(in, line);
	if (!comp) return ParseError(line, "Invalid Program");

	token = Parser::GetNextToken(in, line);
	if (token != DONE) return ParseError(line, "Something went horribly wrong");
	cout << "(DONE)" << endl;

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
	LexItem token = Parser::GetNextToken(in, line);

	// Detects a declaration statement
	if (token == INT || token == FLOAT || token == BOOL || token == CHAR || token == STRING) {
		Parser::PushBackToken(token);
		return DeclStmt(in, line);
	} else if (token == LBRACE) {
		// Detects a compound statement
		Parser::PushBackToken(token);
		return CompStmt(in, line);
	} else {
		Parser::PushBackToken(token);
	}

	// FIX: hacky way for testprog16
	token = Parser::GetNextToken(in, line);
	if (token == ELSE) return false;
	Parser::PushBackToken(token);

	// Defaults to a control statement
	bool controlstmt = ControlStmt(in, line);
	return controlstmt ? true : ParseError(line, "Invalid control statement");
}

// DeclStmt ::= ( INT | FLOAT | BOOL | CHAR | STRING ) VarList ;
bool DeclStmt(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != INT && token != FLOAT && token != BOOL && token != CHAR && token != STRING)
		return ParseError(line, "Invalid declaration type");

	bool varlist = VarList(in, line);
	if (!varlist) return ParseError(line, "Invalid variable list");

	token = Parser::GetNextToken(in, line);
	if (token != SEMICOL) {
		line--;
		return ParseError(line, "Missing ; after declaration statement");
	}

	return true;
}

// VarList ::= Var [= Expr] { ,Var [= Expr] }
bool VarList(istream &in, int &line) {
	assigning = false;
	bool var = Var(in, line);
	if (!var) return ParseError(line, "Invalid variable");

	LexItem token = Parser::GetNextToken(in, line);
	if (token == ASSOP) {
		bool expr = Expr(in, line);
		if (!expr) return false;
		cout << "Initialization of the variable " << currVar << " in the declaration statement at line " << line
			 << endl;

	} else {
		Parser::PushBackToken(token);
	}

	token = Parser::GetNextToken(in, line);
	if (token == COMMA) {
		bool varlist = VarList(in, line);
		if (!varlist) return false;
	} else {
		Parser::PushBackToken(token);  // Puts the token back if not used
	}

	return true;
}

// ControlStmt ::= AssgnStmt ; | IfStmt | PrintStmt
bool ControlStmt(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);

	// Detects an if statement
	if (token == IF) {
		Parser::PushBackToken(token);
		bool ifstmt = IfStmt(in, line);
		return ifstmt ? true : ParseError(line, "Invalid IF statement");
	} else if (token == PRINT) {
		// Detects a print statement
		Parser::PushBackToken(token);
		bool printstmt = PrintStmt(in, line);
		return printstmt ? true : ParseError(line, "Invalid PRINT statement");
	} else {
		Parser::PushBackToken(token);
	}

	// Defaults to an assignment statement
	bool assgnstmt = AssignStmt(in, line);
	if (!assgnstmt) return ParseError(line, "Invalid Assignment Statement");

	token = Parser::GetNextToken(in, line);
	if (token != SEMICOL) {
		line--;
		return ParseError(line, "Missing ; after assignment statement");
	}

	return true;
}

// PrintStmt ::= PRINT (ExprList) ;
bool PrintStmt(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != PRINT) return ParseError(line, "Missing PRINT keyword");  // This should never be hit

	token = Parser::GetNextToken(in, line);
	if (token != LPAREN) return ParseError(line, "Missing (");

	bool ex = ExprList(in, line);
	if (!ex) return ParseError(line, "Missing expression list for PRINT");

	token = Parser::GetNextToken(in, line);
	if (token != RPAREN) return ParseError(line, "Missing )");

	token = Parser::GetNextToken(in, line);
	if (token != SEMICOL) {
		line--;
		return ParseError(line, "Missing ; after print statement");
	}

	return true;
}

// CompStmt ::= '{' StmtList '}'
bool CompStmt(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != LBRACE) return false;

	bool stmtlist = StmtList(in, line);
	if (!stmtlist) return ParseError(line, "Incorrect statement list");

	token = Parser::GetNextToken(in, line);
	if (token != RBRACE) return ParseError(line, "Missing }");

	return true;
}
// IfStmt ::= IF (Expr) Stmt [ ELSE Stmt ]
bool IfStmt(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != IF) return ParseError(line, "Missing IF keyword");	 // This should never be hit

	token = Parser::GetNextToken(in, line);
	if (token != LPAREN) return ParseError(line, "Missing ( in if statement");

	bool expr = Expr(in, line);
	if (!expr) return ParseError(line, "Invalid expression in if statement");

	token = Parser::GetNextToken(in, line);
	if (token != RPAREN) return ParseError(line, "Missing ) in if statement");

	nestingLevel++;
	cout << "in IfStmt then-clause at nesting level: " << nestingLevel << endl;
	bool stmt = Stmt(in, line);
	if (!stmt) return ParseError(line, "Invalid if-clause in if statement");
	nestingLevel--;

	// Else
	token = Parser::GetNextToken(in, line);
	if (token == ELSE) {
		nestingLevel++;
		cout << "in IfStmt else-clause at nesting level: " << nestingLevel << endl;
		stmt = Stmt(in, line);
		nestingLevel--;
		if (!stmt) return ParseError(line, "Invalid else-clause in if statement");
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// AssignStmt ::= Var ( = | += | -= | *= | /= | %= ) Expr
bool AssignStmt(istream &in, int &line) {
	assigning = true;
	bool var = Var(in, line);
	if (!var) return ParseError(line, "Invalid LHS variable in assignment");

	LexItem token = Parser::GetNextToken(in, line);
	if (token != ASSOP && token != ADDASSOP && token != SUBASSOP && token != MULASSOP && token != DIVASSOP &&
		token != REMASSOP)
		return ParseError(line, "Missing assignment operator");

	bool expr = Expr(in, line);
	if (!expr) return ParseError(line, "Invalid expression in assignment");

	return true;
}

// Var ::= IDENT
bool Var(istream &in, int &line) {
	LexItem token = Parser::GetNextToken(in, line);
	if (token != IDENT) return false;

	// Handles the creation of variables
	// TODO: maybe there's a better way to do this
	currVar = token.GetLexeme();
	if (defVar.find(currVar) != defVar.end()) {
		// The variable exists
		if (!assigning) {
			// Defining an already existing variable
			return ParseError(line, "Variable redefinition");
		}
	} else {
		// The variable does not exist
		if (assigning) {
			// Assining to a nonexisting variable
			return ParseError(line, "Undefined variable");
		} else {
			// Defining a nonexisting varialble
			defVar[currVar] = true;
		}
	}

	return true;
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
	if (!logAndExpr) return false;

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
	if (!equalExpr) return false;

	LexItem token = Parser::GetNextToken(in, line);
	if (token == AND) {
		equalExpr = LogANDExpr(in, line);
		if (!equalExpr) return ParseError(line, "Invalid AND expression");
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// EqualExpr ::= RelExpr [ ( == | != ) RelExpr ]
bool EqualExpr(istream &in, int &line) {
	bool relexpr = RelExpr(in, line);
	if (!relexpr) return false;

	LexItem token = Parser::GetNextToken(in, line);
	if (token == EQ || token == NEQ) {
		relexpr = RelExpr(in, line);
		if (!relexpr) return ParseError(line, "Invalid equality expression");

		token = Parser::GetNextToken(in, line);
		if (token == EQ || token == NEQ) { return ParseError(line, "Cannot have chained equality expression"); }

		Parser::PushBackToken(token);
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// RelExpr ::= AddExpr [ ( < | > ) AddExpr ]
bool RelExpr(istream &in, int &line) {
	bool addexpr = AddExpr(in, line);
	if (!addexpr) return false;

	LexItem token = Parser::GetNextToken(in, line);
	if (token == LTHAN || token == GTHAN) {
		addexpr = AddExpr(in, line);
		if (!addexpr) return ParseError(line, "Invalid relative expression");

		token = Parser::GetNextToken(in, line);
		if (token == LTHAN || token == GTHAN) { return ParseError(line, "Cannot have chained relative expression"); }

		Parser::PushBackToken(token);
	} else {
		Parser::PushBackToken(token);
	}

	return true;
}

// AddExpr ::= MultExpr { ( + | - ) MultExpr }
bool AddExpr(istream &in, int &line) {
	bool multexpr = MultExpr(in, line);
	if (!multexpr) return false;

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
	if (!unaryexpr) return false;

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

	if (token == LPAREN) {
		bool expr = Expr(in, line);
		if (!expr) return ParseError(line, "Missing expression after (");

		token = Parser::GetNextToken(in, line);
		if (token != RPAREN) return ParseError(line, "Missing ) after expression");

		return true;
	}

	return false;
}
