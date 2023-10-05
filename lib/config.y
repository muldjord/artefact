%{
#include <config.h>
#include <stdio.h>
#include <string.h>

  using namespace Pentominos;

  int yylex (void);
  void yyerror (char const *);
	
#include <vector>

	std::vector< Configs > stack;

%}

%union {
  int integer;
  bool boolean;
  double number;
  char *string;
  char *ip;
  char *var;
  //  void *listtype;
  void *variant;
	//	void *configs;
}

%defines /* Create header file */

 /* %parse-param {Config *config} */
 //%pure-parser

/* Bison declarations.  */
%token <var> VAR
%token <boolean> BOOLEAN
%token <integer> INTEGER
%token <number> NUMBER
%token <string> STRING
%token <ip> IP

%type <variant> list
%type <variant> integerlist
%type <variant> stringlist
%type <variant> numberlist
%type <variant> booleanlist
%type <variant> iplist

 //%type <configs> exps

%verbose

%start exps

%% /* The grammar follows.  */

exps: exp      { }
    | exps exp { } 
    ;

exp: VAR assign list  {
  VariantList *v = (VariantList*)$3;
  VariantList::reverse_iterator i = v->rbegin();
  while(i != v->rend()) {
    stack.back()[$1].push_back(*i);
    i++;
  }
  delete v;
 }
| VAR assign blkst exps '}' {
	Configs c = stack.back();
	stack.pop_back();
	Variant v(c);
	stack.back()[$1].push_back(v);
}
;

blkst: '{' { Configs c; stack.push_back(c); }
     ;

assign: /*nothing*/
      | '='
      ;

list: integerlist { $$ = $1; }
    | stringlist  { $$ = $1; }
    | numberlist  { $$ = $1; }
    | booleanlist { $$ = $1; }
    | iplist      { $$ = $1; }
    ;

integerlist: INTEGER                 { VariantList *v = new VariantList(); v->push_back(Variant($1)); $$ = v; }
           | INTEGER ',' integerlist { VariantList *v = (VariantList*)$3; v->push_back(Variant($1)); $$ = v; }
           ;

stringlist: STRING                { VariantList *v = new VariantList(); v->push_back(Variant(std::string($1).substr(1, strlen($1)-2))); $$ = v; }
          | STRING ',' stringlist { VariantList *v = (VariantList*)$3; v->push_back(Variant(std::string($1).substr(1, strlen($1)-2))); $$ = v; }
          ;

numberlist: NUMBER                { VariantList *v = new VariantList(); v->push_back(Variant($1)); $$ = v; }
          | NUMBER ',' numberlist { VariantList *v = (VariantList*)$3; v->push_back(Variant($1)); $$ = v; }
          ;

booleanlist: BOOLEAN                 { VariantList *v = new VariantList(); v->push_back(Variant($1)); $$ = v; }
           | BOOLEAN ',' booleanlist { VariantList *v = (VariantList*)$3; v->push_back(Variant($1)); $$ = v; }
           ;

iplist: IP            { VariantList *v = new VariantList(); v->push_back(Variant(std::string($1))); $$ = v; }
      | IP ',' iplist { VariantList *v = (VariantList*)$3; v->push_back(Variant(std::string($1))); $$ = v; }
      ;

%%
extern std::string config_filename;
void yyerror(char const*) {}
void reset_parser();

void Pentominos::parse_config(std::string filename)
{
  config_filename = filename;
  stack.clear();
  stack.push_back(Pentominos::config);
  yyparse();
  Pentominos::config = stack.front();
  reset_parser();
}
