#include "serializacion_discordiador.h"

void enviar_accion_seleccionada(op_code codigo_operacion, uint32_t socket){

	switch (codigo_operacion){
		case EXPULSAR_TRIPULANTE:
			printf("Entre al case de op_code %i.\n", codigo_operacion);

			expulsar_tripulante(socket);

			break;
		case INICIAR_TRIPULANTE:
			printf("Entre al case de op_code %i.\n", codigo_operacion);

			//iniciar_tripulante(socket);

			break;
		case INFORMAR_TAREAS_PATOTA:
			printf("Entre al case de op_code %i.\n", codigo_operacion);

			informar_tareas_patota_test(socket);

			break;
		case MOV_TRIPULANTE:
			printf("Entre al case de op_code %i.\n", codigo_operacion);

			//mover_tripulante(socket);

			break;
		case TAREA_SIGUIENTE:
			printf("Entre al case de op_code %i.\n", codigo_operacion);

			tarea_siguiente(socket);

			break;
		case PAQUETE:
		  printf("Entre al case de op_code %i.\n", codigo_operacion);

		  paquete_de_prueba(socket);

		  break;
		case OBTENER_BITACORA:
		  printf("Entre al case de op_code %i.\n", codigo_operacion);

		  obtener_bitacora(socket);

		  break;

		case FSCK:
		  printf("Entre al case de op_code %i.\n", codigo_operacion);

		  iniciar_fsck(socket);

		  break;

		case INFORMAR_TAREA:
		  printf("Entre al case de op_code %i.\n", codigo_operacion);

		  generar_oxigeno(socket);

		  break;
		
		default:
				printf("CODIGO DE OPERACION INCORRECTO");
			break;
	}
    close(socket);
}


void iniciar_tripulante(uint32_t socket){
    printf("Funcion de INICIAR TRIPULANTE \n");

    t_paquete* paquete = crear_paquete(INICIAR_TRIPULANTE);
	t_buffer* buffer;

    char* id_tripulante;
    printf("\nPor favor, ingrese el ID del tripulante a iniciar: \n");
    id_tripulante = readline(">>");

    char* id_patota;
    printf("\nPor favor, ingrese el ID de la patota a la que pertenece el tripulante: \n");
    id_patota = readline(">>");
    // int id_seleccionado = atoi(id_tripulante);
	// buffer = serializar_numero(id_seleccionado);

    t_list* lista_mensajes = list_create();
    list_add(lista_mensajes,id_tripulante);
    list_add(lista_mensajes,id_patota);
    buffer = serializar_lista_strings(lista_mensajes);
	paquete ->buffer = buffer;
	enviar_paquete(paquete, socket);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket);
	if (codigo_operacion == OK) {

		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
		t_list* lista = deserializar_lista_strings(buffer);

		loggear_lista_strings(lista);

		list_destroy(lista);

		eliminar_buffer(buffer);
		
		miLogInfo("Recibi los mensajes del destino correctamente");
	} else {
		miLogInfo("No recibi los mensajes del destino correctamente");
	}
}

t_list* iniciar_patota_miram(uint32_t socket, t_list* lista_mensajes ){

    t_paquete* paquete = crear_paquete(INICIAR_PATOTA);
	t_buffer* buffer;
    buffer = serializar_lista_strings(lista_mensajes);
	paquete ->buffer = buffer;
	enviar_paquete(paquete, socket);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket);
	if (codigo_operacion == OK) {

		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
		t_list* lista = deserializar_lista_strings(buffer);

		//loggear_lista_strings(lista);

		eliminar_buffer(buffer);

		miLogInfo("Se inicio patota en MIRAM correctamente \n");
		return lista;	
		
	} else {
		miLogError("No se inicio patota en MIRAM correctamente \n");
		t_list* lista = list_create();
		char* numero = string_new();
		string_append(&numero,"-1");
		list_add(lista,numero);
		return lista;
	}
}
void informar_tareas_patota_test(uint32_t socket){

    t_paquete* paquete = crear_paquete(INFORMAR_TAREAS_PATOTA);
	t_buffer* buffer;

    char* id_patota;
    printf("\nPor favor, ingrese el ID de la patota: \n");
    id_patota = readline(">>");

    char* nombre_tarea;
    printf("\nPor favor, ingrese el nombre de la tarea: \n");
    nombre_tarea = readline(">>");

    char* parametros;
    printf("\nPor favor, ingrese los parametros: \n");
    parametros = readline(">>");

    char* pos_x;
    printf("\nPor favor, ingrese la posicion X: \n");
    pos_x = readline(">>");

    char* pos_y;
    printf("\nPor favor, ingrese la posicion Y: \n");
    pos_y = readline(">>");

    char* tiempo;
    printf("\nPor favor, ingrese el tiempo: \n");
    tiempo = readline(">>");

    t_list* lista_mensajes = list_create();
    list_add(lista_mensajes,id_patota);
    list_add(lista_mensajes,nombre_tarea);
    list_add(lista_mensajes,parametros);
    list_add(lista_mensajes,pos_x);
    list_add(lista_mensajes,pos_y);
    list_add(lista_mensajes,tiempo);
    buffer = serializar_lista_strings(lista_mensajes);
	paquete ->buffer = buffer;
	enviar_paquete(paquete, socket);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket);
	if (codigo_operacion == OK) {

		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
		t_list* lista = deserializar_lista_strings(buffer);

		loggear_lista_strings(lista);

		list_destroy(lista);

		eliminar_buffer(buffer);
		
		miLogInfo("Recibi los mensajes del destino correctamente");
	} else {
		miLogInfo("No recibi los mensajes del destino correctamente");
	}
}

// void mover_tripulante(uint32_t socket){
//     printf("\nFuncion de MOVER TRIPULANTE");

//     t_paquete* paquete = crear_paquete(MOV_TRIPULANTE);
// 	t_buffer* buffer;

//     char* id_tripulante;
//     printf("\nPor favor, ingrese el ID del tripulante: \n");
//     id_tripulante = readline(">>");

//     char* pos_x;
//     printf("\nPor favor, ingrese la posicion X: \n");
//     pos_x = readline(">>");

//     char* pos_y;
//     printf("\nPor favor, ingrese la posicion Y: \n");
//     pos_y = readline(">>");

//     t_list* lista_mensajes = list_create();
//     list_add(lista_mensajes,id_tripulante);
//     list_add(lista_mensajes,pos_x);
//     list_add(lista_mensajes,pos_y);
//     buffer = serializar_lista_strings(lista_mensajes);
// 	paquete ->buffer = buffer;
// 	enviar_paquete(paquete, socket);

// 	//recibe respuesta de destino
// 	op_code codigo_operacion = recibir_operacion(socket);
// 	if (codigo_operacion == OK) {

// 		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
// 		t_list* lista = deserializar_lista_strings(buffer);

// 		loggear_lista_strings(lista);

// 		list_destroy(lista);

// 		eliminar_buffer(buffer);
		
// 		miLogInfo("Recibi los mensajes del destino correctamente");
// 	} else {
// 		miLogInfo("No recibi los mensajes del destino correctamente");
// 	}
// }

void tarea_siguiente(uint32_t socket){
    printf("\nFuncion de TAREA SIGUIENTE");

    t_paquete* paquete = crear_paquete(TAREA_SIGUIENTE);
	t_buffer* buffer;

    char* id_tripulante;
    printf("\nPor favor, ingrese el ID del tripulante: \n");
    id_tripulante = readline(">>");

    t_list* lista_mensajes = list_create();
    list_add(lista_mensajes,id_tripulante);
    buffer = serializar_lista_strings(lista_mensajes);
	paquete ->buffer = buffer;
	enviar_paquete(paquete, socket);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket);
	if (codigo_operacion == OK) {

		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
		t_list* lista = deserializar_lista_strings(buffer);

		loggear_lista_strings(lista);

		list_destroy(lista);

		eliminar_buffer(buffer);
		
		miLogInfo("Recibi los mensajes del destino correctamente");
	} else {
		miLogInfo("No recibi los mensajes del destino correctamente");
	}
}

void paquete_de_prueba(uint32_t socket){
	printf("\nFuncion de PAQUETE DE PRUEBA");

    t_paquete* paquete = crear_paquete(PAQUETE);
	t_buffer* buffer;

    char* mensaje_1;
    printf("\nPor favor, ingrese un mensaje generico: \n");
    mensaje_1 = readline(">>");

    t_list* lista_mensajes = list_create();
    list_add(lista_mensajes,mensaje_1);
    buffer = serializar_lista_strings(lista_mensajes);
	paquete ->buffer = buffer;
	enviar_paquete(paquete, socket);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket);
	if (codigo_operacion == OK) {

		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
		t_list* lista = deserializar_lista_strings(buffer);

		loggear_lista_strings(lista);

		list_destroy(lista);

		eliminar_buffer(buffer);
		
		miLogInfo("Recibi los mensajes del destino correctamente");
	} else {
		miLogInfo("No recibi los mensajes del destino correctamente");
	}
}

void obtener_bitacora_OLD(uint32_t socket){
	printf("\nFuncion de OBTENER BITACORA");

    t_paquete* paquete = crear_paquete(OBTENER_BITACORA);
	t_buffer* buffer;

    char* id_tripulante;
    printf("\nPor favor, ingrese el ID del tripulante: \n");
    id_tripulante = readline(">>");

    t_list* lista_mensajes = list_create();
    list_add(lista_mensajes,id_tripulante);
    buffer = serializar_lista_strings(lista_mensajes);
	paquete ->buffer = buffer;
	enviar_paquete(paquete, socket);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket);
	if (codigo_operacion == OK) {

		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
		t_list* lista = deserializar_lista_strings(buffer);

		loggear_lista_strings(lista);

		list_destroy(lista);

		eliminar_buffer(buffer);
		
		miLogInfo("Recibi los mensajes del destino correctamente");
	} else {
		miLogInfo("No recibi los mensajes del destino correctamente");
	}
}

void iniciar_fsck(uint32_t socket){
	printf("Funcion de INICIAR FSCK \n");

    t_paquete* paquete = crear_paquete(FSCK);
	t_buffer* buffer;
	paquete ->buffer = buffer;
	enviar_paquete(paquete, socket);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket);
	if (codigo_operacion == OK) {

		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
		t_list* lista = deserializar_lista_strings(buffer);

		loggear_lista_strings(lista);

		list_destroy(lista);

		eliminar_buffer(buffer);
		
		miLogInfo("Se inicio FSCK en STORE correctamente");
	} else {
		miLogInfo("No se inicio FSCK en STORE correctamente");
	}
}

void generar_oxigeno(uint32_t socket){
    printf("\nFuncion de GENERAR OXIGENO");

    t_paquete* paquete = crear_paquete(INFORMAR_TAREA);
	t_buffer* buffer;

    char* cantidad;
    printf("\nPor favor, ingrese la cantidad: \n");
    cantidad = readline(">>");

    char* pos_x;
    printf("\nPor favor, ingrese la posicion X: \n");
    pos_x = readline(">>");

    char* pos_y;
    printf("\nPor favor, ingrese la posicion Y: \n");
    pos_y = readline(">>");

    char* tiempo;
    printf("\nPor favor, ingrese el tiempo: \n");
    tiempo = readline(">>");

    t_list* lista_mensajes = list_create();
    list_add(lista_mensajes,cantidad);
    list_add(lista_mensajes,pos_x);
    list_add(lista_mensajes,pos_y);
    list_add(lista_mensajes,tiempo);
    buffer = serializar_lista_strings(lista_mensajes);
	paquete ->buffer = buffer;
	enviar_paquete(paquete, socket);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket);
	if (codigo_operacion == OK) {

		t_buffer* buffer = (t_buffer*)recibir_buffer(socket);
		t_list* lista = deserializar_lista_strings(buffer);

		loggear_lista_strings(lista);
		
		list_destroy(lista);

		eliminar_buffer(buffer);

		miLogInfo("Recibi los mensajes del destino correctamente");
	} else {
		miLogInfo("No recibi los mensajes del destino correctamente");
	}
}