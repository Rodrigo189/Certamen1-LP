%{
#include <stdio.h>
#include <stdlib.h>
#include "maquina.h"

// El lexer se implementa en lexer.l
int yylex(void);

// Mensaje de error de bison
void yyerror(const char *s) { fprintf(stderr, "Error sintactico: %s\n", s); }

// Marca que hubo un error semantico
int error_semantico = 0;
%}

// Tipos de datos que viajan en yylval
%union {
    char *str;
    char ch;
    int num;
}

// Tokens que no llevan valor
%token MAQUINA SUBRUTINA USA ALFABETO ESTADOS INICIAL FINALES TRANSICIONES ENTRADA
%token DER IZQ QUIETO

// Tokens que si llevan valor
%token <str> ID CADENA
%token <ch> SIMBOLO
%token <num> NUMERO

// Tipos de los no terminales que producen valor
%type <str> estado
%type <ch> movimiento simbolo_o_num

%%

// Un programa = subrutinas (0 o mas) + maquinas (1 o mas)
programa: decl_subrutinas decl_maquinas ;

// Lista de definiciones de subrutinas
decl_subrutinas: decl_subrutinas def_subrutina 
               |;

// Subrutina con parametro o sin parametro
def_subrutina: SUBRUTINA ID[nombre] '(' ID ')' '{' lista_trans_sub '}' { registrar_subrutina($nombre, 1); } 
             | SUBRUTINA ID[nombre] '{' lista_trans_sub '}'           { registrar_subrutina($nombre, 0); } ;

// Una o mas transiciones dentro de la subrutina
lista_trans_sub: lista_trans_sub trans_sub 
                | trans_sub ;

// Transicion de subrutina
trans_sub: ID[estado] ',' simbolo_o_num[c_alfabeto] '-' '>' ID[nuevo_estado] ',' simbolo_o_num[escribir] ',' movimiento[m] ';' {
    agregar_transicion_subrutina($estado, $c_alfabeto, $nuevo_estado, $escribir, $m);
};

// Definicion de la maquina
decl_maquinas: def_maquina ;

// Bloque completo de una maquina de Turing
def_maquina: MAQUINA ID[nombre] '{' 
    ALFABETO '{' simbolos '}' 
    ESTADOS '{' lista_estados '}' 
    INICIAL ':' estado[inicial] ';' 
    FINALES ':' '{' lista_finales '}' 
    ENTRADA ':' CADENA[entrada] ';' 
    TRANSICIONES '{' cuerpo_trans '}' '}' {
        if (!finalizar_declaracion_maquina($nombre, $inicial, $entrada)) {
            error_semantico = 1;
        }
};

// Lista de simbolos del alfabeto, separados por coma
simbolos: simbolos ',' simbolo_o_num { agregar_simbolo_alfabeto($3); }
        | simbolo_o_num              { agregar_simbolo_alfabeto($1); } ;

// Un simbolo del alfabeto
simbolo_o_num: SIMBOLO 
             | NUMERO { $$ = $1 + '0'; } ;

// Lista de estados, separados por coma
lista_estados: lista_estados ',' estado { agregar_estado_maquina($3); }
             | estado                   { agregar_estado_maquina($1); } ;

// Un estado es simplemente un identificador
estado: ID ;

// Lista de estados finales, separados por coma
lista_finales: lista_finales ',' estado { agregar_estado_final($3); }
             | estado                   { agregar_estado_final($1); } ;

// Transiciones: pueden ser directas o llamadas a subrutinas
cuerpo_trans: cuerpo_trans elemento_trans 
            | elemento_trans ;

elemento_trans: trans 
              | instanciacion_sub ;

// Transicion directa de la maquina: se valida y agrega
trans: ID[entrada] ',' simbolo_o_num[c_alfabeto] '-' '>' ID[nuevo_estado] ',' simbolo_o_num[escribir] ',' movimiento[m] ';' {
    if (!agregar_transicion($entrada, $c_alfabeto, $nuevo_estado, $escribir, $m)) {
        error_semantico = 1;
    }
};

// Instancia de subrutina con parametro (cantidad de repeticiones)
instanciacion_sub: USA ID[nombre] '(' NUMERO[cantidad] ')' ':' estado[estado_1] '-' '>' estado[estado_2] ';' { 
                     if (!expandir_subrutina($nombre, $cantidad, $estado_1, $estado_2)) error_semantico = 1; 
                 }
                 // Instancia de subrutina sin parametro
                 | USA ID[nombre] ':' estado[estado_1] '-' '>' estado[estado_2] ';' { 
                     if (!expandir_subrutina($nombre, 0, $estado_1, $estado_2)) error_semantico = 1; 
                 } ;

// Direccion del movimiento de la cabeza
movimiento: DER { $$ = 'D'; } 
          | IZQ { $$ = 'I'; } 
          | QUIETO { $$ = 'Q'; } ;

%%

// Punto de entrada: parsea y, si no hay errores, simula la maquina
int main(void) {
    inicializar_maquina();

    if (yyparse() != 0 || error_semantico) {
        fprintf(stderr, "Ejecucion abortada debido a errores semanticos o sintacticos.\n");
        return 1;
    }
    ejecutar();
    return 0;
}