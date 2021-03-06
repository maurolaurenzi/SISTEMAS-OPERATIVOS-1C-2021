#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

typedef struct
{
	uint32_t PID;
	uint32_t Tareas;
} PCB;

typedef struct
{
	uint32_t TID;
	uint32_t pos_X;
	uint32_t pos_y;
	uint32_t proxima_instruccion;
	uint32_t PCB;
	char estado;
} TCB;

typedef struct Punto
{
	int pos_x;
	int pos_y;
} Punto;

typedef struct Identidad_grilla
{
	int TID;
	char identificador;
} Identidad_grilla;

void *MEMORIA;

int contador_patotas;
int contador_tripulantes;

#endif