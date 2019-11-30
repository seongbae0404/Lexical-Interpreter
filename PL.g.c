#include <stdio.h>
#include <string.h>
#include "PL.y"

extern FILE *yyin;
extern void yyerror(const char *s);
extern int yylex();
extern int yyparse();

typedef struct YYSTYPE
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
} YYSTYPE;

%union
{
	Block* block;
	Expression* expr;
	Statement* stmt;
	Identifier* idt;
	VariableDeclaration* var_dec;
	ArrayIndex* index;
	std::vector<shared_ptr<VariableDeclaration>>* varvec;
	std::vector<shared_ptr<Expression>>* exprvec;
	std::string* string;
	int token;
}

%token <string> TIDENTIFIER TINTEGER TDOUBLE TYINT TYDOUBLE TYFLOAT TYCHAR TYBOOL TYVOID TYSTRING TEXTERN TLITERAL
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT TSEMICOLON TLBRACKET TRBRACKET TQUOTATION
%token <token> TPLUS TMINUS TMUL TDIV TAND TOR TXOR TMOD TNEG TNOT TSHIFTL TSHIFTR
%token <token> TIF TELSE TFOR TWHILE TRETURN TSTRUCT

%type <index> array_index
%type <idt> idt primary_typename array_typename struct_typename typename
%type <expr> numeric expr assign
%type <varvec> func_dec_args struct_members
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_dec func_dec struct_dec if_stmt for_stmt while_stmt
%type <token> comparison

%left TPLUS TMINUS
%left TMUL TDIV TMOD

%start program

%%
program : stmts{ programBlock = $1; }
;
stmts: stmt{ $$ = new Block(); $$->statements->push_back(shared_ptr<Statement>($1)); }
| stmts stmt{ $1->statements->push_back(shared_ptr<Statement>($2)); }
;
stmt: var_dec | func_dec | struct_dec
	| expr{ $$ = new ExpressionStatement(shared_ptr<Expression>($1)); }
	  | TRETURN expr{ $$ = new ReturnStatement(shared_ptr<Expression>($2)); }
		  | if_stmt
		  | for_stmt
		  | while_stmt
		  ;

  block: TLBRACE stmts TRBRACE{ $$ = $2; }
	  | TLBRACE TRBRACE{ $$ = new Block(); }
	  ;

  primary_typename: TYINT{ $$ = new Identifier(*$1); $$->isType = true;  delete $1; }
	  | TYDOUBLE{ $$ = new Identifier(*$1); $$->isType = true; delete $1; }
	  | TYFLOAT{ $$ = new Identifier(*$1); $$->isType = true; delete $1; }
	  | TYCHAR{ $$ = new Identifier(*$1); $$->isType = true; delete $1; }
	  | TYBOOL{ $$ = new Identifier(*$1); $$->isType = true; delete $1; }
	  | TYVOID{ $$ = new Identifier(*$1); $$->isType = true; delete $1; }
	  | TYSTRING{ $$ = new Identifier(*$1); $$->isType = true; delete $1; }

	  array_typename: primary_typename TLBRACKET TINTEGER TRBRACKET{
						  $1->isArray = true;
						  $1->arraySize->push_back(make_shared<Integer>(atol($3->c_str())));
						  $$ = $1;
	  }
		  | array_typename TLBRACKET TINTEGER TRBRACKET{
			  $1->arraySize->push_back(make_shared<Integer>(atol($3->c_str())));
			  $$ = $1;
	  }

	  struct_typename: TSTRUCT idt{
			$2->isType = true;
			$$ = $2;
	  }

	  typename : primary_typename{ $$ = $1; }
	  | array_typename{ $$ = $1; }
	  | struct_typename{ $$ = $1; }

  var_dec: typename idt{ $$ = new VariableDeclaration(shared_ptr<Identifier>($1), shared_ptr<Identifier>($2), nullptr); }
	  | typename idt TEQUAL expr{ $$ = new VariableDeclaration(shared_ptr<Identifier>($1), shared_ptr<Identifier>($2), shared_ptr<Expression>($4)); }
		  | typename idt TEQUAL TLBRACKET call_args TRBRACKET{
			  $$ = new ArrayInitialization(make_shared<VariableDeclaration>(shared_ptr<Identifier>($1), shared_ptr<Identifier>($2), nullptr), shared_ptr<ExpressionList>($5));
	  }
	  ;

  func_dec: typename idt TLPAREN func_dec_args TRPAREN block
  { $$ = new FunctionDeclaration(shared_ptr<Identifier>($1), shared_ptr<Identifier>($2), shared_ptr<VariableList>($4), shared_ptr<Block>($6)); }
			| TEXTERN typename idt TLPAREN func_dec_args TRPAREN{ $$ = new FunctionDeclaration(shared_ptr<Identifier>($2), shared_ptr<Identifier>($3), shared_ptr<VariableList>($5), nullptr, true); }

		func_dec_args: /* blank */ { $$ = new VariableList(); }
			| var_dec{ $$ = new VariableList(); $$->push_back(shared_ptr<VariableDeclaration>($ < var_dec>1)); }
			| func_dec_args TCOMMA var_dec{ $1->push_back(shared_ptr<VariableDeclaration>($ < var_dec>3)); }
			;

		idt: TIDENTIFIER{ $$ = new Identifier(*$1); delete $1; }
			;

		 numeric: TINTEGER{ $$ = new Integer(atol($1->c_str())); }
			 | TDOUBLE{ $$ = new Double(atof($1->c_str())); }
			 ;
			 void yyerror(const char *s)
			 {
				 fprintf(stderr, "%s\n", s);                                                // 에러 메시지 출력..?
			 }