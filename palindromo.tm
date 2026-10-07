subrutina marcar0 {
    INICIO, 0 -> FIN, #, DER;
}

subrutina marcar1 {
    INICIO, 1 -> FIN, +, DER;
}

maquina Palindromo {
    alfabeto { 0, 1, #, +, _ }
    estados { q_inicio, q_derecha0, q_derecha1, q_compara0, q_compara1, q_regresa, q_palindromo, q_no_palindromo }
    inicial: q_inicio;
    finales: { q_palindromo }
    entrada: "1101011";
    transiciones {

        usa marcar0: q_inicio -> q_derecha0;
        usa marcar1: q_inicio -> q_derecha1;

        q_inicio, # -> q_inicio, #, DER;
        q_inicio, + -> q_inicio, +, DER;

        q_inicio, _ -> q_palindromo, _, QUIETO;


        q_derecha0, 0 -> q_derecha0, 0, DER;
        q_derecha0, 1 -> q_derecha0, 1, DER;
        q_derecha0, # -> q_derecha0, #, DER;
        q_derecha0, + -> q_derecha0, +, DER;

        q_derecha0, _ -> q_compara0, _, IZQ;


        q_derecha1, 0 -> q_derecha1, 0, DER;
        q_derecha1, 1 -> q_derecha1, 1, DER;
        q_derecha1, # -> q_derecha1, #, DER;
        q_derecha1, + -> q_derecha1, +, DER;

        q_derecha1, _ -> q_compara1, _, IZQ;


        q_compara0, # -> q_compara0, #, IZQ;
        q_compara0, + -> q_compara0, +, IZQ;

        q_compara0, 0 -> q_regresa, #, IZQ;

        q_compara0, 1 -> q_no_palindromo, 1, QUIETO;

        q_compara0, _ -> q_palindromo, _, QUIETO;


        q_compara1, # -> q_compara1, #, IZQ;
        q_compara1, + -> q_compara1, +, IZQ;

        q_compara1, 1 -> q_regresa, +, IZQ;

        q_compara1, 0 -> q_no_palindromo, 0, QUIETO;

        q_compara1, _ -> q_palindromo, _, QUIETO;

        q_regresa, 0 -> q_regresa, 0, IZQ;
        q_regresa, 1 -> q_regresa, 1, IZQ;
        q_regresa, # -> q_regresa, #, IZQ;
        q_regresa, + -> q_regresa, +, IZQ;

        q_regresa, _ -> q_inicio, _, DER;
    }
}