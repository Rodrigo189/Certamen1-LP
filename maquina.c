#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "maquina.h"

Maquina m;
Subrutina subrutinas[MAX_SUB];
int nsubrutinas = 0;
// temporal mientras se lee una subrutina, sus transiciones se acumulan aqui al cerrar el bloque se copia al arreglo y se limpia.
Subrutina sub_actual;

int estado_declarado(const char *nombre) {
    // Recorre los "n" primeros estados
    for (int i = 0; i < m.nestados; i++) {
        // strcmp devuelve 0 cuando las cadenas son identicas
        if (strcmp(m.estados[i], nombre) == 0)
        return 1;   //y ahi se corta: el estado existe
    }
    return 0;   // se acabaron los estados y ninguno coincidio
}

int es_final(const char *s) {
    for(int i = 0; i < m.nfinales; i++) {
        if(!strcmp(m.finales[i], s)) return 1;   // encontrado: se acepta
    }
    return 0;   // no estaba en la lista de finales
}

int simbolo_en_alfabeto(char c) {
    for(int i = 0; i < m.nalfabeto; i++) {
        if(m.alfabeto[i] == c) return 1;
    }
    return 0;
}

int agregar_transicion(const char *estado, char c_alfabeto, const char *nuevo_estado, char escribir, char mov) {
    // Debe existir tanto un estado inicial y destino, si no, no se guardara nada
    if(!estado_declarado(estado) || !estado_declarado(nuevo_estado)) {
        fprintf(stderr, "Error semantico: Estado no declarado (%s -> %s)\n", estado, nuevo_estado);
        return 0;   // no se guarda nada
    }

    // Debe existir el simbolo en el alfabeto, si no, no se hara nada
    if(!simbolo_en_alfabeto(c_alfabeto) || !simbolo_en_alfabeto(escribir)) {
        fprintf(stderr, "Error semantico: Simbolo '%c' o '%c' fuera de alfabeto\n", c_alfabeto, escribir);
        return 0;
    }

    // La maquina es Determinista, si hay 1 estado con 2 salidas con el mismo caracter del alfabeto, no sabra donde ir, entonces es un error
    for(int i = 0; i < m.ntrans; i++) {
        if(!strcmp(m.trans[i].estado, estado) && m.trans[i].leer == c_alfabeto) {
            fprintf(stderr, "Error semantico: Transicion duplicada (No determinismo) en %s con '%c'\n", estado, c_alfabeto);
            return 0;
        }
    }
    // simplemente para evitar que supere el limite, no explote la maquina
    if(m.ntrans >= MAX_TRANS) return 0;
    // si esta todo bien, almacenara todo los datos de la transicion, en este caso "t".
    Transicion *t = &m.trans[m.ntrans++];
    strcpy(t->estado, estado);              // nombre del estado origen
    strcpy(t->nuevo_estado, nuevo_estado);  // nombre del estado destino
    t->leer = c_alfabeto;                   // simbolo que la dispara
    t->escribir = escribir;                 // simbolo nuevo para la celda
    t->mov = mov;                           // 'D', 'I' o 'Q'
    return 1;
}

// Declara un estado, pero si no existia anteriormente
static void declarar_estado_generado(const char *nombre) {
    // Su estado existe todavia Y queda sitio en el arreglo.
    if(!estado_declarado(nombre) && m.nestados < MAX_ESTADOS) {
        strcpy(m.estados[m.nestados++], nombre);  // copia y avanza el contador
    }
}

int expandir_subrutina(const char *nom_sub, int val_param, const char *est_origen, const char *est_destino) {
    Subrutina *s = NULL;   // puntero a la subrutina buscada

    //Busca la subrutina por nombre en el arreglo global.
    for(int i = 0; i < nsubrutinas; i++) {
        if(!strcmp(subrutinas[i].nombre, nom_sub)) {
            s = &subrutinas[i];   // apunta directamente al original
            break;                // coincide con la subrutina, no buscamos hasta el siguiente
        }
    }
    if(!s) {
        fprintf(stderr, "Error semantico: Subrutina %s no existe\n", nom_sub);
        return 0;   // se produjo ese error, se aborta
    }
    // Si tiene repeticiones, tomara el "val_param", si no, solo se hara una sola vez la subrutina
    int repeticiones = (s->param) ? val_param : 1;

    // e_act = estado donde ENTRA la repeticion actual, e_sig = estado donde SALDRA esa repeticion
    char e_act[120], e_sig[120];
    strcpy(e_act, est_origen);   // la primera entra por el estado pedido

    for(int k = 0; k < repeticiones; k++) {
        // la ultima repeticion conecta con el est_destino del usuario
        if(k == repeticiones - 1) {
            strcpy(e_sig, est_destino);
        } else {
            // las intermedias con un estado inventado para que la siguiente repeticion tenga por donde engancharse.
            sprintf(e_sig, "%s_%d", nom_sub, k);   // p.ej. "avanzar_0"
        }
        //Recorre las transiciones que forman la subrutina
        for(int j = 0; j < s->ntrans; j++) {
            // nombre generado para estado, ej. "avanzar_0_M"
            char origen_real[120], destino_real[120];

            //  Renombra el ORIGEN de esta transicion, "INICIO" es el punto de entrada: se une a e_act.
            if(!strcmp(s->trans[j].estado, "INICIO") || !strcmp(s->trans[j].estado, s->trans[0].estado)) {
                strcpy(origen_real, e_act);
            } else {
                // cualquier otro estado es interno y se hace unico con subrutina + repeticion + estado original.
                sprintf(origen_real, "%s_%d_%s", nom_sub, k, s->trans[j].estado);
            }

            // Renombra el DESTINO de esta transicion, "FIN" es el punto de salida: se une a e_sig.
            if(!strcmp(s->trans[j].nuevo_estado, "FIN")) {
                strcpy(destino_real, e_sig);
            } else if(!strcmp(s->trans[j].nuevo_estado, "INICIO") || !strcmp(s->trans[j].nuevo_estado, s->trans[0].estado)) {
            // Si es que debe hacer llamado al mismo estado (ej q1 -> q1), no haya problemas
                strcpy(destino_real, e_act);
            } else {
                // cualquier otro estado es interno y se hace unico con subrutina + repeticion + estado original.
                sprintf(destino_real, "%s_%d_%s", nom_sub, k, s->trans[j].nuevo_estado);
            }

            // El resto de estados quedarian fuera de la lista de la maquina y agregar_transicion rechazaria la transicion.
            declarar_estado_generado(origen_real);
            declarar_estado_generado(destino_real);

            // Añade la transicion ya, se valida normal, igual que una transicion escrita a mano por el usuario.
            if(!agregar_transicion(origen_real, s->trans[j].leer, destino_real, s->trans[j].escribir, s->trans[j].mov)) {
                return 0;   // si falla una sola, se aborta toda la expansion
            }
        }
        // lo que salia de esta repeticion entra en la siguiente (ej: e1 -> e2   pasa a:  e2 -> algo )
        strcpy(e_act, e_sig);
    }
    return 1;   // expansion completa sin errores
}

static void mostrar_cinta(void) {
    // Recorre las MAX_CINTA (50) celdas de la cinta
    for(int i = 0; i < 50; i++) {
        // y imprime cada una. El operador ? : convierte el nulo en el caracter _
        putchar(m.cinta[i] ? m.cinta[i] : '_');
    }
    putchar('\n');  // salto para la flecha
    for(int i = 0; i < m.cabeza; i++) putchar(' ');   // espacio hasta la cabeza
    puts("^");    // la flecha indicando donde esta 
}


void ejecutar(void) {
    printf("=== SIMULACION: %s ===\n", m.nombre);   // cabecera de la traza
    for(int paso = 0; paso < 500; paso++) {
        // Imprime en que paso, en que estado y donde esta la cabeza
        printf("Paso %d | Estado: %s | Cabezal: %d\n", paso, m.estado_actual, m.cabeza);
        mostrar_cinta();   // dibuja la cinta con su flecha
        // Se comprueba ANTES de buscar transicion: si ya se llego a un estado final, la cadena se acepta y listo.
        if(es_final(m.estado_actual)) {
            puts("RESULTADO: ACEPTADA");
            return;
        }

        int ok = 0; // variable para ver si todo termino bien; 0 si esta mal

        //Busca la transicion que corresponde
        for(int i = 0; i < m.ntrans; i++) {
            Transicion *t = &m.trans[i];   // para no escribir m.trans[i] mil veces xd

            // Deben coincidir el estado actual Y el simbolo de la celda
            if(!strcmp(t->estado, m.estado_actual) && t->leer == m.cinta[m.cabeza]) {
                m.cinta[m.cabeza] = t->escribir;      // escribe en la celda
                strcpy(m.estado_actual, t->nuevo_estado);   // cambia de estado
                
                // movimiento de la cabezera dependiendo hacia donde se dirige la transicion
                if(t->mov == 'D') m.cabeza++;      // derecha
                else if(t->mov == 'I') m.cabeza--; // izquierda

                // Si supera el borde de la cinta, tirara error.
                if(m.cabeza < 0 || m.cabeza >= MAX_CINTA) {
                    puts("ERROR: Borde de cinta alcanzado");
                    return;   // simulacion interrumpida
                }

                ok = 1;   // si se aplico una transicion
                break;    // no hace falta seguir buscando
            }
        }

        if(!ok) {
            // se verifica si el estado en el que se encuentra, es un estado final o de aceptacion, si lo es, mandara mensaje correcto
            puts(es_final(m.estado_actual) ? "RESULTADO: ACEPTADA" : "RESULTADO: RECHAZADA");
            return;   // fin de la simulacion
        }
    }

    // si se agotaron el limite de pasos, tirara el mensaje y se detendra
    puts("Detenido: Limite de pasos alcanzado.");
}

// Pone la maquina a cero antes de empezar para los contadores y atributos de la maquina
void inicializar_maquina(void) {
    memset(&m, 0, sizeof(Maquina));
}

// Añade un simbolo al alfabeto. Ignora duplicados y respeta el tope.
void agregar_simbolo_alfabeto(char c) {
    if (!simbolo_en_alfabeto(c) && m.nalfabeto < MAX_ALFABETO) {
        m.alfabeto[m.nalfabeto++] = c;   // guardar y avanzar el contador
    }
}

// Añade un estado al bloque "estados { }". Respeta el tope (100).
void agregar_estado_maquina(const char *nombre) {
    if (m.nestados < MAX_ESTADOS) {
        strcpy(m.estados[m.nestados++], nombre);
    }
}

// Añade un estado al bloque "finales { }"
void agregar_estado_final(const char *nombre) {
    if (m.nfinales < MAX_ESTADOS) {
        strcpy(m.finales[m.nfinales++], nombre);
    }
}

// Añade una transicion "sub_actual"
void agregar_transicion_subrutina(const char *estado, char c_alfabeto, const char *nuevo_estado, char escribir, char mov) {
    // Se escribe en sub_actual, NO en m. Tampoco se comprueba MAX_TRANS.
    Transicion *t = &sub_actual.trans[sub_actual.ntrans++];
    strcpy(t->estado, estado);
    strcpy(t->nuevo_estado, nuevo_estado);
    t->leer = c_alfabeto;
    t->escribir = escribir;
    t->mov = mov;
}

// guarda lo acumulado en sub_actual, dentro del arreglo global y deja el bucle limpio para la siguiente.
void registrar_subrutina(const char *nombre, int tiene_param) {
    strcpy(sub_actual.nombre, nombre);
    sub_actual.param = tiene_param;           // 1 = declarada con (parametro)
    subrutinas[nsubrutinas++] = sub_actual;
    memset(&sub_actual, 0, sizeof(Subrutina));
}

int finalizar_declaracion_maquina(const char *nombre, const char *estado_actual, const char *entrada) {
    strcpy(m.nombre, nombre);            // para la cabecera de la traza
    strcpy(m.estado_actual, estado_actual);    // estado actual = estado inicial
    memset(m.cinta, '_', MAX_CINTA);

    // Carga la entrada en la cinta (si hay)
    if (entrada) {
        int len = strlen(entrada);   // longitud de la cadena
        // La entrada se CENTRA en la celda 25, para que la maquina tenga como moverse de izquierda y derecha
        int inicio = MAX_CINTA / 2;   // = 25

        // Si la cadena es larga demas se TRUNCA en lugar de desbordar
        if (len > MAX_CINTA - inicio)
            len = MAX_CINTA - inicio;

        memcpy(&m.cinta[inicio], entrada, len);   // copia la cadena ahi
        // La cabeza arranca sobre el primer caracter de la entrada
        m.cabeza = inicio;
    }

    // el estado inicial debe existir y debe haber finales sin estados finales la maquina nunca podria aceptar nada.
    if (!estado_declarado(m.estado_actual) || m.nfinales == 0) {
        fprintf(stderr, "Error semantico: Estado inicial invalido o falta de estados finales.\n");
        return 0;
    }
    return 1;   // todo correcto, la maquina queda lista para ejecutar
}