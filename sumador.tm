maquina Sumador {
    alfabeto { 1, +, _ }
    estados { q0, q_mira_derecha, q_escribe_mas, q_fin }
    inicial: q0;
    finales: { q_fin }
    entrada: "11111+111";

    transiciones {
        q0, 1 -> q0, 1, DER;
        
        q0, + -> q_mira_derecha, +, DER;

        q_mira_derecha, 1 -> q_escribe_mas, +, IZQ;
    
        q_escribe_mas, + -> q0, 1, DER;

        q_mira_derecha, _ -> q_fin, _, QUIETO;
    }
}
