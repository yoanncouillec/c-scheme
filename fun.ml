open Machine

let enrich_env env0 = 
  let econs = EAbstraction ("x", EAbstraction ("y", EAbstraction ("f", EApplication (EApplication (EVariable ("f"), EVariable ("x")), EVariable ("y"))))) in
  let env1 = set_env env0 "cons" (eval econs env0) in
  let efirst = EAbstraction ("x", EAbstraction ("y", EVariable ("x"))) in
  let env2 = set_env env1 "first" (eval efirst env1) in
  let efst = EAbstraction ("x", EApplication (EVariable "x", EVariable "first")) in
  let env3 = set_env env2 "fst" (eval efst env2) in
  env3

let _ = 
  let verbose = ref false in
  let options = [
    "-v", Arg.Set verbose, "Verbose mode";
  ] in
    Arg.parse options (fun x -> ()) "Options: ";
  let in_chan = stdin in
  let out_chan = stdout in
  let lexbuf = Lexing.from_channel in_chan in
  let expression = Parser.start Lexer.token lexbuf in
  let value = eval expression (enrich_env []) in
  print_endline (string_of_expression expression) ;
  print_string "=> " ;
  print_endline (string_of_expression value) ;
  close_out out_chan
