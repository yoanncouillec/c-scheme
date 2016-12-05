#include "machine.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ------------------------------------ ENV --------------------------------- */

struct ENV * make_env () {
  return NULL;
}

struct VALUE * get_env (struct ENV * env, char * ident) {
  if (env == NULL) {
    fprintf (stderr, "get_env : no such binding %s\n", ident);
    return NULL;
  }
  else {
    if (strcmp (ident, env->ident) == 0) {
      return env->value;
    }
    else {
      return get_env (env->next, ident);
    }
  }
}

struct ENV * set_env (struct ENV * env, char * ident, struct VALUE * value) {
  struct ENV * ret = malloc (sizeof (struct ENV));
  ret->ident = ident;
  ret->value = value;
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
  ret->quote.term = term;
  return ret;
}

struct TERM * make_term_abstraction (char * variable, struct TERM * term) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_ABSTRACTION;
  ret->abstraction.variable = variable;
  ret->abstraction.term = term;
  return ret;
}

struct TERM * make_term_application (struct TERM * term1, struct TERM * term2) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_APPLICATION;
  ret->application.term1 = term1;
  ret->application.term2 = term2;
  return ret;
}

struct TERM * make_term_let (char * variable, struct TERM * term1, struct TERM * term2) {
  struct TERM * ret = malloc (sizeof (struct TERM));
  ret->type = TYPE_TERM_LET;
  ret->let.variable = variable;
  ret->let.term1 = term1;
  ret->let.term2 = term2;
  return ret;
}

/* ----------------------------------- VALUE -------------------------------- */

struct VALUE * make_value_integer (int value) {
  struct VALUE * ret = malloc (sizeof (struct VALUE));
  ret->type = TYPE_VALUE_INTEGER;
  ret->integer.value = value;
  return ret;
}

struct VALUE * make_value_quote (struct TERM * term) {
  struct VALUE * ret = malloc (sizeof (struct VALUE));
  ret->type = TYPE_VALUE_QUOTE;
  ret->quote.term = term;
  return ret;
}

struct VALUE * make_value_closure (struct TERM * term, struct ENV * env) {
  struct VALUE * ret = malloc (sizeof (struct VALUE));
  ret->type = TYPE_VALUE_CLOSURE;
  ret->closure.term = term;
  ret->closure.env = env;
  return ret;
}

/* ----------------------------------- EVAL --------------------------------- */

struct VALUE * evaluate_term (struct TERM * term, struct ENV * env) {
  switch (term->type) {
  case TYPE_TERM_INTEGER : {
    return make_value_integer (term->integer.value) ; }
  case TYPE_TERM_VARIABLE : {
    return get_env (env, term->variable.value) ; }
  case TYPE_TERM_QUOTE : {
    return make_value_quote (term->quote.term) ; }
  case TYPE_TERM_ABSTRACTION : {
    return make_value_closure (term, env); }
  case TYPE_TERM_APPLICATION : {
    struct VALUE * value1 = evaluate_term (term->application.term1, env);
    switch (value1->type) {
    case TYPE_VALUE_CLOSURE : {
      switch (value1->closure.term->type) {
      case TYPE_TERM_ABSTRACTION : {
	return evaluate_term (value1->closure.term->abstraction.term, 
			      set_env (value1->closure.env,
				       value1->closure.term->abstraction.variable, 
				       evaluate_term (term->application.term2, 
						      env)));
      }
      default : {
	fprintf (stderr, "Not an abstraction\n");
	return NULL;
      }}
    }
    default : {
      fprintf (stderr, "Not a closure\n");
      return NULL;
    }}}
  case TYPE_TERM_LET : {
    struct VALUE * value1 = evaluate_term (term->let.term1, env);
    return evaluate_term (term->let.term2, set_env (env, term->let.variable, value1));
  }}
}

/* ----------------------------------- PRINT -------------------------------- */

/* Return the ident. coresponding to a debruijn term */
/* char * get_alias (struct ENV * env, struct DEBRUIJN * debruijn) { */
/*   if (env == NULL) { */
/*     return NULL; */
/*   } */
/*   else { */
/*     if (compare_debruijn (debruijn, term_to_debruijn (env->value, NULL)) == 0) { */
/*       return env->ident; */
/*     } */
/*     else { */
/*       return get_alias (env->next, debruijn); */
/*     } */
/*   } */
/* } */

void fprint_term (FILE * out, struct TERM * tree, struct ENV * env) {
  //struct DEBRUIJN * debruijn = term_to_debruijn (tree, NULL);
  //char * alias = get_alias (env, debruijn);
  //if (alias == NULL) {
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
      fprint_term (out,tree->quote.term, env) ;
      fprintf(out,")");
      break;
    case TYPE_TERM_ABSTRACTION : 
      TRACE("abstraction")
      fprintf(out,"(lambda (%s) ", tree->abstraction.variable);
      fprint_term (out, tree->abstraction.term, env) ;
      fprintf(out,")");
      break;
    case TYPE_TERM_APPLICATION : 
      TRACE("application")
      fprintf(out,"(");
      fprint_term (out, tree->application.term1, env) ;
      fprintf(out, " ");
      fprint_term (out, tree->application.term2, env) ;
      fprintf(out,")");
      break;
    case TYPE_TERM_LET : 
      TRACE("let")
      fprintf(out,"(let (%s ", tree->let.variable);
      fprint_term (out, tree->let.term1, env) ;
      fprintf(out, ")\n");
      fprint_term (out, tree->let.term2, env) ;
      fprintf(out,")");
      break;
    }
    //}
    //else {
    //fprintf(out,"%s", alias);
    //}
}

void fprint_value (FILE * out, struct VALUE * value, struct ENV * env) {
  switch (value->type) {
  case TYPE_VALUE_INTEGER :
    fprintf(out,"%d", value->integer.value);
    break;
  case TYPE_VALUE_QUOTE :
    fprintf(out,"(quote ");
    fprint_term (out,value->quote.term, NULL);
    fprintf(out,")");
    break;
  case TYPE_VALUE_CLOSURE : 
    fprint_term (out,value->closure.term, env) ;
    fprintf(out,"[");
    fprint_env (out,value->closure.env);
    fprintf(out,"]");
    break;
  }
}

void fprint_env (FILE * out, struct ENV * env) {
  if (env != NULL) {
    if (env->next != NULL) {
      fprintf(out,"%s -> ", env->ident);
      fprint_value (out,env->value, env);
      printf(", ");
    }
    else{
      fprintf(out,"%s -> ", env->ident);
      fprint_value (out,env->value, env);
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
    fprint_debruijn (out, debruijn->quote.debruijn);
    fprintf(out,")");
    break ; }
  case TYPE_DEBRUIJN_ABSTRACTION : {
    fprintf(out,"(lambda [.] ");
    fprint_debruijn (out, debruijn->abstraction.debruijn);
    fprintf(out,")");
    break ; }
  case TYPE_DEBRUIJN_CLOSURE : {
    fprint_debruijn (out, debruijn->closure.debruijn);
    fprintf(out, " { ");
    fprint_stack (out, debruijn->closure.stack);
    fprintf(out, "}");
    break ; }
  case TYPE_DEBRUIJN_APPLICATION : {
    fprintf(out,"(");
    fprint_debruijn (out, debruijn->application.debruijn1);
    fprintf(out," ");
    fprint_debruijn (out, debruijn->application.debruijn2);
    fprintf(out,")");
    break ; }
  }
}

/* --------------------------------- DEBRUIJN ------------------------------- */

struct STACK * make_stack() {
  return NULL;
}

struct DEBRUIJN * get_stack (struct STACK * stack, int position) {
  while (stack != NULL && position > 0) {
    position--;
    stack = stack->down;
  }
  if (stack == NULL) {
    fprintf(stderr, "Error: Not found position %d\n", position);
    exit(1);
  }
  else {
    return stack->debruijn;
  }
}

int get_stack_position (struct STACK * stack, char * ident) {
  int n = 0;
  while (stack != NULL && strcmp (stack->ident, ident) != 0) {
    n++;
    stack = stack->down;
  }
  if (stack == NULL) {
    fprintf(stderr, "Error: Not found ident \"%s\"\n", ident);
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

struct DEBRUIJN * make_debruijn_closure (struct DEBRUIJN * debruijn,
					 struct STACK * stack){
  struct DEBRUIJN * ret;
  ret = malloc (sizeof (struct DEBRUIJN));
  ret->type = TYPE_DEBRUIJN_CLOSURE;
  ret->closure.debruijn = debruijn;
  ret->closure.stack = stack;
  return ret;
}

struct DEBRUIJN * term_to_debruijn (struct TERM * term, struct STACK * stack) {
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
    debruijn->quote.debruijn = term_to_debruijn (term->quote.term, NULL);
    break; }
  case TYPE_TERM_ABSTRACTION : {
    TRACE("abstraction")
    debruijn->type = TYPE_DEBRUIJN_ABSTRACTION;
    debruijn->abstraction.debruijn = 
      term_to_debruijn (term->abstraction.term, 
			set_stack (stack, term->abstraction.variable, NULL));
    break; }
  case TYPE_TERM_APPLICATION : {
    TRACE("application")
    debruijn->type = TYPE_DEBRUIJN_APPLICATION;
    debruijn->application.debruijn1 = term_to_debruijn (term->application.term1, stack);
    debruijn->application.debruijn2 = term_to_debruijn (term->application.term2, stack);
    break; }
  case TYPE_TERM_LET : {
    TRACE("let")
    struct DEBRUIJN * abstraction;
    abstraction = malloc (sizeof (struct DEBRUIJN));
    abstraction->type = TYPE_DEBRUIJN_ABSTRACTION;
    abstraction->abstraction.debruijn = 
      term_to_debruijn (term->let.term2,
			set_stack (stack, term->let.variable, NULL));
    debruijn->type = TYPE_DEBRUIJN_APPLICATION;
    debruijn->application.debruijn1 = abstraction;
    debruijn->application.debruijn2 = term_to_debruijn (term->let.term1, stack);
    break; }
  }
  return debruijn;
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
      return compare_debruijn (debruijn1->quote.debruijn,
			       debruijn2->quote.debruijn);
      break ; }
    case TYPE_DEBRUIJN_ABSTRACTION : {
      return compare_debruijn (debruijn1->abstraction.debruijn,
			       debruijn2->abstraction.debruijn);
      break ; }
    case TYPE_DEBRUIJN_CLOSURE : {
      return compare_debruijn (debruijn1->closure.debruijn,
			       debruijn2->closure.debruijn);
      break ; }
    case TYPE_DEBRUIJN_APPLICATION : {
      return compare_debruijn (debruijn1->application.debruijn1,
			       debruijn2->application.debruijn1)
	&& compare_debruijn (debruijn1->application.debruijn2,
			     debruijn2->application.debruijn2);
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
    return get_stack (stack, debruijn->variable.value);
    break;
  }
  case TYPE_DEBRUIJN_QUOTE : {
    TRACE("quote");
    return debruijn->quote.debruijn;
    break ; 
  }
  case TYPE_DEBRUIJN_ABSTRACTION : {
    TRACE("abstraction");
    return make_debruijn_closure (debruijn, stack);
    break ; 
  }
  case TYPE_DEBRUIJN_CLOSURE : {
    TRACE("closure");
    return debruijn;
    break ; 
  }
  case TYPE_DEBRUIJN_APPLICATION : {
    TRACE("application");
    struct DEBRUIJN * v = eval_debruijn (debruijn->application.debruijn1, stack);
    switch (v->type) {
    case TYPE_DEBRUIJN_CLOSURE : {
      switch (v->closure.debruijn->type) {
      case TYPE_DEBRUIJN_ABSTRACTION : {
	return eval_debruijn (v->closure.debruijn->abstraction.debruijn, 
			      set_stack (v->closure.stack,
					 NULL,
					 eval_debruijn (debruijn->application.debruijn2,
							stack)));
	break;
      }
      default: {
	fprintf (stderr, "Not a debruijn abstraction\n");
	exit(1);
      }}
    }
    default : {
      fprintf(stderr, "Not a debruijn closure\n");
      exit(1);
    }}
  }}
}
