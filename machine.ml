type expression = 
  | EInteger of int
  | EVariable of string
  | EQuote of expression
  | EAbstraction of string * expression
  | EClosure of string * expression * env
  | EApplication of expression * expression
  | ELet of string * expression * expression

 and env = (string * expression) list

let rec get_env (env:env) (s:string) : expression =
  match env with
  | (s', e) :: rest-> 
     if String.compare s s' == 0 then e else get_env rest s
  | [] -> failwith ("Not found binding '" ^ s ^"'")

let set_env (env:env) (s:string) (e:expression) : env = 
  (s, e) :: env

let rec eeval (e:expression) (env:env) : expression = 
  match e with
  | EInteger (_) as n -> n
  | EVariable (s) -> get_env env s
  | EQuote (e) -> e
  | EAbstraction (s, e) -> EClosure (s, e, env)
  | EClosure _ -> failwith "Cannot eval closure"
  | EApplication (e1, e2) -> 
     (match eeval e1 env with
      | EClosure (s, e, env) -> 
	 eeval e (set_env env s e2)
      | _ -> failwith "Should be a closure")
  | ELet (s, e1, e2) ->
     let env' = (set_env env s (eeval e1 env)) in
     eeval e2 env'

and string_of_env (env:env) : string =
  match env with
  | [] -> ""
  | (s, e) :: [] -> 
     s ^ " -> " ^ (string_of_expression e)
  | (s, e) :: rest-> 
     s ^ " -> " ^ (string_of_expression e) ^ ", " ^ (string_of_env rest)

and string_of_expression (e:expression) : string =
  match e with
  | EInteger n -> string_of_int n
  | EVariable (s) -> s
  | EQuote (e) -> 
     "(quote " ^ (string_of_expression e) ^ ")"
  | EAbstraction (s, e) -> 
     "(lambda (" ^ s ^ ") " ^ (string_of_expression e) ^ ")"
  | EClosure (s, e, env) -> 
     "[closure (" ^ s ^ ") " ^ (string_of_expression e) ^ " {" ^ (string_of_env env) ^ "}]"
  | EApplication (e1, e2) -> 
     "(" ^ (string_of_expression e1) ^ " " ^ (string_of_expression e2) ^ ")"
  | ELet (s, e1, e2) -> 
     "(let (" ^ s ^ " " ^ (string_of_expression e1) ^ ") " ^ (string_of_expression e2) ^ ")"

(* DEBRUIJN *)

type dexpression = 
  | DInteger of int
  | DVariable of int
  | DQuote of dexpression
  | DAbstraction of dexpression
  | DClosure of dexpression * denv
  | DApplication of dexpression * dexpression
  | DEmpty

 and denv = (string * dexpression) list
				   
let counter = ref 0

let gensym () : string =
  let s = "x" ^ (string_of_int !counter) in
  counter := !counter + 1 ; 
  s

let rec get_denv_position (denv:denv) (s:string) (n:int) : int = 
  match denv with 
  | (s',_) :: next ->
     if String.compare s s' == 0 then n else get_denv_position next s (n + 1)
  | [] -> failwith ("Not found ident " ^ s ^ " in denv")

and get_denv_ident (denv:denv) (n:int) : string = 
  match denv with 
  | (s,_) :: next ->
     if n == 0 then s else get_denv_ident next (n - 1)
  | [] -> failwith ("Not found ident in denv")

and get_denv (denv:denv) (n:int) : dexpression = 
  match denv with 
  | (s,e) :: next ->
     if n == 0 then e else get_denv next (n - 1)
  | [] -> failwith ("Not found in denv")

and set_denv denv (s:string) (d:dexpression) : denv = 
  (s,d) :: denv

and dexpression_of_expression (expression:expression) (denv:denv) : dexpression = 
  match expression with
  | EInteger (n) -> DInteger (n)
  | EVariable (s) -> 
     DVariable (get_denv_position denv s 0)
  | EQuote (e) -> DQuote (dexpression_of_expression e denv)
  | EAbstraction (s, e) -> 
     DAbstraction (dexpression_of_expression e (set_denv denv s DEmpty))
  | EClosure (s, e, env) -> 
     DClosure (dexpression_of_expression e (set_denv denv s DEmpty), denv_of_env env)
  | EApplication (e1, e2) ->
     DApplication (dexpression_of_expression e1 denv,
		   dexpression_of_expression e2 denv)
  | ELet (s, e1, e2) ->
     DApplication (DAbstraction (dexpression_of_expression e2 (set_denv denv s DEmpty)),
		   dexpression_of_expression e1 denv)

and denv_of_env (env:env) : denv = 
  print_endline ("denv_of_env");
  match env with
  | [] -> []
  | (s, e) :: rest -> (s, (dexpression_of_expression e [])) :: (denv_of_env rest)

and env_of_denv (denv:denv) : env = 
  match denv with
  | [] -> []
  | (s, e) :: rest -> 
     (s, expression_of_dexpression e []) :: (env_of_denv rest)

and expression_of_dexpression (dexpression:dexpression) (denv:denv) : expression = 
  match dexpression with
  | DInteger (n) -> EInteger (n)
  | DVariable (n) -> EVariable (get_denv_ident denv n)
  | DQuote (e) -> EQuote (expression_of_dexpression e denv)
  | DAbstraction (e) -> 
     let s = gensym () in
     let denv' = set_denv denv s DEmpty in
     EAbstraction (s, expression_of_dexpression e denv')
  | DClosure (e, denv) ->
     let s = gensym () in
     let denv' = set_denv denv s DEmpty in
     EClosure (s, expression_of_dexpression e denv', env_of_denv denv)
  | DApplication (e1, e2) -> 
     EApplication (expression_of_dexpression e1 denv,
		   expression_of_dexpression e2 denv)
  | DEmpty -> failwith "DEmpty"

and string_of_dexpression (dexpression:dexpression) : string =
  match dexpression with
  | DInteger (n) -> string_of_int n
  | DVariable (n) -> "#" ^ (string_of_int n)
  | DQuote (e) -> 
     "(quote " ^ (string_of_dexpression e) ^ ")"
  | DAbstraction (e) -> 
     "(lambda () " ^ (string_of_dexpression e) ^ ")"
  | DClosure (e, denv) -> 
     "[closure [] " ^ (string_of_dexpression e) ^ " [" ^ (string_of_denv denv) ^ "]]"
  | DApplication (e1, e2) -> 
     "(" ^ (string_of_dexpression e1) ^ " " ^ (string_of_dexpression e2) ^ ")"
  | DEmpty -> ""

  and string_of_denv (denv:denv) : string =
    match denv with
    | [] -> ""
    | (s,e) :: [] -> 
       s ^ " -> " ^ (string_of_dexpression e)
    | (s,e) :: rest-> 
       s ^ " -> " ^ (string_of_dexpression e) ^ ", " ^ (string_of_denv rest)

let rec deval (dexpression:dexpression) (denv:denv) : dexpression = 
  match dexpression with
  | DInteger (_) as d -> d
  | DVariable (n) -> get_denv denv n
  | DQuote (e) -> e
  | DAbstraction (e) -> DClosure (e, denv)
  | DClosure (e, denv) -> failwith ("Cannot evaluate a closure")
  | DApplication (e1, e2) -> 
     (match deval e1 denv with
      | DClosure (e, denv') -> 
	 let v = deval e2 denv in
	 deval e (set_denv denv' (gensym()) v)
      | _ -> failwith "Is not a closure")
  | DEmpty -> failwith "DEmpty"

let eval (expression:expression) (env:env) : expression =
  print_endline ("eval");
  print_endline (string_of_expression expression);
  let denv = (denv_of_env env) in
  print_endline ("111");
  let dexpression = (dexpression_of_expression expression denv) in
  print_endline ("222");
  let dvalue = (deval dexpression denv) in
  print_endline ("333");
  let value = expression_of_dexpression dvalue denv in
  print_endline ("444");
  value
  
