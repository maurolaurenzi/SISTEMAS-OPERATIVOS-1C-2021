#ifndef PAQUETE_H_
#define PAQUETE_H_

#include "buffer.h"

typedef enum
{
	MENSAJE,
	PAQUETE,
	EXPULSAR_TRIPULANTE,
	INICIAR_PATOTA,
	OBTENER_BITACORA,
	INICIAR_TRIPULANTE,
	MOV_TRIPULANTE,
	TAREA_SIGUIENTE,
	INFORMAR_TAREA,//discor->store
	INFORMACION_BITACORA,//discor->store - 
	FSCK,
	ALERTA_SABOTAJE,
	SIN_POS_SABOTAJE,
	INFORMAR_TAREAS_PATOTA,//Borrar luego
	COMPACTACION,
	CAMBIO_COLA,
    OK = 20,
	FAIL = 21,	
} op_code;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_paquete* crear_paquete(op_code);
void agregar_a_paquete(t_paquete*, char*);
t_buffer* serializar_paquete(t_paquete*);
void eliminar_paquete(t_paquete*);

#endif