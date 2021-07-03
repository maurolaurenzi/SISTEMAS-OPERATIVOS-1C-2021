#include "store.h"
#include "file_system.h"
#include "request_analyzer.h"

#define MODULE_NAME "I-Mongo-Store"
#define CONFIG_FILE_PATH "cfg/store.cfg"
#define LOG_FILE_PATH "store.log"

void signalHandler(int signal){
	
	switch(signal) {
		case SIGUSR1:
			miLogInfo("Me llegó la señal %d, atiendo el sabotaje", signal);	
			atenderSabotaje();	
			break;
		case SIGINT:
			miLogInfo("Se forzó el cierre del I-Mongo-Store.");
			miLogDestroy();
			finalizarFS();
			exit(130);	//Control+C
			break;
		default:
			break;
	}	
}

void syncPeriodico(void){

    int tamanioBlocks = cantidadBloques * tamanioBloque;

	while(1){
        sleep(configuracion->tiempoSincro);
        msync(punteroBlocks, tamanioBlocks, 0); //Fuerzo la actualización del bitmap en el archivo.
		miLogInfo("Se forzó la sincronizacion del archivo Blocks.ims a disco. La próxima sincronización será dentro de %d segundos.", configuracion->tiempoSincro);
    }   
}

int main(int argc, char* argv[]) {

	//Inicio el log en un thread... :O
	miLogInitMutex(LOG_FILE_PATH, MODULE_NAME, true, LOG_LEVEL_INFO);

	//Configuro el signal SIGUSR1 para iniciar el sabotaje.
	signal(SIGUSR1, signalHandler);
	signal(SIGINT, signalHandler);

	//Inicio el thread de msync para bloques periodico.
	pthread_t th_msync;
	pthread_create(&th_msync, NULL, &syncPeriodico, NULL);
	
	if(leerConfig()){
		miLogInfo("Error al iniciar I-Mongo-Store: No se encontró el archivo de configuración");
		miLogDestroy();
		return EXIT_FAILURE;
	}

	inicializarStore();


	miLogInfo("==== Finalizó I-Mongo-Store ====");
	free(configuracion);
	miLogDestroy();
	list_destroy(posicionesSabotaje);
	finalizarFS();
	
	return EXIT_SUCCESS;
}


int leerConfig(void){

	t_config* config;
	configuracion = malloc(sizeof(Configuracion));

	config = config_create(CONFIG_FILE_PATH);

	if(config==NULL){
		return EXIT_FAILURE;
	}

	configuracion->puntoMontaje = strdup(config_get_string_value(config, "PUNTO_MONTAJE"));
	configuracion->puerto = strdup(config_get_string_value(config, "PUERTO"));
	configuracion->tiempoSincro = config_get_int_value(config, "TIEMPO_SINCRONIZACION");
	configuracion->blockSizeDefault = config_get_int_value(config, "BLOCK_SIZE");
	configuracion->blocksQtyDefault = config_get_int_value(config, "BLOCKS");
	configuracion->posicionesSabotaje = strdup(config_get_string_value(config, "POSICIONES_SABOTAJE")); //POSICIONES_SABOTAJE=[1|1, 2|2, 3|3, 4|4, 5|5, 6|6, 7|7]	
	configuracion->ipDiscordiador = strdup(config_get_string_value(config, "IP_DISCORDIADOR"));
	configuracion->puertoDiscordiador = strdup(config_get_string_value(config, "PUERTO_DISCORDIADOR"));

	config_destroy(config);
	return EXIT_SUCCESS;
}

void inicializarStore(void){
	
	miLogInfo("==== Inició I-Mongo-Store =====");

	seContectoElDiscordiador = true;

	inicializarParametrosFS();

	if (!verificarFS()){
		miLogInfo("No se encontró un FILE SYSTEM válido, se procede a crearlo con los parámetros default");
		borrarTodosLosArchivos(puntoMontaje);
		crearArbolDirectorios();
		crearSuperbloque();
		crearBlocks();
		miLogInfo("FILE SYSTEM creado exitósamente");
	}

	leerSuperbloque();
	subirBlocksAMemoria();
	inicializarPosicionesSabotaje();
	

	levantar_servidor(atender_request_store, configuracion->puerto);
}

void inicializarParametrosFS(void){
	puntoMontaje = strdup(configuracion->puntoMontaje);
	pathSuperbloque = string_from_format("%s/%s", puntoMontaje, SUPERBLOQUE_FILE); //ej: /home/utnso/mnt/SuperBloque.ims
	pathBlocks = string_from_format("%s/%s", puntoMontaje, BLOCKS_FILE);
	pathFiles = string_from_format("%s/%s", puntoMontaje, "Files");
	pathBitacoras = string_from_format("%s/%s", puntoMontaje, "Files/Bitacoras");

	tamanioBloque = configuracion->blockSizeDefault;
	cantidadBloques = configuracion->blocksQtyDefault;
}

void inicializarPosicionesSabotaje(){
	
	posicionesSabotaje = list_create();
	
	t_list* posiciones = list_create();
	posiciones = obtenerListaSabotaje(configuracion->posicionesSabotaje);	

	PosicionSabotaje* posicionSabotaje;

	for(int i = 0; i < list_size(posiciones); i++){
		posicionSabotaje = malloc(sizeof(PosicionSabotaje));
		posicionSabotaje->posicion = (t_pos*) list_get(posiciones, i);
		posicionSabotaje->atendida = false;

		list_add(posicionesSabotaje, posicionSabotaje);		
	}
	//TEST
	/*for(int i = 0; i < list_size(posicionesSabotaje); i++){
		posicionSabotaje = (PosicionSabotaje*) list_get(posicionesSabotaje, i);	
		printf("x:%s, y:%s\n", string_itoa(posicionSabotaje->posicion->x), string_itoa(posicionSabotaje->posicion->y));
	}*/

	list_destroy(posiciones);
}

void iniciarConexionDiscordiador()
{	//inicia conexion con destino
	int socket = crear_conexion(logger, configuracion->ipDiscordiador, configuracion->puertoDiscordiador);
	miLogInfo("Obtuve el socket y vale %d.\n", socket);

	if (socket == -1) {
		miLogError("No fue posible establecer la conexión del socket solicitado.\n");
		exit(3);
	}
	socket_discordiador = socket;
}

void atenderSabotaje(){
	//1.Avisar a discordiador -> enviar posiciones de sabotaje (configuracion)
	op_code respuestaDiscordiador = enviarAvisoDeSabotaje(posicionesSabotaje); //esta en ejecucion_tareas
	
	//2.Esperar a que el Discordiador me active el protocolo fsck.
	if (respuestaDiscordiador == FSCK) {
		miLogInfo("\nSe da inicio al protocolo FSCK");
	//3.Llamar al protocolo de sabotaje
		protocoloFsck();

	} else if (respuestaDiscordiador == FAIL){
        miLogError("ERROR AL ENVIAR EL INICIO DEL PROTOCOLO FSCK. \n");
	}

	//TODO: Preguntarle a Mau si es necesario esto? para que es?
	//buffer = (t_buffer*)recibir_buffer(socket_discordiador);
}

void protocoloFsck(){
	//Inmediatamente despues que se recibe la llamada de FSCK
	/*
	pthread_mutex_lock(&lockStore);
	puedeEjecutar = 0;
	pthread_mutex_unlock(&lockStore);
	*/
	/* Otra opcion
	sem_wait(&sem_sabotajeSuperbloque);
	sem_wait(&sem_sabotajeBloque);
	*/
	//Analizar sabotaje en Superbloque
	verificarCantidadBloques();
	verificarBitmap();

	//Analizar sabotaje en files
	verificarSizeEnFile();
	verificarBlockCount();
	verificarBlocks();	
	/*
	sem_signal(&sem_sabotajeSuperbloque);
	sem_signal(&sem_sabotajeBloque);
	*/
	//Inmediatamente despues de que se haya ejecutado el protocolo.	
	/*
	pthread_mutex_lock(&lockStore);
	puedeEjecutar = 1;
	pthread_cond_signal(&condStore);
	pthread_mutex_unlock(&lockStore);*/
}

t_list* obtenerListaSabotaje(char* strPosicionesSabotaje){
	
	char** posicionesSabotaje = string_get_string_as_array(strPosicionesSabotaje);
	int largo = strlen(strPosicionesSabotaje) - 1;

	t_list* listaPosicionesSabotaje = list_create();

	t_pos* posicion;

	for(int i = 0;largo > 0; i++){
		char** strPosicion =  string_split(posicionesSabotaje[i], "|");
		posicion = malloc(sizeof(t_pos));
		posicion->x = atoi(strPosicion[0]);	
		posicion->y = atoi(strPosicion[1]);
		list_add(listaPosicionesSabotaje,(void*) posicion);

		largo-=3;
		largo-=strlen(strPosicion[0]);
		largo-=strlen(strPosicion[1]);
	}
	
	return listaPosicionesSabotaje;
}

