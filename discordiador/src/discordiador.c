#include "discordiador.h"
#include "serializacion_discordiador.h"
#include "servidor_discordiador.h"
#include "tripulante.h"
#include "sabotajes_discordiador.h"

#define MODULE_NAME "Discordiador"
#define CONFIG_FILE_PATH "cfg/discordiador.cfg"
#define LOG_FILE_PATH "cfg/discordiador.log"

int id_patota;

const char *comandos_table[] = {"INICIAR_PATOTA",
								"INICIAR_PLANIFICACION",
								"PAUSAR_PLANIFICACION",
								"LISTAR_TRIPULANTES",
								"EXPULSAR_TRIPULANTE",
								"OBTENER_BITACORA",
								"FIN",
								"TEST_MENSAJES",
								"ALERTA_SABOTAJE",
								"COMPACTAR", NULL};

const char *estados_table[] = {"NEW",
							   "READY",
							   "EXEC",
							   "BLOCK_IO",
							   "BLOCK_EM",
							   "EXIT", NULL};

int main()
{

	id_patota = 0;
	aux_id_tripulante = 0;

	// La planificación comienza desactivada hasta tanto se inicie
	planificacion_activada = false;

	//enviar_tareas_miram("/home/utnso/tareas/tareasPatota5.txt",id_patota);
	//Inicio el log en un thread... :O
	miLogInitMutex(LOG_FILE_PATH, MODULE_NAME, false, LOG_LEVEL_INFO);
	miLogInfo("Inició Discordiador. \n");

	if (leer_config())
	{
		miLogInfo("Error al iniciar Discordiador: No se encontró el archivo de configuración");
		miLogDestroy();
		return EXIT_FAILURE;
	}

	puerto_discordiador = string_itoa(configuracion->puerto);

	//Obtención datos para conexion con miram
	char *ip_miram = configuracion->ip_mi_ram_hq;
	char *puerto_miram = configuracion->puerto_mi_ram_hq;

	miLogInfo("Conectándose a MiRAM");
	printf("\nInicia conexion con MIRAM:\n");

	socket_miram = iniciar_conexion_miram(ip_miram, puerto_miram);

	char *ip_store = configuracion->ip_i_mongo_store;
	char *puerto_store = configuracion->puerto_i_mongo_store;

	miLogInfo("Conectándose a Store");
	printf("\nInicia conexion con STORE:\n");

	socket_store = iniciar_conexion_store(ip_store, puerto_store);

	new_list = list_create();
	execute_list = list_create();
	blocked_io = list_create();
	exit_list = list_create();
	ready_list = list_create();
	blocked_em = list_create();

	tripulantes_totales = list_create();

	//inicializo semaforos
	//OJO! los semaforos mutex deben usar la biblioteca pthread

	sem_init(&semaforoEXEC, 0, configuracion->grado_multitarea);
	sem_init(&semaforoREADY, 0, 0);

	if (pthread_create(&threadSERVER_DISC, NULL, (void *)iniciar_servidor_discordiador,
					   NULL) != 0)
	{
		printf("Error iniciando servidor/n");
	}

	if (pthread_create(&threadCONSOLA_DISC, NULL, (void *)consola,
					   NULL) != 0)
	{
		printf("Error iniciando consola/n");
	}

	//pthread_join(threadSERVER_DISC, NULL);
	pthread_join(threadCONSOLA_DISC, NULL);

	miLogInfo("Finalizó Discordiador\n");
	//vaciar_listas();
	free(configuracion);
	miLogDestroy();

	sem_destroy(&mutexNEW);
	sem_destroy(&mutexREADY);
	sem_destroy(&semaforoEXEC);
	sem_destroy(&mutexBLOCK);
	sem_destroy(&mutexBLOCK_EM);
	sem_destroy(&mutexEXIT);
	sem_destroy(&mutexEXIT);
}

void consola()
{
	char *input_consola; // = "INICIAR_PATOTA 1 /home/utnso/tareas/tareasPatota5.txt 1|1";
	char **list;
	printf("Hola!\n");
	printf("Que desea hacer?\n");

	input_consola = readline(">>");
	string_trim(&input_consola);

	//*se podria poner ejemplitos de que puede hacer...
	while (strcmp(input_consola, "FIN") != 0)
	{
		printf("%s\n", input_consola);
		//*la primer linea debe ser la tarea. dependiendo esta chequeamos si va a tener mas tokens o no.
		list = string_split(input_consola, " ");

		Comandos opc = find_enum_consola(list[0]);
		switch (opc)
		{
		case INICIAR_PATOTA_COM:
			printf("Comando es Iniciar patota\n");
			if (!checkFileExists(list[2]))
			{
				printf("La ruta especificada es erronea\n");
				//free(input_consola);
				break;
			}
			else if (pthread_create(&threadPATOTA, NULL, (void *)iniciar_patota,
									(char *)input_consola) != 0)
			{
				printf("Error iniciando patota/n");
			}
			pthread_detach(threadPATOTA);
			
			break;
		case INICIAR_PLANIFICACION_COM:
			printf("Comando es Iniciar Planificacion\n");
			if (pthread_create(&threadINICIAR_PLANIFICACION, NULL, (void *)iniciar_planificacion,
							   NULL) != 0)
			{
				printf("Error iniciando planificacion/n");
			}
			free(input_consola);
			pthread_detach(threadINICIAR_PLANIFICACION);
			break;
		case PAUSAR_PLANIFICACION_COM:
			printf("Comando es Pausar Planificacion\n");
			if (pthread_create(&threadPAUSAR_PLANIFICACION, NULL, (void *)pausar_planificacion,
							   NULL) != 0)
			{
				printf("Error pausando planificacion\n");
			}
			free(input_consola);
			pthread_detach(threadPAUSAR_PLANIFICACION);
			break;
		case LISTAR_TRIPULANTE_COM:
			printf("\nComando es Listar Tripulantes\n");
			time_t current_time;
			char *c_time_string;

			current_time = time(NULL);

			/* Convert to local time format. */
			c_time_string = ctime(&current_time);

			printf("\n--------------------------------------------------------------------\n");
			printf("Estado de la Nave: %s", c_time_string);
			for (int i = 0; i < list_size(tripulantes_totales); i++)
			{
				Tripulante *tripu = (Tripulante *)list_get(tripulantes_totales, i);
				printf("Tripulante: %-5d  Patota: %-5d  Status: %-5s \n", tripu->id_tripulante,
					   tripu->id_patota,
					   estados_table[tripu->estado]);
			}
			printf("--------------------------------------------------------------------\n");
			free(input_consola);
			break;
		case EXPULSAR_TRIPULANTE_COM:
			printf("Comando es Expulsar Tripulante\n");
			if (pthread_create(&threadEXPULSAR_TRIPULANTE, NULL, (void *)expulsar_tripulante,
							   (char *)input_consola) != 0)
			{
				printf("Error expulsando tripulante\n");
			}
			pthread_detach(threadEXPULSAR_TRIPULANTE);
			break;
		case COMPACTACION_COM:
			printf("Comando es Compactacion\n");
			if (pthread_create(&threadCOMPACTACION, NULL, (void *)compactacion,
							   (char *)input_consola) != 0)
			{
				printf("Error compactacion\n");
			}
			pthread_detach(threadCOMPACTACION);
			break;
		case OBTENER_BITACORA_COM:
			printf("Comando es Obtener Bitacora\n");
			if (pthread_create(&threadOBTENERBITACORA, NULL, (void *)obtener_bitacora,
							   (char *)input_consola) != 0)
			{
				printf("Error obteniendo bitacora\n");
			}
			pthread_detach(threadOBTENERBITACORA);
			break;
		case ALERTA_SABOTAJE_COM:
			//atender_sabotaje(input_consola);
			break;
		case TEST_MENSAJES:
			printf("1. Miram\n");
			printf("2. Store\n");

			char test_modulo = readline(">>");

			if (test_modulo == "1")
			{
				consola_miram();
			}
			else
			{
				consola_store();
			}
			//free(input_consola);

			break;

		default:
			printf("No conozco ese comando, seguro que esta bien?\n");
			free(input_consola);
			break;
		}
		printf("Siguiente comando?\n");
		input_consola = readline(">>");
		string_trim(&input_consola);
		liberar_array(list);
	}
}

int checkFileExists(char *filename)
{
	if (fopen(filename, "r") == NULL)
	{
		return 0;
	}
	return 1;
}

void obtener_bitacora(char *comando)
{

	t_list *lista_mensajes = list_create();
	t_list *mensajes_respuesta;

	char **list;
	Tripulante *trip;

	list = string_split(comando, " ");

	char *tripulante = list[1]; //leemos el tripulante a pedir bitacora

	list_add(lista_mensajes, tripulante);

	t_paquete *paquete = crear_paquete(OBTENER_BITACORA);
	t_buffer *buffer;

	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;
	enviar_paquete(paquete, socket_store);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket_store);
	if (codigo_operacion == OK)
	{
		t_buffer *buffer_respuesta = (t_buffer *)recibir_buffer(socket_store);
		mensajes_respuesta = deserializar_lista_strings(buffer_respuesta);
		eliminar_buffer(buffer_respuesta);

		char *bitacora = string_new();
		string_append(&bitacora,(char*)list_get(mensajes_respuesta, 0));

		generar_archivo_bitacora(tripulante, bitacora);
		miLogInfo("Bitacora del tripulante %s obtenida correctamente", tripulante);
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("ERROR OBTENIENDO LA BITACORA DEL TRIPULANTE %s.", tripulante);
	}

	list_destroy(lista_mensajes);
	list_destroy_and_destroy_elements(mensajes_respuesta,free);
	liberar_array(list);
	free(comando);
}

void generar_archivo_bitacora(char *tripulante, char *bitacora)
{

	char *nombreArchivo = string_new();
	string_append(&nombreArchivo, "Bitacora_tripulante_");
	string_append(&nombreArchivo, tripulante);
	string_append(&nombreArchivo, ".bitacora");

	int tamanio = string_length(bitacora);

	FILE *bitacora_file = fopen(nombreArchivo, "w+");
	fwrite(bitacora, 1, tamanio, bitacora_file);
	fclose(bitacora_file);
	free(nombreArchivo);
	free(bitacora);
}

void expulsar_tripulante(char *comando)
{
	t_list *lista_mensajes = list_create();
	//char *string_tareas;
	//char *string_posiciones = string_new();
	char **list;
	Tripulante *trip;

	list = string_split(comando, " ");

	char *tripulante = list[1]; //leemos el tripulante a expulsar

	list_add(lista_mensajes, tripulante);

	for (int i = 0; i < list_size(tripulantes_totales); i++)
	{

		trip = list_get(tripulantes_totales, i);
		char* id_trip;
		id_trip = string_itoa(trip->id_tripulante);

		if (strcmp(tripulante, id_trip) == 0)
		{
			free(id_trip);
			break;
		}
		free(id_trip);
	}

	//Duermo al tripulante
	trip->tripulante_despierto = false;

	t_paquete *paquete = crear_paquete(EXPULSAR_TRIPULANTE);
	t_buffer *buffer;

	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;
	enviar_paquete(paquete, trip->socket_miram);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(trip->socket_miram);
	if (codigo_operacion == OK)
	{
		buffer = (t_buffer *)recibir_buffer(trip->socket_miram);
		miLogInfo("Tripulante %d expulsado correctamente\n", trip->id_tripulante);
		//Elimino al tripulante
		borrar_tripulante(trip);
		free(buffer->stream);
		free(buffer);
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("ERROR EXPULSANDO TRIPULANTE %d. \n", trip->id_tripulante);
	}

	list_destroy(lista_mensajes);
	free(comando);
	liberar_array(list);
}

void compactacion(char *comando)
{
	t_list *lista_mensajes = list_create(), *mensajes_respuesta = list_create();
	char *string_tareas;
	char *string_posiciones = string_new();
	char **list;

	list_add(lista_mensajes, "COMPACTACION"); //OJO CON ESTO -> NO SE PUEDE LIBERAR

	t_paquete *paquete = crear_paquete(COMPACTACION);
	t_buffer *buffer;

	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;
	enviar_paquete(paquete, socket_miram);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(socket_miram);
	if (codigo_operacion == OK)
	{
		buffer = (t_buffer *)recibir_buffer(socket_miram);
		eliminar_buffer(buffer);
		miLogInfo("Compactacion OK\n");
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("COMPACTACION CON ERROR. \n");
	}

	list_destroy(lista_mensajes);
	free(comando);
}

void iniciar_patota(char *comando)
{
	//INICIAR_PATOTA 5 /home/utnso/tareas/tareasPatota5.txt 1|1 3|4
	//pthread_t new_hilo_tripulante;

	miLogDebug("Comienza el INICIAR_PATOTA\n");

	t_list *lista_mensajes = list_create();
	t_list *mensajes_respuesta;
	Tripulante_disc trip_hilo;

	char *string_tareas;
	char *string_posiciones = string_new();
	char **list;

	list = string_split(comando, " ");
	string_tareas = leer_tareas_txt(list[2]); //leemos las tareas y las traemos como un solo string

	int cantidad_trip = atoi(list[1]);

	int size;
	for (size = 0; list[size] != NULL; size++)
		;

	for (int i = 0; i < cantidad_trip; i++)
	{ //string con todas las posiciones

		if ((size - 3) >= (i + 1))
		{
			string_append_with_format(&string_posiciones, "%s;", list[i + 3]);
		}
		else
		{
			string_append(&string_posiciones, "0|0;");
		}
	}

	list_add(lista_mensajes, strdup(list[1]));
	list_add(lista_mensajes, string_tareas);
	list_add(lista_mensajes, string_posiciones);
	mensajes_respuesta = iniciar_patota_miram(socket_miram, lista_mensajes);

	char *patota_id = list_get(mensajes_respuesta, 0);

	if (strcmp(patota_id, "-1") != 0)
	{

		pthread_t hilos_tripulantes[cantidad_trip];

		Tripulante *new_tripulante;

		for (int i = 0; i < cantidad_trip; i++)
		{
			new_tripulante = malloc(sizeof(Tripulante));

			new_tripulante->id_patota = atoi(patota_id);
			//new_tripulante->tarea_actual = proxima_tarea;
			new_tripulante->tripulante_despierto = false;
			new_tripulante->completo_tareas = false;
			new_tripulante->recibio_input_store = false;
			new_tripulante->realizo_movimientos_tarea = false;
			//new_tripulante->pos_x=atoi(list_get(mensajes_respuesta,1));
			//new_tripulante->pos_y=atoi(list_get(mensajes_respuesta,2));

			if (pthread_create(&hilos_tripulantes[i], NULL, inicializar_tripulante,
							   (Tripulante *)new_tripulante) != 0)
			{
				printf("Error inicializando tripulante/n");
			}
			else
			{
				new_tripulante->id_hilo = &hilos_tripulantes[i]; //le asigno el hilo a cada tripulante
			}
			pthread_detach(hilos_tripulantes[i]);
		}
	}
	else
	{
		miLogError("Pedido de INICIAR PATOTA denegado por MiRAM");
	}

	list_destroy_and_destroy_elements(lista_mensajes, (void *)char_destroy);
	list_destroy_and_destroy_elements(mensajes_respuesta,(void*)char_destroy);

	miLogDebug("Finaliza el INICIAR_PATOTA\n");

	free(comando);
	liberar_array(list);
	//pthread_exit(0);
}

void vaciar_listas()
{
	//! not so sure this will work running...
	void *list_value;
	Tripulante_disc *hilo_tripulante;
	for (int i = 0; list_size(new_list) < i; i++)
	{
		list_value = list_get(new_list, i);
		hilo_tripulante = (Tripulante_disc *)list_value;
		pthread_join(hilo_tripulante->id_hilo, NULL);
	}
	list_destroy(new_list);
	printf("Cerro hilo\n");
}

char *leer_tareas_txt(char *direccion_txt)
{
	// ejemplo de tarea TAREA PARAMETROS;POS X;POS Y;TIEMPO
	char *lista_tareas = string_new();
	char line[100];
	int index = 0;

	FILE *file_tarea = fopen(direccion_txt, "r");

	if (!feof(file_tarea))
	{
		fgets(line, sizeof(line), file_tarea);

		while (line[index] != '\n')
		{ //string con todas las posiciones

			if (line[index] != '\0')
			{
				string_append_with_format(&lista_tareas, "%c", line[index]);
				index++;
			}
			else
			{
				break;
			}
		}

		index = 0;
	}

	index = 0;
	while (!feof(file_tarea))
	{
		fgets(line, sizeof(line), file_tarea);

		index = 0;
		while (line[index] != '\n')
		{ //string con todas las posiciones

			if (index == 0)
			{
				string_append(&lista_tareas, "|");
			}

			if (line[index] != '\0')
			{
				string_append_with_format(&lista_tareas, "%c", line[index]);
				index++;
			}
			else
			{
				break;
			}
		}
	}

	string_append(&lista_tareas, "$");

	fclose(file_tarea);
	return lista_tareas;
}

void tripulante_listo(Tripulante *trip)
{
	//Se saca tripulante de cola de NEW y se pasa a cola de READY.
	pthread_mutex_lock(&mutexNEW);
	pthread_mutex_lock(&mutexREADY);
	list_add(ready_list, list_remove(new_list, 0));
	pthread_mutex_unlock(&mutexNEW);
	pthread_mutex_unlock(&mutexREADY);
	trip->estado = listo;
	miLogInfo("Se pasa al tripulante %d de la cola NEW a la cola READY\n", trip->id_tripulante);
}

int iniciar_conexion_miram(char *ip_destino, char *puerto_destino)
{

	int socket = crear_conexion(logger, ip_destino, puerto_destino);

	miLogInfo("Obtuve el socket y vale %d.\n", socket);

	if (socket == -1)
	{
		miLogInfo("No fue posible establecer la conexión del socket solicitado.\n");
		exit(3);
	}
	return socket;
}
void consola_miram()
{
	//iniciamos consola
	char *opcion_seleccionada;
	iniciar_consola_miram();
	opcion_seleccionada = readline(">>");

	while (opcion_invalida_miram(opcion_seleccionada))
	{
		printf("\nError! %s no es una opción correcta.\n", opcion_seleccionada);
		iniciar_consola_miram();
		opcion_seleccionada = readline(">>");
	}

	printf("\nOpcion seleccionada: %s \n", opcion_seleccionada);

	//obtenemos el codigo de la operacion seleccionada por el usuario
	op_code operacion_seleccionada;
	operacion_seleccionada = convertir_codigo_operacion_miram(opcion_seleccionada);

	//enviamos accion a socket
	enviar_accion_seleccionada(operacion_seleccionada, socket_miram);
}
int iniciar_conexion_store(char *ip_destino, char *puerto_destino)
{

	//inicia conexion con destino
	int socket = crear_conexion(logger, ip_destino, puerto_destino);

	miLogInfo("Obtuve el socket y vale %d.\n", socket);

	if (socket == -1)
	{
		miLogInfo("No fue posible establecer la conexión del socket solicitado.\n");
		exit(3);
	}
	return socket;
}
void consola_store()
{
	//iniciamos consola
	char *opcion_seleccionada;
	iniciar_consola_store();
	opcion_seleccionada = readline(">>");

	while (opcion_invalida_store(opcion_seleccionada))
	{
		printf("\nError! %s no es una opción correcta.\n", opcion_seleccionada);
		iniciar_consola_store();
		opcion_seleccionada = readline(">>");
	}

	printf("\nOpcion seleccionada: %s \n", opcion_seleccionada);

	//obtenemos el codigo de la operacion seleccionada por el usuario
	op_code operacion_seleccionada;
	operacion_seleccionada = convertir_codigo_operacion_store(opcion_seleccionada);

	//enviamos accion a socket
	enviar_accion_seleccionada(operacion_seleccionada, socket_store);
}
int leer_config(void)
{

	t_config *config;
	configuracion = malloc(sizeof(Configuracion));

	config = config_create(CONFIG_FILE_PATH);

	if (config == NULL)
	{
		return EXIT_FAILURE;
	}
	configuracion->puerto = config_get_int_value(config, "PUERTO");
	configuracion->ip_mi_ram_hq = strdup(config_get_string_value(config, "IP_MI_RAM_HQ"));
	configuracion->puerto_mi_ram_hq = strdup(config_get_string_value(config, "PUERTO_MI_RAM_HQ"));
	configuracion->ip_i_mongo_store = strdup(config_get_string_value(config, "IP_I_MONGO_STORE"));
	configuracion->puerto_i_mongo_store = strdup(config_get_string_value(config, "PUERTO_I_MONGO_STORE"));
	configuracion->grado_multitarea = config_get_int_value(config, "GRADO_MULTITAREA");
	configuracion->algoritmo = strdup(config_get_string_value(config, "ALGORITMO"));
	if (strncmp(configuracion->algoritmo, "RR", 2) == 0)
	{
		configuracion->quantum = config_get_int_value(config, "QUANTUM");
	}
	configuracion->duracion_sabotaje = config_get_int_value(config, "DURACION_SABOTAJE");
	configuracion->retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");

	config_destroy(config);
	return EXIT_SUCCESS;
}

void iniciar_consola_miram()
{
	printf("Mensajes posibles:\n\n");
	printf("1) Expulsar tripulante\n");
	printf("2) Iniciar tripulante\n");
	printf("3) Informar tareas de patota\n");
	printf("4) Mover tripulante\n");
	printf("5) Tarea siguiente\n");
	printf("\nSeleccione el numero correspondiente\n\n");
}

void iniciar_consola_store()
{
	printf("Mensajes posibles:\n\n");
	printf("1) Obtener bitacora\n");
	printf("2) Iniciar FSCK\n");
	printf("3) Generar oxigeno\n");
	printf("4) Enviar paquete de prueba\n");
	printf("\nSeleccione el numero correspondiente\n\n");
}

bool opcion_invalida_miram(char *opcion)
{
	return ((strncmp(opcion, "1", 1) != 0) && (strncmp(opcion, "2", 1) != 0) &&
			(strncmp(opcion, "3", 1) != 0) && (strncmp(opcion, "4", 1) != 0) &&
			(strncmp(opcion, "5", 1) != 0));
}

bool opcion_invalida_store(char *opcion)
{
	return ((strncmp(opcion, "1", 1) != 0) && (strncmp(opcion, "2", 1) != 0) &&
			(strncmp(opcion, "3", 1) != 0) && (strncmp(opcion, "4", 1) != 0));
}

bool modulo_invalido(char *opcion)
{
	return ((strncmp(opcion, "1", 1) != 0) && (strncmp(opcion, "2", 1) != 0));
}

op_code convertir_codigo_operacion_store(char *codigo)
{
	op_code codigo_operacion;

	if (strncmp(codigo, "1", 1) == 0)
	{
		codigo_operacion = OBTENER_BITACORA;
	}
	else if (strncmp(codigo, "2", 1) == 0)
	{
		codigo_operacion = FSCK;
	}
	else if (strncmp(codigo, "3", 1) == 0)
	{
		codigo_operacion = INFORMAR_TAREA;
	}
	else if (strncmp(codigo, "4", 1) == 0)
	{
		codigo_operacion = PAQUETE;
	}

	return codigo_operacion;
}

op_code convertir_codigo_operacion_miram(char *codigo)
{
	op_code codigo_operacion;

	if (strncmp(codigo, "1", 1) == 0)
	{
		codigo_operacion = EXPULSAR_TRIPULANTE;
	}
	else if (strncmp(codigo, "2", 1) == 0)
	{
		codigo_operacion = INICIAR_TRIPULANTE;
	}
	else if (strncmp(codigo, "3", 1) == 0)
	{
		codigo_operacion = INFORMAR_TAREAS_PATOTA;
	}
	else if (strncmp(codigo, "4", 1) == 0)
	{
		codigo_operacion = MOV_TRIPULANTE;
	}
	else if (strncmp(codigo, "5", 1) == 0)
	{
		codigo_operacion = TAREA_SIGUIENTE;
	}

	return codigo_operacion;
}

void elegir_modulo()
{
	printf("Elija el modulo con el que desea comunicarse:\n\n");
	printf("1) Mi-RAM\n");
	printf("2) I-Store\n");
	printf("\nSeleccione el numero correspondiente\n\n");
}

Comandos find_enum_consola(char *sval)
{
	Comandos result = INICIAR_PATOTA_COM; /* value corresponding to etable[0] */
	int i = 0;
	for (i = 0; comandos_table[i] != NULL; ++i, ++result)
		if (0 == strcmp(sval, comandos_table[i]))
			return result;
	return -1;
}

void obtener_tarea(char *tarea_str, Tarea *nueva_tarea)
{
	char *token;

	int cont = 0;
	char *parametros;
	char *pos_x;
	char *pos_y;
	char *tiempo;
	char **tarea;

	token = strtok(tarea_str, ";");

	while (token != NULL)
	{

		if (cont == 0)
		{
			parametros = strdup(token);
		}
		else if (cont == 1)
		{
			pos_x = strdup(token);
		}
		else if (cont == 2)
		{
			pos_y = strdup(token);
		}
		else
		{
			tiempo = strdup(token);
		}

		token = strtok(NULL, ";");

		cont++;
	}

	tarea = string_split(parametros," ");

	//nueva_tarea->nombre_tarea = strdup(strtok(parametros, " "));
	nueva_tarea->nombre_tarea = strdup(tarea[0]);
	if(tarea[1]!=NULL){
	  nueva_tarea->parametros = strdup(tarea[1]);
	}
	//printf(" nombre tarea: %s\n", nueva_tarea->nombre_tarea);
	//nueva_tarea->parametros = strdup(strtok(NULL, " "));
	//printf(" parametros: %s\n", nueva_tarea->parametros);
	nueva_tarea->pos_x = pos_x;
	//printf(" pos x: %s\n", nueva_tarea->pos_x);
	nueva_tarea->pos_y = pos_y;
	//printf(" pos y: %s\n", nueva_tarea->pos_y);
	char *tiempo_aux = strdup(strtok(tiempo, "|"));
	nueva_tarea->tiempo = atoi(tiempo_aux);
	//printf(" tiempo: %i\n", nueva_tarea->tiempo);
	free(parametros);
	//free(token);
	free(tiempo_aux);
	free(tiempo);
	liberar_array(tarea);
}

//Descripción: Planifica en una única vez, un tripulante: de listo a en ejecución
//Hecho por: Emiliano
int planificar()
{
	pthread_mutex_lock(&mutexEXEC);
	pthread_mutex_lock(&mutexREADY);

	Tripulante *tripulante;

	if (list_size(ready_list) > 0)
	{
		tripulante = (Tripulante *)list_get(ready_list, 0); //obtiene primero en cola de READY
		list_add(execute_list, tripulante);
		list_remove(ready_list, 0);
		miLogInfo("El tripulante: %d pasa de READY a EXEC\n", tripulante->id_tripulante);
		//aviso cambio de cola a MIRAM
		informar_cambio_de_cola_miram(tripulante, "EXEC");
		tripulante->estado = trabajando;
		tripulante->tripulante_despierto = true;
		if (strncmp(configuracion->algoritmo, "RR", 2) == 0)
		{
			tripulante->quantum = configuracion->quantum;
		}
		pthread_mutex_unlock(&tripulante->semaforo_trip);
		pthread_mutex_unlock(&mutexEXEC);
	    pthread_mutex_unlock(&mutexREADY);
	}
	else
	{
		miLogInfo("No existen más tripulantes en la cola de READY\n");
		pthread_mutex_unlock(&mutexEXEC);
	    pthread_mutex_unlock(&mutexREADY);
		return -1;
	}
}

void finalizar_tripulante(Tripulante *trip)
{
	//obtengo indice del tripulante en la cola de EXEC
	int indice;
	Tripulante *trip_auxiliar;
	bool tripulante_encontrado = false;

	t_list *lista_mensajes = list_create();
	char *tripulante = string_itoa(trip->id_tripulante);
	list_add(lista_mensajes, tripulante);
	t_paquete *paquete = crear_paquete(EXPULSAR_TRIPULANTE);
	t_buffer *buffer;
	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;
	enviar_paquete(paquete, trip->socket_miram);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(trip->socket_miram);
	if (codigo_operacion == OK)
	{
		buffer = (t_buffer *)recibir_buffer(trip->socket_miram);
		eliminar_buffer(buffer);
		trip->estado = finalizado;
		miLogInfo("Tripulante %d expulsado correctamente\n", trip->id_tripulante);
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("ERROR EXPULSANDO TRIPULANTE %d. \n", trip->id_tripulante);
	}

	list_destroy_and_destroy_elements(lista_mensajes,free);

	pthread_mutex_lock(&mutexEXEC);
	t_list_iterator* iterador_ejecucion = list_iterator_create(execute_list);
	while(list_iterator_has_next(iterador_ejecucion)){
		trip_auxiliar = list_iterator_next(iterador_ejecucion);
		if (trip->id_tripulante == trip_auxiliar->id_tripulante)
		{
			tripulante_encontrado = true;
			list_iterator_remove(iterador_ejecucion);
			break;
		}
	}
	list_iterator_destroy(iterador_ejecucion);
	pthread_mutex_unlock(&mutexEXEC);

	/* for (int i = 0; i < list_size(execute_list); i++)
	{

		trip_auxiliar = list_get(execute_list, i);

		if (trip->id_tripulante == trip_auxiliar->id_tripulante)
		{
			indice = i;
			tripulante_encontrado = true;
			break;
		}
	}*/

	//Antes de finalizar al tripulante, lo borro de la lista de tripulantes totales
	/*for(int i =0; i<list_size(tripulantes_totales);i++){

    Tripulante* tripu = list_get(tripulantes_totales,i);

    if(trip->id_tripulante == tripu->id_tripulante){
      list_remove(tripulantes_totales, i);
    }
  }*/
	if (tripulante_encontrado)
	{

		pthread_mutex_lock(&mutexEXIT);
		//empieza seccion critica
		list_add(exit_list, trip_auxiliar);
		miLogDebug("El tripulante %d se agrego a la cola de EXIT.", trip_auxiliar->id_tripulante);
		//finaliza seccion critica
		pthread_mutex_unlock(&mutexEXIT);

		trip_auxiliar->estado = finalizado;
		//libero lugar en la cola de EXEC
		sem_post(&semaforoEXEC);

		//libero recursos ocupados por el Hilo
		pthread_exit(0);
	}
}

bool tarea_informada(Tripulante *tripulante, char *nombre_tarea, char *parametro)
{

	if (!tripulante->tripulante_despierto)
	{
		pthread_mutex_lock(&tripulante->semaforo_trip);
	}

	bool se_informo = false;

	t_paquete *paquete = crear_paquete(INFORMAR_TAREA);
	t_buffer *buffer;

	char *id_auxiliar = string_itoa(tripulante->id_tripulante);
	char *mensaje_tarea = string_new();
	char *espacio = string_new();
	string_append(&espacio, " ");
	string_append(&mensaje_tarea, nombre_tarea);
	string_append(&mensaje_tarea, espacio);
	string_append(&mensaje_tarea, parametro);

	t_list *lista_mensajes = list_create();
	list_add(lista_mensajes, id_auxiliar);
	list_add(lista_mensajes, mensaje_tarea);

	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;

	enviar_paquete(paquete, tripulante->socket_store);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(tripulante->socket_store);
	if (codigo_operacion == OK)
	{
		//miLogInfo("Tarea informada correctamente\n");
		se_informo = true;
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("Tripulante %d: ERROR INFORMANDO TAREA A STORE. \n", tripulante->id_tripulante);
	}

	buffer = (t_buffer *)recibir_buffer(tripulante->socket_store);

	eliminar_buffer(buffer);
	list_destroy_and_destroy_elements(lista_mensajes,free);
	free(espacio);
	return se_informo;
}

void avisar_movimiento_miram(Tripulante *trip, char *eje)
{

	if (!trip->tripulante_despierto)
	{
		pthread_mutex_lock(&trip->semaforo_trip);
	}

	t_paquete *paquete = crear_paquete(MOV_TRIPULANTE);
	t_buffer *buffer;

	char* id_auxiliar = string_itoa(trip->id_tripulante);
	char* nueva_posicion;
	if (strncmp(eje, "X", 1) == 0)
	{
		nueva_posicion = string_itoa(trip->pos_x);
	}
	else
	{
		nueva_posicion = string_itoa(trip->pos_y);
	}
	//Lista de elementos a enviar a miram: 1. ID del tripulante, 2. Eje, 3. Nueva Posicion
	t_list* lista_mensajes = list_create();
	list_add(lista_mensajes, id_auxiliar);
	list_add(lista_mensajes, eje);
	list_add(lista_mensajes, nueva_posicion);

	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;

	enviar_paquete(paquete, trip->socket_miram);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(trip->socket_miram);
	if (codigo_operacion == OK)
	{
		buffer = (t_buffer *)recibir_buffer(trip->socket_miram);
		eliminar_buffer(buffer);
		//miLogInfo("Nueva posicion informada a MIRAM correctamente\n");
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("Tripulante %d: ERROR INFORMANDO NUEVA POSICION A MIRAM. \n", trip->id_tripulante);
	}

	//list_destroy_and_destroy_elements(lista_mensajes,(void*)char_destroy);
	list_destroy_and_destroy_elements(lista_mensajes,free);
}

void avisar_movimiento_bitacora(Tripulante *tripulante, char *origen, char *destino)
{

	if (!tripulante->tripulante_despierto)
	{
		pthread_mutex_lock(&tripulante->semaforo_trip);
	}

	t_paquete *paquete = crear_paquete(INFORMACION_BITACORA);
	t_buffer *buffer;
	char *id_trip = string_itoa(tripulante->id_tripulante);

	t_list *lista_mensajes = list_create();

	char *mensaje = string_new();
	string_append(&mensaje, "Se DESPLAZA el tripulante: ");
	string_append(&mensaje, id_trip);
	string_append(&mensaje, " desde: ");
	string_append(&mensaje, origen);
	string_append(&mensaje, " hasta: ");
	string_append(&mensaje, destino);

	//Se envia mensaje a Store:
	list_add(lista_mensajes, id_trip);
	list_add(lista_mensajes, mensaje);

	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;

	enviar_paquete(paquete, tripulante->socket_store);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(tripulante->socket_store);
	if (codigo_operacion == OK)
	{
		//miLogInfo("Nuevo desplazamiento informado a bitacora correctamente\n");
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("Tripulante %d: ERROR INFORMANDO NUEVA POSICION A BITACORA. \n", tripulante->id_tripulante);
	}

	buffer = (t_buffer *)recibir_buffer(tripulante->socket_store);

	//list_destroy_and_destroy_elements(lista_mensajes,(void*)char_destroy);
	list_destroy_and_destroy_elements(lista_mensajes,free);
	eliminar_buffer(buffer);
	//free(mensaje);
}

void avisar_inicio_tarea_bitacora(Tripulante *tripulante, char *tarea_nombre)
{

	if (!tripulante->tripulante_despierto)
	{
		pthread_mutex_lock(&tripulante->semaforo_trip);
	}

	t_paquete *paquete = crear_paquete(INFORMACION_BITACORA);
	t_buffer *buffer;

	t_list *lista_mensajes = list_create();
	char *id_trip = string_itoa(tripulante->id_tripulante);
	//Parametros que se envian a Store:
	list_add(lista_mensajes, id_trip);
	char *mensaje = string_new();
	string_append(&mensaje, "Comienza ejecucion de tarea: ");
	string_append(&mensaje, tarea_nombre);
	list_add(lista_mensajes, mensaje);

	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;

	enviar_paquete(paquete, tripulante->socket_store);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(tripulante->socket_store);
	if (codigo_operacion == OK)
	{
		//miLogInfo("Nueva tarea informada a bitacora correctamente\n");
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("Tripulante %d: ERROR INFORMANDO NUEVA TAREA A BITACORA. \n", tripulante->id_tripulante);
	}

	buffer = (t_buffer *)recibir_buffer(tripulante->socket_store);

	//list_destroy(lista_mensajes);
	eliminar_buffer(buffer);
	//free(mensaje);
	list_destroy_and_destroy_elements(lista_mensajes,free);
}

void avisar_fin_tarea_bitacora(Tripulante *tripulante, char *tarea_nombre)
{

	if (!tripulante->tripulante_despierto)
	{
		pthread_mutex_lock(&tripulante->semaforo_trip);
	}

	t_paquete *paquete = crear_paquete(INFORMACION_BITACORA);
	t_buffer *buffer;

	t_list *lista_mensajes = list_create();

	char *id_trip = string_itoa(tripulante->id_tripulante);

	//Parametros que se envian a Store:
	list_add(lista_mensajes, id_trip);
	char *mensaje = string_new();
	string_append(&mensaje, "Finaliza ejecucion de tarea: ");
	string_append(&mensaje, tarea_nombre);
	list_add(lista_mensajes, mensaje);

	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;

	enviar_paquete(paquete, tripulante->socket_store);
	free(id_trip);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(tripulante->socket_store);
	if (codigo_operacion == OK)
	{
		//miLogInfo("Fin de tarea informado a bitacora correctamente\n");
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("Tripulante %d: ERROR INFORMANDO FIN DE TAREA A BITACORA. \n", tripulante->id_tripulante);
	}

	buffer = (t_buffer *)recibir_buffer(tripulante->socket_store);

	list_destroy(lista_mensajes);
	eliminar_buffer(buffer);
	free(mensaje);
}

void informar_cambio_de_cola_miram(Tripulante *tripulante, char *nueva_cola)
{
	t_paquete *paquete = crear_paquete(CAMBIO_COLA);
	t_buffer *buffer;

	t_list *lista_mensajes = list_create();

	//Parametros que se envian a Miram:
	char *id_trip = string_itoa(tripulante->id_tripulante);
	list_add(lista_mensajes, id_trip);
	list_add(lista_mensajes, nueva_cola);

	buffer = serializar_lista_strings(lista_mensajes);
	paquete->buffer = buffer;

	enviar_paquete(paquete, tripulante->socket_miram);
	free(id_trip);

	//recibe respuesta de destino
	op_code codigo_operacion = recibir_operacion(tripulante->socket_miram);
	if (codigo_operacion == OK)
	{
		//miLogInfo("Cambio de estado/cola informado a miram.\n");
	}
	else if (codigo_operacion == FAIL)
	{
		miLogError("ERROR INFORMANDO CAMBIO DE ESTADO/COLA DE TRIPULANTE: %d A MIRAM. \n", tripulante->id_tripulante);
	}

	buffer = (t_buffer *)recibir_buffer(tripulante->socket_miram);

	list_destroy(lista_mensajes);
	eliminar_buffer(buffer);
}

void pasar_tripulante_de_exec_a_ready(Tripulante *trip)
{
	miLogDebug("Por verificar si tripulante %d dormido", trip->id_tripulante);
	if (!trip->tripulante_despierto)
	{
		pthread_mutex_lock(&trip->semaforo_trip);
	}
	int indice;
	Tripulante *trip_auxiliar;
	bool tripulante_encontrado = false;

	miLogDebug("Tripulante %d : Por hacer wait de EXEC", trip->id_tripulante);
	pthread_mutex_lock(&mutexEXEC); //esta bien?
	miLogDebug("Tripulante %d : Por hacer wait de READY", trip->id_tripulante);
	pthread_mutex_lock(&mutexREADY);

	for (int i = 0; i < list_size(execute_list); i++)
	{

		trip_auxiliar = list_get(execute_list, i);

		if (trip->id_tripulante == trip_auxiliar->id_tripulante)
		{
			indice = i;
			tripulante_encontrado = true;
			break;
		}
	}

	if (tripulante_encontrado)
	{
		miLogDebug("Tripulante %d : encontrado en EXEC", trip->id_tripulante);
		//empieza seccion critica
		//list_remove() devuelve el tripulante que se elimina de la lista
		trip_auxiliar = list_remove(execute_list, indice);
		list_add(ready_list, trip_auxiliar);
		miLogInfo("El tripulante: %d pasa de EXEC a READY\n", trip->id_tripulante);
		if (strncmp(configuracion->algoritmo, "RR", 2) == 0)
		{
			trip->quantum = configuracion->quantum;
		}
		trip->estado = listo;
		trip->tripulante_despierto = false;

		trip_auxiliar->estado = listo;
		//aviso cambio de cola a MIRAM
		informar_cambio_de_cola_miram(trip, "READY");

		//libero lugar en la cola de EXEC
		sem_post(&semaforoEXEC);
		sem_post(&semaforoREADY);

		//al liberarse un espacio en la cola de EXEC, replanifico
		//planificar();
	}
	//finaliza seccion critica
	pthread_mutex_unlock(&mutexEXEC);
	pthread_mutex_unlock(&mutexREADY);
}

void borrar_tripulante(Tripulante *trip)
{
	if (trip->estado == llegada)
	{
		pthread_mutex_lock(&mutexNEW);
		for (int i = 0; i < list_size(new_list); i++)
		{
			Tripulante *trip_aux = list_get(new_list, i);
			if (trip_aux->id_tripulante == trip->id_tripulante)
			{
				list_remove(new_list, i);
				list_add(exit_list, trip);
				trip->estado = finalizado;
				miLogInfo("Se elimina al tripulante: %d de la cola de NEW\n", trip->id_tripulante);
			}
		}
		pthread_mutex_unlock(&mutexNEW);
	}
	else if (trip->estado == listo)
	{
		pthread_mutex_lock(&mutexREADY);
		for (int i = 0; i < list_size(ready_list); i++)
		{
			Tripulante *trip_aux = list_get(ready_list, i);
			if (trip_aux->id_tripulante == trip->id_tripulante)
			{
				list_remove(ready_list, i);
				list_add(exit_list, trip);
				trip->estado = finalizado;
				miLogInfo("Se elimina al tripulante: %d de la cola de READY\n", trip->id_tripulante);
			}
		}
		pthread_mutex_unlock(&mutexREADY);
	}
	else if (trip->estado == trabajando)
	{
		pthread_mutex_lock(&mutexEXEC);
		for (int i = 0; i < list_size(execute_list); i++)
		{
			Tripulante *trip_aux = list_get(execute_list, i);
			if (trip_aux->id_tripulante == trip->id_tripulante)
			{
				list_remove(execute_list, i);
				list_add(exit_list, trip);
				trip->estado = finalizado;
				miLogInfo("Se elimina al tripulante: %d de la cola de EXEC\n", trip->id_tripulante);
				//libero lugar en la cola de EXEC
				sem_post(&semaforoEXEC);
			}
		}
		pthread_mutex_unlock(&mutexEXEC);
	}
	else if (trip->estado == bloqueado_io)
	{
		pthread_mutex_lock(&mutexBLOCK);
		for (int i = 0; i < list_size(blocked_io); i++)
		{
			Tripulante *trip_aux = list_get(blocked_io, i);
			if (trip_aux->id_tripulante == trip->id_tripulante)
			{
				list_remove(blocked_io, i);
				list_add(exit_list, trip);
				trip->estado = finalizado;
				miLogInfo("Se elimina al tripulante: %d de la cola de BLOCK_IO\n", trip->id_tripulante);
			}
		}
		pthread_mutex_unlock(&mutexBLOCK);
	}
	else if (trip->estado == bloqueado_emergencia)
	{
		pthread_mutex_lock(&mutexBLOCK_EM);
		for (int i = 0; i < list_size(blocked_em); i++)
		{
			Tripulante *trip_aux = list_get(blocked_em, i);
			if (trip_aux->id_tripulante == trip->id_tripulante)
			{
				list_remove(blocked_em, i);
				list_add(exit_list, trip);
				trip->estado = finalizado;
				miLogInfo("Se elimina al tripulante: %d de la cola de BLOCK_EM\n", trip->id_tripulante);
			}
		}
		pthread_mutex_unlock(&mutexBLOCK_EM);
	}
}

static void char_destroy(char *self)
{
	free(self);
}

void liberar_array(char** lista){
	int contador = 0;
	while(lista[contador]!=NULL){
		free(lista[contador]);
		contador++;
	}
	free(lista);
}