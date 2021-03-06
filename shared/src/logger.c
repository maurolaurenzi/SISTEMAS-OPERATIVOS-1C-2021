#include "logger.h"

pthread_mutex_t logMutex;


void miLogInitMutex(char* file, char* program_name, bool is_active_console, t_log_level level){
	pthread_mutex_init(&logMutex,NULL);
	logger = log_create(file, program_name, is_active_console, level);
}

void miLogDestroy(){
	log_destroy(logger);
	pthread_mutex_destroy(&logMutex);
}

void miLogInfo(const char* message_template, ...){
	pthread_mutex_lock(&logMutex);
	va_list arguments;
	va_start(arguments, message_template);
	char* message = string_from_vformat(message_template, arguments);
	log_info(logger,message);
	free(message);
	va_end(arguments);
	pthread_mutex_unlock(&logMutex);
}

void miLogDebug(const char* message_template, ...){
	pthread_mutex_lock(&logMutex);
	va_list arguments;
	va_start(arguments, message_template);
	char* message = string_from_vformat(message_template, arguments);
	log_debug(logger,message);
	free(message);
	va_end(arguments);
	pthread_mutex_unlock(&logMutex);
}

void miLogTrace(const char* message_template, ...){
	pthread_mutex_lock(&logMutex);
	va_list arguments;
	va_start(arguments, message_template);
	char* message = string_from_vformat(message_template, arguments);
	log_trace(logger,message);
	free(message);
	va_end(arguments);
	pthread_mutex_unlock(&logMutex);
}

void miLogWarning(const char* message_template, ...){
	pthread_mutex_lock(&logMutex);
	va_list arguments;
	va_start(arguments, message_template);
	char* message = string_from_vformat(message_template, arguments);
	log_warning(logger,message);
	free(message);
	va_end(arguments);
	pthread_mutex_unlock(&logMutex);
}

void miLogError(const char* message_template, ...){
	pthread_mutex_lock(&logMutex);
	va_list arguments;
	va_start(arguments, message_template);
	char* message = string_from_vformat(message_template, arguments);
	log_error(logger,message);
	free(message);
	va_end(arguments);
	pthread_mutex_unlock(&logMutex);
}

void loggear_lista_strings(t_list* lista){

	if(list_size(lista)!=0){
		log_info(logger, "El contenido de la lista es el siguiente\n");
		for(int i = 0; i < list_size(lista); i++){
			log_info(logger, "\tEl elemento %d es : %s\n", i+1, (char*)list_get(lista,i));
		}
	}else{
		log_info(logger, "Lista vacia\n");
	}
}

