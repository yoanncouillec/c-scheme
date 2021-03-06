#include "machine.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* */

char * gen_ident () {
  static int n = 0;
  char * ret = malloc(sizeof(char)*32);
  sprintf (ret, "x%d",n);
  n++;
  return ret;
}

/* ------------------------------------ ENV --------------------------------- */

struct ENV * make_env () {
  return NULL;
}

struct TERM * get_env (struct ENV * env, char * ident) {
  if (env == NULL) {
    fprintf (stderr, "get_env : no such binding %s\n", ident);
    return NULL;
  }
  else {
    if (strcmp (ident, env->ident) == 0) {
      return env->term;
    }
    else {
      return get_env (env->next, ident);
    }
  }
}

struct ENV * set_env (struct ENV * env, char * ident, struct TERM * term) {
  struct ENV * ret = malloc (sizeof (struct ENV));
  ret->ident = ident;
  ret->term = term;
  ret->next = env;
  return ret;
}

/* ----------------------------------- TERM --------------------------------- */

struct TERM * make_term_integer (int value) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_INTEGER;
  ret->integer.value = value;
  return ret;
}

struct TERM * make_term_variable (char * value) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_VARIABLE;
  ret->variable.value = value;
  return ret;
}

struct TERM * make_term_quote (struct TERM * term) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_QUOTE;
  ret->quote.content = term;
  return ret;
}

struct TERM * make_term_abstraction (char * variable, struct TERM * body) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_ABSTRACTION;
  ret->abstraction.variable = variable;
  ret->abstraction.body = body;
  return ret;
}

struct TERM * make_term_closure (char * variable, struct TERM * body, struct ENV * env) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_CLOSURE;
  ret->closure.variable = variable;
  ret->closure.body = body;
  ret->closure.env = env;
  return ret;
}

struct TERM * make_term_application (struct TERM * left, struct TERM * right) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_APPLICATION;
  ret->application.left = left;
  ret->application.right = right;
  return ret;
}

struct TERM * make_term_let (char * variable, struct TERM * init, struct TERM * body) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_LET;
  ret->let.variable = variable;
  ret->let.init = init;
  ret->let.body = body;
  return ret;
}

/* ----------------------------------- EVAL --------------------------------- */

struct TERM * evaluate_term (struct TERM * term, struct ENV * env) {
  switch (term->type) {
  case TYPE_TERM_INTEGER : {
    return term ;
  }
  case TYPE_TERM_VARIABLE : {
    return get_env (env, term->variable.value) ; 
  }
  case TYPE_TERM_QUOTE : {
    return term->quote.content ; 
  }
  case TYPE_TERM_ABSTRACTION : {
    return make_term_closure (term->abstraction.variable, term->abstraction.body, env); 
  }
  case TYPE_TERM_CLOSURE : {
    return term;
  }
  case TYPE_TERM_APPLICATION : {
    struct TERM * value1 = evaluate_term (term->application.left, env);
    switch (value1->type) {
    case TYPE_TERM_CLOSURE : {
      return evaluate_term (value1->closure.body,
			    set_env (value1->closure.env,
				     value1->closure.variable, 
				     evaluate_term (term->application.right, 
						    env)));
    }
    default : {
      fprintf (stderr, "Not a closure\n");
      return NULL;
    }}}
  case TYPE_TERM_LET : {
    struct TERM * value1 = evaluate_term (term->let.init, env);
    return evaluate_term (term->let.body, set_env (env, term->let.variable, value1));
  }}
}

void fprint_term (FILE * out, struct TERM * tree) {
  TRACE("fprint_term\n");
  switch (tree->type) {
    case TYPE_TERM_INTEGER :
      TRACE("integer");
      fprintf(out,"%d", tree->integer.value);
      break;
    case TYPE_TERM_VARIABLE : 
      TRACE("variable");
      fprintf (out,"%s", tree->variable.value);
      break;
    case TYPE_TERM_QUOTE : 
      TRACE("quote");
      fprintf (out, "(quote ");
      fprint_term (out,tree->quote.content) ;
      fprintf(out,")");
      break;
    case TYPE_TERM_ABSTRACTION : 
      TRACE("abstraction")
      fprintf(out,"(lambda (%s) ", tree->abstraction.variable);
      fprint_term (out, tree->abstraction.body) ;
      fprintf(out,")");
      break;
    case TYPE_TERM_CLOSURE : 
      TRACE("closure")
      fprintf(out,"(lambda (%s) ", tree->closure.variable);
      fprint_term (out, tree->closure.body) ;
      fprintf(out,")[");
      fprint_env(out, tree->closure.env);
      fprintf(out,"]");
      break;
    case TYPE_TERM_APPLICATION : 
      TRACE("application")
      fprintf(out,"(");
      fprint_term (out, tree->application.left) ;
      fprintf(out, " ");
      fprint_term (out, tree->application.right) ;
      fprintf(out,")");
      break;
    case TYPE_TERM_LET : 
      TRACE("let")
      fprintf(out,"(let (%s ", tree->let.variable);
      fprint_term (out, tree->let.init) ;
      fprintf(out, ") ");
      fprint_term (out, tree->let.body) ;
      fprintf(out,")");
      break;
    }
}

void fprint_env (FILE * out, struct ENV * env) {
  if (env != NULL) {
    if (env->next != NULL) {
      fprintf(out,"%s -> ", env->ident);
      fprint_term (out,env->term);
      printf(", ");
    }
    else{
      fprintf(out,"%s -> ", env->ident);
      fprint_term (out,env->term);
    }
  }
}

void fprint_debruijn (FILE * out, struct DEBRUIJN * debruijn) {
  switch (debruijn->type) {
  case TYPE_DEBRUIJN_INTEGER : {
    fprintf(out,"%d", debruijn->integer.value);
    break;
  }
  case TYPE_DEBRUIJN_VARIABLE : {
    fprintf(out,"[%d]", debruijn->variable.value);
    break; }
  case TYPE_DEBRUIJN_QUOTE : {
    fprintf(out,"(quote ");
    fprint_debruijn (out, debruijn->quote.value);
    fprintf(out,")");
    break ; }
  case TYPE_DEBRUIJN_ABSTRACTION : {
    fprintf(out,"(lambda [.] ");
    fprint_debruijn (out, debruijn->abstraction.body);
    fprintf(out,")");
    break ; }
  case TYPE_DEBRUIJN_CLOSURE : {
    fprint_debruijn (out, debruijn->closure.body);
    fprintf(out, " { ");
    fprint_stack (out, debruijn->closure.stack);
    fprintf(out, "}");
    break ; }
  case TYPE_DEBRUIJN_APPLICATION : {
    fprintf(out,"(");
    fprint_debruijn (out, debruijn->application.left);
    fprintf(out," ");
    fprint_debruijn (out, debruijn->application.right);
    fprintf(out,")");
    break ; }
  }
}

/* --------------------------------- DEBRUIJN ------------------------------- */

struct STACK * make_stack() {
  return NULL;
}

struct STACK * get_stack (struct STACK * stack, int position) {
  while (stack != NULL && position > 0) {
    position--;
    stack = stack->down;
  }
  if (stack == NULL) {
    fprintf(stderr, "Error: Not found position %d\n", position);
    exit(1);
  }
  else {
    return stack;
  }
}

int get_stack_position (struct STACK * stack, char * ident) {
  int n = 0;
  while (stack != NULL && strcmp (stack->ident, ident) != 0) {
    n++;
    stack = stack->down;
  }
  if (stack == NULL) {
    fprintf(stderr, "%s: Not found ident \"%s\"\n",__func__,  ident);
    exit(1);
  }
  else {
    return n;
  }
}

struct STACK * set_stack (struct STACK * stack,
				   char * ident,
				   struct DEBRUIJN * debruijn) {
  struct STACK * ret = malloc (sizeof (struct STACK));
  ret->ident = ident;
  ret->debruijn = debruijn;
  ret->down = stack;
  return ret;

}

void fprint_stack (FILE * out, struct STACK * stack) {
  while (stack != NULL) {
    fprintf(out,"%s -> ", stack->ident);
    fprint_debruijn (out, stack->debruijn);
    fprintf(out," ");
    stack = stack->down;
  }
}

struct DEBRUIJN * make_debruijn_closure (struct DEBRUIJN * body,
					 struct STACK * stack){
  struct DEBRUIJN * ret;
  ret = malloc (sizeof (struct DEBRUIJN));
  ret->type = TYPE_DEBRUIJN_CLOSURE;
  ret->closure.body = body;
  ret->closure.stack = stack;
  return ret;
}

struct DEBRUIJN * term_to_debruijn (struct TERM * term, struct STACK * stack) {
  TRACE("term_to_debruijn");
  struct DEBRUIJN * debruijn;
  debruijn = malloc (sizeof (struct DEBRUIJN));
  switch (term->type) {
  case TYPE_TERM_INTEGER : {
    TRACE("integer")
    debruijn->type = TYPE_DEBRUIJN_INTEGER;
    debruijn->integer.value = term->integer.value;
    break; }
  case TYPE_TERM_VARIABLE : {
    TRACE("variable")
    debruijn->type = TYPE_DEBRUIJN_VARIABLE;
    int pos = get_stack_position (stack, term->variable.value);
    debruijn->variable.value = pos;
    break; }
  case TYPE_TERM_QUOTE : {
    TRACE("quote")
    debruijn->type = TYPE_DEBRUIJN_QUOTE;
    debruijn->quote.value = term_to_debruijn (term->quote.content, NULL); // TODO: NULL stack ?
    break; }
  case TYPE_TERM_ABSTRACTION : {
    TRACE("abstraction")
    debruijn->type = TYPE_DEBRUIJN_ABSTRACTION;
    debruijn->abstraction.body = 
      term_to_debruijn (term->abstraction.body, 
			set_stack (stack, term->abstraction.variable, NULL));
    break; }
  case TYPE_TERM_CLOSURE : {
    TRACE("closure")
    debruijn->type = TYPE_DEBRUIJN_CLOSURE;
    debruijn->closure.body = 
      term_to_debruijn (term->closure.body, 
			set_stack (stack, term->closure.variable, NULL));
    debruijn->closure.stack = NULL; // TODO: env to stack :/
    break; }
  case TYPE_TERM_APPLICATION : {
    TRACE("application")
    debruijn->type = TYPE_DEBRUIJN_APPLICATION;
    debruijn->application.left = term_to_debruijn (term->application.left, stack);
    debruijn->application.right = term_to_debruijn (term->application.right, stack);
    break; }
  case TYPE_TERM_LET : {
    TRACE("let")
    struct DEBRUIJN * abstraction;
    abstraction = malloc (sizeof (struct DEBRUIJN));
    abstraction->type = TYPE_DEBRUIJN_ABSTRACTION;
    abstraction->abstraction.body = 
      term_to_debruijn (term->let.body,
			set_stack (stack, term->let.variable, NULL));
    debruijn->type = TYPE_DEBRUIJN_APPLICATION;
    debruijn->application.left = abstraction;
    debruijn->application.right = term_to_debruijn (term->let.init, stack);
    break; }
  }
  return debruijn;
}

struct TERM * debruijn_to_term (struct DEBRUIJN * debruijn, struct STACK * stack) {
  switch (debruijn->type) {
  case TYPE_DEBRUIJN_INTEGER : {
    TRACE("integer");
    return make_term_integer(debruijn->integer.value);
    break;
  }
  case TYPE_DEBRUIJN_VARIABLE : {
    TRACE("variable");
    struct STACK * v = get_stack (stack, debruijn->variable.value);
    return make_term_variable (v->ident);
    break;
  }
  case TYPE_DEBRUIJN_QUOTE : {
    TRACE("quote");
    return make_term_quote (debruijn_to_term (debruijn->quote.value, make_stack()));
    break ; 
  }
  case TYPE_DEBRUIJN_ABSTRACTION : {
    TRACE("abstraction");
    char * var = gen_ident();
    struct STACK * s = set_stack(stack, var, NULL); // We just want to get ident
    return make_term_abstraction (var,debruijn_to_term(debruijn->abstraction.body, s));
    break ; 
  }
  case TYPE_DEBRUIJN_CLOSURE : {
    TRACE("closure");
    char * var = gen_ident();
    struct STACK * s = set_stack(stack, var, NULL); // We just want to get ident
    return make_term_closure (var,debruijn_to_term(debruijn->abstraction.body, s), NULL); // Shouldn't be NULL. Should be stack->env transformation
    break ; 
  }
  case TYPE_DEBRUIJN_APPLICATION : {
    TRACE("application");
    return make_term_application(debruijn_to_term(debruijn->application.left, stack),
				 debruijn_to_term(debruijn->application.right, stack));
    break;
  }
  default:{
    fprintf(stderr, "Unknown debruijn term type\n");
    exit(1);
  }
  }
}

int compare_debruijn (struct DEBRUIJN * debruijn1, struct DEBRUIJN * debruijn2) {
  if (debruijn1->type == debruijn2->type) {
    switch (debruijn1->type) {
    case TYPE_DEBRUIJN_INTEGER : {
      return debruijn1->integer.value == debruijn2->integer.value;
      break ; }
    case TYPE_DEBRUIJN_VARIABLE : {
      return debruijn1->variable.value == debruijn2->variable.value;
      break ; }
    case TYPE_DEBRUIJN_QUOTE : {
      return compare_debruijn (debruijn1->quote.value,
			       debruijn2->quote.value);
      break ; }
    case TYPE_DEBRUIJN_ABSTRACTION : {
      return compare_debruijn (debruijn1->abstraction.body,
			       debruijn2->abstraction.body);
      break ; }
    case TYPE_DEBRUIJN_CLOSURE : {
      return compare_debruijn (debruijn1->closure.body,
			       debruijn2->closure.body);
      break ; }
    case TYPE_DEBRUIJN_APPLICATION : {
      return compare_debruijn (debruijn1->application.left,
			       debruijn2->application.left)
	&& compare_debruijn (debruijn1->application.right,
			     debruijn2->application.right);
      break ; }
    }
  }
  else { return 0 ; }
}

struct DEBRUIJN * eval_debruijn (struct DEBRUIJN * debruijn, struct STACK * stack) {
  switch (debruijn->type) {
  case TYPE_DEBRUIJN_INTEGER : {
    TRACE("integer");
    return debruijn;
    break;
  }
  case TYPE_DEBRUIJN_VARIABLE : {
    TRACE("variable");
    return get_stack (stack, debruijn->variable.value)->debruijn;
    break;
  }
  case TYPE_DEBRUIJN_QUOTE : {
    TRACE("quote");
    return debruijn->quote.value;
    break ; 
  }
  case TYPE_DEBRUIJN_ABSTRACTION : {
    TRACE("abstraction");
    return make_debruijn_closure (debruijn->abstraction.body, stack);
    break ; 
  }
  case TYPE_DEBRUIJN_CLOSURE : {
    TRACE("closure");
    return debruijn;
    break ; 
  }
  case TYPE_DEBRUIJN_APPLICATION : {
    TRACE("application");
    struct DEBRUIJN * v = eval_debruijn (debruijn->application.left, stack);
    switch (v->type) {
    case TYPE_DEBRUIJN_CLOSURE : {
      return eval_debruijn (v->closure.body, 
			    set_stack (v->closure.stack,
				       gen_ident(),
				       eval_debruijn (debruijn->application.right,
						      stack)));
    }
    default : {
      fprintf(stderr, "Not a debruijn closure\n");
      exit(1);
    }}
  }}
}
