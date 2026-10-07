#ifndef MAQUINA_H
#define MAQUINA_H

// Limites de cada estructura
#define MAX_ESTADOS 100
#define MAX_TRANS 100
#define MAX_CINTA 50
#define MAX_SUB 10
#define MAX_ALFABETO 30

typedef struct {
    char estado[100];
    char nuevo_estado[100];
    char leer;
    char escribir;
    char mov;      // 'D' derecha, 'I' izquierda, 'Q' quieto
} Transicion;

// Subrutina: bloque de transiciones reutilizable, param indica si recibe parametro
typedef struct {
    char nombre[30];
    int param;
    Transicion trans[MAX_TRANS];
    int ntrans;
} Subrutina;

// Maquina de Turing completa
typedef struct {
    char nombre[40];
    char estados[MAX_ESTADOS][100];
    char finales[MAX_ESTADOS][100];
    char alfabeto[MAX_ALFABETO];
    int nestados;
    int nfinales;
    int ntrans;
    int nalfabeto;
    char estado_actual[100];
    char cinta[MAX_CINTA];
    int cabeza;
    Transicion trans[MAX_TRANS];
} Maquina;

// Variables globales compartidas entre parser y simulador
extern Maquina m;
extern Subrutina subrutinas[MAX_SUB];
extern int nsubrutinas;
extern Subrutina sub_actual;   // subrutina que se esta definiendo

// Consultas
int estado_declarado(const char *s);
int es_final(const char *s);
int simbolo_en_alfabeto(char c);

// Construccion y validacion
int agregar_transicion(const char *estado, char c_alfabeto, const char *nuevo_estado, char escribir, char mov);
int expandir_subrutina(const char *nom_sub, int val_param, const char *est_origen, const char *est_destino);
int finalizar_declaracion_maquina(const char *nombre, const char *estado_actual, const char *entrada);
void inicializar_maquina(void);

// Agregados por el parser durante el parsing
void agregar_simbolo_alfabeto(char c);
void agregar_estado_maquina(const char *nombre);
void agregar_estado_final(const char *nombre);
void agregar_transicion_subrutina(const char *estado, char c_alfabeto, const char *nuevo_estado, char escribir, char mov);
void registrar_subrutina(const char *nombre, int tiene_param);

// Simulacion
void ejecutar(void);

#endif
