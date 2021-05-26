#include "tripulante.h"

void *inicializar_tripulante(void* tripulante_dato){
  Tripulante *tripulante = (Tripulante *)tripulante_dato;

  //1. Aviso a MIRAM que deseo iniciar, indicando a que patota pertenezco.
   t_paquete* paquete = crear_paquete(INICIAR_TRIPULANTE);
   t_buffer* buffer;
   
   char* id_patota = string_itoa(tripulante->id_patota);
   t_list* lista_mensajes = list_create();
   list_add(lista_mensajes,id_patota);
   buffer = serializar_lista_strings(lista_mensajes);
   paquete ->buffer = buffer;

   enviar_paquete(paquete, socket_miram);

   //recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket);
	if (codigo_operacion == OK) {

		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
		t_list* lista = deserializar_lista_strings(buffer);

		loggear_lista_strings(lista);

        tripulante->id_tripulante = atoi(list_get(lista,0));
        char** posicion = string_split(list_get(lista,1),"|");
        tripulante->pos_x = atoi(posicion[0]);
        tripulante->pos_y = atoi(posicion[1]);
		tripulante->tarea_actual = list_get(lista,2);

        miLogInfo("Se pasa el tripulante a la cola de NEW");

        sem_wait(&mutexNEW);
        list_add(new_list, tripulante);
        sem_post(&mutexNEW);

		miLogInfo("Tripulante iniciado correctamente");

        //2. Una vez realizadas las tareas de preparacion, aviso a Discordiador que estoy listo
        tripulante_listo(tripulante);
	} else {
        miLogInfo("ERROR: TRIPULANTE NO INICIADO \n");
	}

  //printf("De responderme q no nos conoce, decodifico nuestas tareas y se las mando\n");
  //printf("Si nos conoce, me guardo la tarea a realizr para cuando el discordiador me mande a EXEC\n"); 
  //and others... but you get it
  //un mutex hasta q estoy en exec?

return;
}

void execute(Tripulante tripulante){

}