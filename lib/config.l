%{
#include <stdio.h>
#include "y.tab.h"

#include <string>

#define ECHO if(fwrite( yytext, yyleng, 1, yyout )) {}

#define YY_INPUT(buf, result, max_size) result = yy_input_proc(buf, max_size)

  static FILE *fp = NULL;
  std::string config_filename;

  int yy_input_proc(char *buf, int size)
  {
    if(!fp) fp = fopen(config_filename.c_str(), "r");
    if(fp) return fread(buf, 1, size, fp);
    return 0;
  }

  void reset_parser()
  {
    fclose(fp);
    fp = NULL;
  }
%}

VAR        [_a-zA-z]+[_a-zA-Z0-9]*
INTEGER    -{0,1}[0-9]|-{0,1}([1-9][0-9]+)
IP         {INTEGER}\.{INTEGER}\.{INTEGER}\.{INTEGER}
NUMBER     {INTEGER}\.[0-9]+
STRING     \"([^\"]|[.]|\\\")*\"
BOOLEAN_T  [Yy]es|[Tt]rue
BOOLEAN_F  [Nn]o|[Ff]alse
COMMENT    (#[^\n]*\n)|(;[^\n]*\n)
WS         [\n \t]+
ERROR      .

%option yylineno
										/*%option reentrant*/

%%
{BOOLEAN_T}  yylval.boolean = true; return BOOLEAN;
{BOOLEAN_F}  yylval.boolean = false; return BOOLEAN;
{STRING}     yylval.string = strdup(yytext); return STRING;
{INTEGER}    yylval.integer = atoi(yytext); return INTEGER;
{NUMBER}     yylval.number = atof(yytext); return NUMBER;
{VAR}        yylval.var = strdup(yytext); return VAR;
{IP}         yylval.ip = strdup(yytext); return IP;
","          return ',';
"="          return '=';
"{"          return '{';
"}"          return '}';
";"          return ';';
{WS}         /* Ignore whitespace */
{COMMENT}    /* Ignore comments */
{ERROR}      printf("ERROR at line %d: Unexpected character %s\n", yylineno, yytext); unput(0);

%%
