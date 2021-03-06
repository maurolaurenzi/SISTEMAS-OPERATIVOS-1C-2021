#ifndef SABOTAJES_DISCORDIADOR_H
#define SABOTAJES_DISCORDIADOR_H

//Shared libs#include "shared_utils.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "logger.h"
#include "paquete.h"
#include <pthread.h>
//#include <commons/string.h>
#include <commons/collections/list.h>
#include "discordiador.h"
#include "planificador.h"
#include "tripulante.h"
#include "servidor.h"

pthread_t threadSABOTAJE;

void *iniciar_servidor_discordiador();
void atender_request_discordiador(Request *request);
void atender_sabotaje(char* posicion);
double raiz_cuadrada(int numero); //no puedo usar sqrt...
void mover_tripulante_a_sabotaje(Tripulante* trip, int x_destino, int y_destino);
void enviar_fsck(Tripulante*);
void avisar_mov_sabotaje_miram(Tripulante* trip, char* eje);
void avisar_mov_sabotaje_bitacora(Tripulante* tripulante, char* origen, char* destino);


#endif