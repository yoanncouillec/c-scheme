all: fun

fun: machine.cmo parser.cmi parser.cmo lexer.cmo fun.cmo
	ocamlc -o $@ machine.cmo parser.cmo lexer.cmo fun.cmo

%.cmi: %.mli
	ocamlc $^

.SUFFIXES: .mll .mly .mli .ml .cmi .cmo .cmx

.mll.mli:
	ocamllex $<

.mll.ml:
	ocamllex $<

.mly.mli:
	ocamlyacc $<

.mly.ml:
	ocamlyacc $<

.mli.cmi:
	ocamlc -c $^

.ml.cmo:
	ocamlc -c $^

test: fun
	./fun < test/test.integer.fun
	./fun < test/test.quote.fun
	./fun < test/test.lambda.fun
	./fun < test/test.let.fun
	./fun < test/test.application.fun
	./fun < test/test.lambda0.fun
	./fun < test/test.lambda1.fun
	./fun < test/test.lambda2.fun
	./fun < test/test.lambda3.fun
	./fun < test/test.cons.fun
	./fun < test/test.fst.fun
	./fun < test/test.snd.fun


clean:
	rm -rf *.cm* fun *~ \#*\# *.mli parser.ml
