#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>

/* ----------------------------------- TRACE -------------------------------- */

FILE * debug_file;

#ifdef DEBUG    
#define TRACE(msg) fprintf(debug_file,"%s:%d %s (%s)\n",__FILE__,__LINE__,__func__,msg);
#define TRACE_DEBUG_MSG(msg) fprintf(debug_file,"%s:%d (%s) %s\n",__FILE__,__LINE__,__func__,msg);
#else
#define TRACE(msg)
#define TRACE_DEBUG_MSG(msg)
#endif

/* ------------------------------------ ENV --------------------------------- */

struct ENV {
  char * ident;
  struct TERM * term;
  struct ENV * next;
};

struct ENV * make_env();
struct TERM * get_env (struct ENV * env, char * ident);
struct ENV * set_env (struct ENV * env, char * ident, struct TERM * value);

/* ----------------------------------- TERM --------------------------------- */

enum TYPE_TERM {
  TYPE_TERM_INTEGER,
  TYPE_TERM_VARIABLE, 
  TYPE_TERM_QUOTE,
  TYPE_TERM_ABSTRACTION,
  TYPE_TERM_CLOSURE,
  TYPE_TERM_APPLICATION,
  TYPE_TERM_LET
};

struct TERM {
  enum TYPE_TERM type;
  union {
    struct {
      int value;
    } integer;
    struct {
      char * value;
    } variable;
    struct {
      struct TERM * content;
    } quote;
    struct {
      char * variable;
      struct TERM * body;
    } abstraction;
    struct {
      char * variable;
      struct TERM * body;
      struct ENV * env;
    } closure;
    struct {
      struct TERM * left;
      struct TERM * right;
    } application;
    struct {
      char * variable;
      struct TERM * init;
      struct TERM * body;
    } let;
  };
};

struct TERM * make_term_variable (char * value);
struct TERM * make_term_integer (int value);
struct TERM * make_term_quote (struct TERM * content);
struct TERM * make_term_abstraction (char * variable, struct TERM * body);
struct TERM * make_term_closure (char * variable, struct TERM * body, struct ENV * env);
struct TERM * make_term_application (struct TERM * left, struct TERM * right);
struct TERM * make_term_let (char * variable, struct TERM * init, struct TERM * body);

/* ----------------------------------- EVAL --------------------------------- */

struct TERM * evaluate_term (struct TERM * term, struct ENV * env);

/* ----------------------------------- PRINT -------------------------------- */

void fprint_term (FILE * out, struct TERM * term);
void fprint_env (FILE * out, struct ENV * env);

/* --------------------------------- DE BRUIJN ------------------------------ */

struct STACK {
  char * ident;
  struct DEBRUIJN * debruijn;
  struct STACK * down;
};

struct STACK * make_stack();
struct STACK * get_stack (struct STACK * stack, int position);
int get_stack_position (struct STACK * stack, char * ident);
struct STACK * set_stack (struct STACK * stack, char * ident, struct DEBRUIJN * debruijn);
void fprint_stack (FILE * out, struct STACK * stack);

enum TYPE_DEBRUIJN {
  TYPE_DEBRUIJN_INTEGER,
  TYPE_DEBRUIJN_VARIABLE,
  TYPE_DEBRUIJN_QUOTE,
  TYPE_DEBRUIJN_ABSTRACTION,
  TYPE_DEBRUIJN_CLOSURE,
  TYPE_DEBRUIJN_APPLICATION
};

struct DEBRUIJN {
  enum TYPE_DEBRUIJN type;
  union {
    struct {
      int value;
    } integer;
    struct {
      int value;
    } variable;
    struct {
      struct DEBRUIJN * value;
    } quote;
    struct {
      struct DEBRUIJN * body;
    } abstraction ;
    struct {
      struct DEBRUIJN * body;
      struct STACK * stack;
    } closure ;
    struct {
      struct DEBRUIJN * left;
      struct DEBRUIJN * right;
    } application ;
  };
};


struct DEBRUIJN * make_debruijn_closure (struct DEBRUIJN * debruijn, struct STACK * stack);
struct DEBRUIJN * term_to_debruijn (struct TERM * term, struct STACK * stack);
struct TERM * debruijn_to_term (struct DEBRUIJN * debruijn, struct STACK * stack);
int compare_debruijn (struct DEBRUIJN * debruijn1, struct DEBRUIJN * debruijn2);
void fprint_debruijn (FILE * out, struct DEBRUIJN * debruijn);
struct DEBRUIJN * eval_debruijn (struct DEBRUIJN * debruijn, struct STACK * stack);

struct TERM * eval_term (struct TERM * term, struct ENV * env);
#endif

