#include "ejecucion_tareas.h"

void generarRecursos(tipoRecurso recurso, int cantidad){

    vericarMetadata(recurso); //Verifico si existe la metadata del recurso, sino lo creo. Por ej: Oxigeno.ims


//-------------------------------------------------APERTURA DE ARCHIVOS-----------------------------------------------------------------------------

    int archivoBlocks = open("Blocks.ims",O_RDWR); //abro el archivo blocks en modo lectura/escritura

    if (archivoBlocks == -1) {
        errExit("open"); //fallo apertura de archivo
    }

    int tamanioDeBlocks = calcularTamañoBlocks(); //Calcula el tamaño de Block.ims, tomando los datos del archivo SuperBloque y haciendo Block_size * Blocks

    int copiaArchivoBlocks = open("copia", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); //creo un archivo nuevo

    if (copiaArchivoBlocks == -1){
        errExit("open"); //fallo apertura de archivo
    }
    
    if (ftruncate(copiaArchivoBlocks, tamanioDeBlocks) == -1){ //trunco la copia del archivo al tamaño de blocks.
        errExit("ftruncate"); //fallo recorte de archivo copia
    }

    
    int archivoMetadata = open(obtenerMetadata(recurso), O_RDWR); //Abro el archivo de metadata del recurso
    

//-----------------------------------------------MAPEO DE ARCHIVOS A MEMORIA-----------------------------------------------------------------------
    
    char *punteroBloque = mmap(0, tamanioDeBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, archivoBlocks, 0); //mapeo el archivo Blocks a memoria
    //char * mmap (void *address, size_t length, int protect, int flags, int filedes, off_t offset)
    
    if (punteroBloque == MAP_FAILED){
        errExit("mmap"); //fallo mapeo de archivo
    }
    
    char *punteroBloqueCopia = mmap(NULL, tamanioDeBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, copiaArchivoBlocks, 0); //mapeo el archivo vacio a memoria
    
    if (punteroBloqueCopia == MAP_FAILED){
        errExit("mmap"); //fallo mapeo de archivo
    }

    memcpy(punteroBloqueCopia, punteroBloque, tamanioDeBlocks);  //copio el contenido del archivo Blocks en el archivo vacio creado

            

    //---------------------------------------------------ESCRITURA DE DATOS------------------------------------------------------------------------------------

    int posicionDisponible = buscarposicionDisponible(archivoMetadata); //busco si tengo un lugar disponible y devuelvo la posición
    
    agregarRecurso(punteroBloqueCopia, recurso, posicionDisponible, cantidad, archivoMetadata); //agrego la cantidad de caracteres en el archivo


    //---------------------------------------------CIERRE, LIBERACION DE MEMORIA Y SINCRONIZACION---------------------------------------------------------
    
    close(archivoBlocks); 
    close(copiaArchivoBlocks);
    close(archivoMetadata); 
        

    if (msync(punteroBloqueCopia, tamanioDeBlocks, MS_SYNC) == -1){ //Sincronizo la copia del archivo bloque a disco
        errExit("msync"); //Fallo la sincronizacion
    }

    if (munmap(punteroBloqueCopia, tamanioDeBlocks) == -1){ //desmapeo la copia del archivo bloque
        errExit("munmap");//Fallo el desmapeo de copia
    }

    if (munmap(punteroBloque, tamanioDeBlocks) == -1){ //desmapeo el archivo bloque
        errExit("munmap");//Fallo el desmapeo de original
    }

}

void agregarRecurso(char * punteroBloqueCopia, tipoRecurso recurso, int posicionDisponible, int cantidad, int archivoMetadata){

    switch(recurso){
        
        case OXIGENO: 

            llenarDeCaracteres(punteroBloqueCopia, posicionDisponible, "O", cantidad, archivoMetadata); 
            break;

        case BASURA:

            llenarDeCaracteres(punteroBloqueCopia, posicionDisponible, "B", cantidad, archivoMetadata);
            break;
        
        case COMIDA:

            llenarDeCaracteres(punteroBloqueCopia, posicionDisponible, "C", cantidad, archivoMetadata);
            break;
        
        default:  break;
    }
}

void llenarDeCaracteres(char * punteroBloqueCopia, int posicionDisponible, char * caracterDeLlenado, int cantidadCaracteres, int archivoMetadata){
    
    int cantEspaciosLibresEnBloque = cantPosicionesLibresEn(posicionDisponible);

    for(int i = 1; i<=cantEspaciosLibresEnBloque; i++){

        punteroBloqueCopia[posicionDisponible] = caracterDeLlenado; //Agrega el caracter deseado al lugar apuntado disponible
        posicionDisponible++;

    }

    if(cantidadCaracteres > cantEspaciosLibresEnBloque){
        
        int cantidadCaracteresRestantes = cantidadCaracteres - cantEspaciosLibresEnBloque;

        posicionDisponible = buscarposicionDisponible(archivoMetadata);
        
        llenarDeCaracteres(punteroBloqueCopia, posicionDisponible, caracterDeLlenado, cantidadCaracteresRestantes, archivoMetadata);
    }
        
}

bool vericarMetadata(tipoRecurso recurso){
    //Verifico si existe el archivo metadata de ese, sino lo creo.
}

int obtenerMetadata(tipoRecurso recurso) {
    //Devuelve el archivo de metadata correspondiente al recurso
}

int calcularTamañoBlocks(){
    //Calcula el tamaño de Blocks.ims
}

int buscarposicionDisponible(int archivoMetadata){
    //Busca un lugar disponible en el archivo Blocks.ims
}

int cantPosicionesLibresEn (int posicionDisponible){
    //Devuelve cuantas posiciones libres hay en el bloque
}