%token<int> INT
%token<string> IDENT
%token QUOTE LAMBDA LET LPAREN RPAREN EOF
%type <Machine.expression> start
%start start

%%

start: 
| expression EOF { $1 }

expression:
| INT { Machine.EInteger ($1) }
| IDENT { Machine.EVariable ($1) }
| LPAREN QUOTE expression RPAREN { Machine.EQuote ($3) }
| LPAREN LAMBDA LPAREN IDENT RPAREN expression RPAREN { Machine.EAbstraction ($4, $6) }
| LPAREN expression expression RPAREN { Machine.EApplication ($2, $3) }
| LPAREN LET LPAREN IDENT expression RPAREN expression RPAREN { Machine.ELet ($4, $5, $7) }

