#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H

//Shared libs
#include "shared_utils.h"
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stdlib.h>
#include <commons/string.h>
#include<commons/collections/list.h>
//#include "servidor_discordiador.h"
#include "logger.h"
#include "paquete.h"
#include<semaphore.h>
#include<time.h>

typedef struct Configuracion{
	int puerto;
	char* ip_mi_ram_hq;
	int puerto_mi_ram_hq;
	char* ip_i_mongo_store;
	int puerto_i_mongo_store;
	int grado_multitarea;
	char* algoritmo;
	int quantum;
	int duracion_sabotaje;
	int retardo_ciclo_cpu;
} Configuracion;

typedef enum Estados{
	llegada,//?
	listo,
	trabajando,
	bloqueado_io,
	bloqueado_emergencia,
	finalizado,//?
}Estados;

typedef enum Comandos{	
	INICIAR_PATOTA_COM,
	INICIAR_PLANIFICACION_COM,
	PAUSAR_PLANIFICACION_COM,
	LISTAR_TRIPULANTE_COM,
	EXPULSAR_TRIPULANTE_COM,
	OBTENER_BITACORA_COM,
	FIN,
	TEST_MENSAJES,
	ALERTA_SABOTAJE_COM,
	COMPACTACION_COM
}Comandos;

	
Comandos find_enum_consola(char *sval);

typedef struct Tarea{
	char* nombre_tarea;
	char* parametros;
	char* pos_x;
	char* pos_y;
	int tiempo;
}Tarea;
 
typedef struct Tripulante_disc{
	pthread_t * id_hilo;
	Estados estado_trip;
	int quantum;
} Tripulante_disc;

typedef struct Tripulante{
    int id_patota;
    int id_tripulante;
    int pos_x;
    int pos_y;
    Estados estado;
	Tarea* tarea_actual;
	pthread_t* id_hilo;
	pthread_mutex_t semaforo_trip; //este semaforo indica si el tripulante puede ejecutar
	bool tripulante_despierto; //indica cuando está despierto el tripulante, en caso de estar dormido no puede ejecutar
    bool completo_tareas;
	int quantum;
	int ciclos_restantes; //para saber cuantos ciclos de CPU le faltan para completar una tarea especifica
	bool realizo_movimientos_tarea; //en caso de interrupcion, para saber donde retomar
	bool recibio_input_store; //en caso de interrupcion, para saber donde retomar
	bool aviso_inicio_tarea_store; 
	int socket_miram;
	int socket_store;
	Estados estado_anterior;
	bool en_sabotaje;
	pthread_mutex_t ciclos_IO; //este semaforo indica si el tripulante puede seguir consumiendo ciclos IO (por sabotaje)
} Tripulante;

Configuracion* configuracion;
//Listas
t_list * new_list;
t_list * execute_list;
t_list * blocked_io;
t_list* blocked_em;
t_list* exit_list;
t_list* ready_list;

t_list* tripulantes_totales;

//Semaforos
sem_t semaforoEXEC; //semaforo contador para controlar grado de multiprocesamiento
sem_t semaforoREADY;

pthread_mutex_t mutexNEW;
pthread_mutex_t mutexEXEC;
pthread_mutex_t mutexBLOCK;
pthread_mutex_t mutexEXIT;
pthread_mutex_t mutexREADY;
pthread_mutex_t mutexBLOCK_EM;
pthread_mutex_t mutexIO;

pthread_t threadSERVER_DISC;
pthread_t threadCONSOLA_DISC;

pthread_t *threadPATOTA;
pthread_t *threadINICIAR_PLANIFICACION;
pthread_t *threadPAUSAR_PLANIFICACION;
pthread_t *threadEXPULSAR_TRIPULANTE;
pthread_t *threadCOMPACTACION;
pthread_t *threadOBTENERBITACORA;

//Metodos mensajes
int leer_config(void);
int iniciar_conexion_miram(char* ip_destino, char* puerto_destino);
int iniciar_conexion_store(char* ip_destino, char* puerto_destino);
int socket_miram;
int socket_store;
char* puerto_discordiador;

int aux_id_tripulante;

bool planificacion_activada;


//Metodos Discordidor
void iniciar_patota(char* ); //?string? chequer commons
int checkFileExists(char* filename);
void expulsar_tripulante(char* );
void obtener_bitacora(char*);
void generar_archivo_bitacora(char*, char*);
void compactacion(char* );
void consola();
char* leer_tareas_txt(char* direccion_txt);
void mandar_tareas_miram(char *);
int enviar_tareas_miram(char* direccion_txt);
void tripulante_listo(Tripulante*);
void obtener_tarea(char* tarea_string, Tarea* nueva_tarea); //dado un string con los parametros de tarea, genera la estructura Tarea
void iniciar_planificacion();
void pausar_planificacion();
void finalizar_tripulante(Tripulante* trip); //Tripulante pasa de EXEC a EXIT, se liberan sus recursos
bool tarea_informada(Tripulante* trip, char* nombre_tarea, char* parametro);
void avisar_movimiento_miram(Tripulante* trip, char* eje);
void avisar_movimiento_bitacora(Tripulante* trip, char* origen, char* destino);
void avisar_inicio_tarea_bitacora(Tripulante* trip, char* tarea_nombre);
void avisar_fin_tarea_bitacora(Tripulante* trip, char* tarea_nombre);
void informar_cambio_de_cola_miram(Tripulante* tripulante, char* nueva_cola);
void pasar_tripulante_de_exec_a_ready(Tripulante* trip);
void borrar_tripulante(Tripulante* trip);
//Metodos Test
void elegir_modulo();
void consola_miram();
void consola_store();
void iniciar_consola_miram();
void iniciar_consola_store();
bool opcion_invalida_miram(char*);
bool opcion_invalida_store(char*);
bool modulo_invalido(char*);
op_code convertir_codigo_operacion_miram(char*);
op_code convertir_codigo_operacion_store(char*);

static void char_destroy(char* self);
void liberar_array(char** lista);

#endif