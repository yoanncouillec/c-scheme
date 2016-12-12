{
  open Parser
}
rule token = parse
  | eof { EOF }
  | [' ' '\t' '\n'] { token lexbuf }
  | '(' { LPAREN }
  | ')' { RPAREN }
  | "quote" { QUOTE }
  | "lambda" { LAMBDA }
  | "let" { LET }
  | ['0'-'9']+ { INT (int_of_string (Lexing.lexeme lexbuf)) }
  | ['a'-'z''A'-'Z']['a'-'z''A'-'Z''0'-'9']* { IDENT (Lexing.lexeme lexbuf) }
