%{
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>
%}

%option noyywrap

%%
"maquina"       return MAQUINA;
"subrutina"     return SUBRUTINA;
"usa"           return USA;
"alfabeto"      return ALFABETO;
"estados"       return ESTADOS;
"inicial"       return INICIAL;
"finales"       return FINALES;
"transiciones"  return TRANSICIONES;
"entrada"       return ENTRADA;
"DER"           return DER;
"IZQ"           return IZQ;
"QUIETO"        return QUIETO;

\"[^\"]*\"      { yylval.str = strndup(yytext+1, yyleng-2); return CADENA; }

[0-9]+          { yylval.num = atoi(yytext); return NUMERO; }

[_#+]           { yylval.ch = yytext[0]; return SIMBOLO; }

[a-zA-Z][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return ID; }

[ \t\r\n]+      ;
.               return yytext[0];

%%
