#ifndef DISCORDIADOR_H
#define DISCORDIADOR_H

//Shared libs
#include "shared_utils.h"
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "logger.h"
#include "paquete.h"

typedef struct Configuracion{
	char* puntoMontaje;
	int puerto;
	int tiempoSincro;
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

Configuracion* configuracion;

int leer_config(void);
void iniciar_consola();
bool opcion_invalida(char*);
op_code convertir_codigo_operacion(char*);
void iniciar_conexion(char* ip_destino, char* puerto_destino);

#endif