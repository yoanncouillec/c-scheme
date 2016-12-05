%{
#include <stdio.h>
#include "machine.h"
%}

ER_INTEGER [0-9]+
ER_IDENT [a-zA-Z][a-zA-Z0-9]*
ER_SEPARATEUR [ \t\n]

%%

"(" { return (TOKEN_LPAREN) ; }
")" { return (TOKEN_RPAREN) ; }
"quote" { return (TOKEN_QUOTE) ; }
"lambda" { return (TOKEN_LAMBDA) ; }
"let" { return (TOKEN_LET) ; }
"letx" { return (TOKEN_LETX) ; }

{ER_SEPARATEUR}+ {}
{ER_INTEGER} { yylval.integer = atoi (yytext) ; return (TOKEN_INTEGER) ; }
{ER_IDENT} { yylval.ident = strdup (yytext) ; return (TOKEN_IDENT) ; }

%%
