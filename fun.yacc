%{
#include <stdio.h>
#include "machine.h"

struct TERM * term;
int yylex();
int yyerror (char *s);
%}

%union{
  char * ident;
  int integer;
  struct TERM * term;
}

%token <integer> TOKEN_INTEGER
%token <ident> TOKEN_IDENT
%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_QUOTE TOKEN_LAMBDA TOKEN_LET TOKEN_LETX

%type <term> TERM QUOTE ABSTRACTION APPLICATION LET LETX

%start START

%%

START : TERM { term = $1 ; }

TERM :
  TOKEN_INTEGER { $$ = make_term_integer ($1) ; }
| TOKEN_IDENT { $$ = make_term_variable ($1) ; }
| QUOTE { $$ = $1 ; }
| ABSTRACTION { $$ = $1 ; }
| APPLICATION { $$ = $1 ; }
| LET { $$ = $1 ; }
| LETX { $$ = $1 ; }
;

QUOTE : TOKEN_LPAREN TOKEN_QUOTE TERM TOKEN_RPAREN
{ $$ = make_term_quote ($3) ; }
;

ABSTRACTION : TOKEN_LPAREN TOKEN_LAMBDA TOKEN_LPAREN TOKEN_IDENT TOKEN_RPAREN TERM TOKEN_RPAREN 
{ $$ = make_term_abstraction ($4, $6) ; }
;

APPLICATION : TOKEN_LPAREN TERM TERM TOKEN_RPAREN { $$ = make_term_application ($2, $3) ; }
;

//(let (x t1) t2) = ((lambda (x) t2) t1)
LET : TOKEN_LPAREN TOKEN_LET TOKEN_LPAREN TOKEN_IDENT TERM TOKEN_RPAREN TERM TOKEN_RPAREN 
      //{ $$ = make_term_application (make_term_abstraction ($4, $7), $5) ; }
{ $$ = make_term_let ($4, $5, $7) ; }
;

LETX : TOKEN_LPAREN TOKEN_LETX TOKEN_LPAREN TOKEN_IDENT TERM TOKEN_RPAREN TERM TOKEN_RPAREN 
{ $$ = make_term_application (make_term_abstraction ($4, $7), $5) ; }
%%

#include "lex.yy.c"
#include <stdio.h>

int main (int argc, char *argv[]) {

  /* OPTIONS */

  FILE * input_file = stdin;
  FILE * output_file = stdout;
  debug_file = fopen("debug.log","w");
  char eval = 0;
  char silent = 0;

  /* --eval, --compile, --bytecode-compile, --bytecode-eval */

  int i = 1;
  while (i < argc) {
    if (strcmp (argv[i], "--input") == 0 || strcmp (argv[i], "-i") == 0) {
      input_file = fopen(argv[i+1], "r");
      i++;
    }
    else if (strcmp (argv[i], "--output") == 0 || strcmp (argv[i], "-o") == 0) {
      output_file = fopen(argv[i+1], "w");
      i++;
    }
    else if (strcmp (argv[i], "--debug") == 0 || strcmp (argv[i], "-d") == 0) {
      debug_file = fopen(argv[i+1], "r");
      i++;
    }
    else if (strcmp (argv[i], "--silent") == 0 || strcmp (argv[i], "-s") == 0) {
      silent = 1;
    }
    else if (strcmp (argv[i], "--eval") == 0 || strcmp (argv[i], "-e") == 0) {
      eval = 1;
    }
    else {
      fprintf (stderr, "Unknown option %s\n", argv[i]);
      exit(1);
    }
    i++;
  }

  /* PARSE INPUT */
  yyparse();

  struct ENV * env = make_env();

  if (silent == 0) {
    fprintf(output_file, ">> Fun 1.0 <<\n");
    fprint_term (output_file, term, env);
    fprintf(output_file, "\n=> ");
  }
  struct TERM * t = eval_term (term, env);
  //fprint_debruijn(output_file, v_debruijn);
  //fprintf (output_file, "\n");
  return 0;
}

int yyerror (char *s) {
  printf ("Syntactic error\n");
  exit (0);
}

struct TERM * eval_term (struct TERM * term, struct ENV * env) {
  TRACE("eval_term");
  struct STACK * stack = make_stack();

  /* POPULATE ENVIRONMENT */

  /* TRUE */
  TRACE("true")
  struct TERM * true_term = 
    make_term_abstraction("y", 
      make_term_variable("x"));
  env = set_env (env, "true", make_value_closure ("x", true_term, env));
  stack = set_stack(stack,"x",NULL);
  struct DEBRUIJN * true_debruijn = term_to_debruijn (true_term, stack);
  struct DEBRUIJN * x = make_debruijn_closure (true_debruijn, stack);
  stack = set_stack (stack, "true", x);

  /* FALSE */
  TRACE("false")
  struct TERM * false_term = 
      make_term_abstraction("y", 
	make_term_variable("y"));
  env = set_env (env, "false", make_value_closure ("x", false_term, env));
  stack = set_stack(stack,"x",NULL);
  struct DEBRUIJN * false_debruijn = term_to_debruijn (false_term, stack);
  struct DEBRUIJN * y = make_debruijn_closure (false_debruijn, stack);
  stack = set_stack (stack, "false", y);

  /* CONS */
  TRACE("cons")
  struct TERM * cons_term = 
      make_term_abstraction("y",
        make_term_abstraction("f",
	  make_term_application(
	    make_term_application(make_term_variable("f"),
				  make_term_variable("x")),
	    make_term_variable("y"))));
  env = set_env (env, "cons", make_value_closure ("x", cons_term, env));
  stack = set_stack(stack,"x",NULL);
  struct DEBRUIJN * cons_debruijn = term_to_debruijn (cons_term, stack);
  stack = set_stack (stack, "cons", make_debruijn_closure (cons_debruijn, stack));
           
  /* FIRST */
  TRACE("first")
  env = set_env (env, "first", get_env (env, "true"));
  stack = set_stack (stack, "first", 
		     get_stack (stack, get_stack_position(stack, "true"))->debruijn);

  /* FST */
  TRACE("fst")
  struct TERM * fst_term = 
      make_term_application(make_term_variable("x"),
	make_term_variable("first"));
  env = set_env (env, "fst", make_value_closure ("x",fst_term, env));
  stack = set_stack(stack,"x",NULL);
  struct DEBRUIJN * fst_debruijn = term_to_debruijn (fst_term, stack);
  stack = set_stack (stack, "fst", make_debruijn_closure (fst_debruijn, stack));
  
  /* SECOND */
  TRACE("second")
  env = set_env (env, "second", get_env (env, "false"));
  stack = set_stack (stack, "second", 
		     get_stack (stack, get_stack_position(stack, "false"))->debruijn);

  /* SND */
  TRACE("snd")
  struct TERM * snd_term = 
      make_term_application(make_term_variable("x"),
	make_term_variable("second"));
  env = set_env (env, "snd", make_value_closure ("x", snd_term, env));
  stack = set_stack(stack,"x",NULL);
  struct DEBRUIJN * snd_debruijn = term_to_debruijn (snd_term, stack);
  stack = set_stack (stack, "snd", make_debruijn_closure (snd_debruijn, stack));
  
  /* BIND */
  TRACE("bind")
  struct TERM * bind_term =
    make_term_abstraction ("f",
      make_term_abstraction("g",
        make_term_abstraction("world",
	  make_term_let("r",
	    make_term_application(make_term_variable("f"),
				  make_term_variable("world")),
	    make_term_application(
	      make_term_application(make_term_variable("g"),
				    make_term_variable("v")),
	      make_term_application(make_term_variable("snd"),
				    make_term_variable("r")))))));

  /* TRANSFORM TERM TO DEBRUIJN TERM */
  fprint_term(stdout, term,env);
  struct DEBRUIJN * d = term_to_debruijn (term, stack);
  
  /* EVAL DEBRUIJN TERM */
  struct DEBRUIJN * v_debruijn = eval_debruijn (d, stack);
  struct TERM * t = debruijn_to_term (v_debruijn, stack);
  return t;
}
