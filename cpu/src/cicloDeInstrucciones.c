#include "cicloDeInstrucciones.h"
#include <semaphore.h>


t_log* log_ciclo;

t_tipoInstruccion string_a_tipo_instruccion (char* ins_char);
t_list* dividir_cadena_en_tokens(const char* linea);
registrosCPU string_a_registro(const char* registro);

// Funciones necesarias para el ciclo de instrucciones

PCB* recibir_pcb(){
    op_code cod_op = recibir_operacion(socket_kernel_d);
    if(cod_op == ENVIO_PCB){
        t_buffer* buffer = recibir_buffer(socket_kernel_d);
        PCB* pcb_recibido = buffer_read_pcb(buffer);
        liberar_buffer(buffer);
        return pcb_recibido;
    }
    return NULL;
}

void enviar_pcb(op_code motivo){
    t_paquete* paquete = crear_paquete(motivo);
    agregar_PCB_a_paquete (paquete, pcb);
    enviar_paquete(paquete, socket_kernel_d);
    eliminar_paquete(paquete);
}

void enviar_PC_a_memoria(uint32_t pc){
    t_paquete* paquete = crear_paquete(ENVIO_PC);
    agregar_int_a_paquete(paquete, &pcb->PID);
    agregar_a_paquete(paquete, &pc, sizeof(uint32_t));
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

char* obtener_instruccion_de_memoria(){
    log_ciclo = log_create("ErrorRecepcionInstruccion.log","Error de recepcion de instruccion",false,LOG_LEVEL_INFO);
    op_code cod_op = recibir_operacion(socket_memoria);
    if (cod_op == ENVIO_DE_INSTRUCCIONES) { 
        t_buffer* buffer = recibir_buffer(socket_memoria);
        char* instruccion = buffer_read_string(buffer);
        liberar_buffer(buffer);
        return instruccion;
    } else { 
        log_error(log_ciclo, "Error al recibir instrucción de la memoria");
        return NULL;
    } 
    log_destroy(log_ciclo);
        
}

//Ciclo de instrucciones

char* fetch(){
    log_ciclo = log_create("Cpu.log", "Instruccion Buscada", false, LOG_LEVEL_INFO);
    int pid = pcb->PID;
 
    log_info(log_ciclo, "PID: %u - FETCH - Program Counter: %u", pid, pcb->programCounter);

    enviar_PC_a_memoria(pcb->programCounter);
    char* instruccionEncontrada = obtener_instruccion_de_memoria();

    pcb->programCounter++;

    log_destroy(log_ciclo);

    return instruccionEncontrada;
}

t_instruccion* decode(char* instruccion_sin_decodificar){

    t_list* lista = dividir_cadena_en_tokens(instruccion_sin_decodificar);
    t_tipoInstruccion tipo_de_instruccion = string_a_tipo_instruccion(list_get(lista,0));
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    int valor;
    int*valor_ptr = malloc(sizeof(int));
    switch(tipo_de_instruccion){
        case iSET:
            valor = atoi(list_get(lista,2));
            if(valor_ptr == NULL){
                perror("error de memoria");
                exit(EXIT_FAILURE);
            }
            *valor_ptr = valor;
            instruccion->tipo = iSET;
            instruccion->arg1 = (void*) string_a_registro(list_get(lista,1));
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg2 = sizeof(int);
            instruccion->arg2 = valor_ptr;
            instruccion->sizeArg3 = 0;
            break;
        case iSUM:
            instruccion->tipo = iSUM;
            instruccion->arg1 = (void*) string_a_registro(list_get(lista,1));
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->arg2 = (void*) string_a_registro(list_get(lista,2));
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg3 = 0;
            break;
        case iSUB:
            instruccion->tipo = iSUB;
            instruccion->arg1 = (void*) string_a_registro(list_get(lista,1));
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->arg2 = (void*) string_a_registro(list_get(lista,2));
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg3 = 0;
            break;
        case iJNZ:
            valor = atoi(list_get(lista,2));
            if(valor_ptr == NULL){
                perror("error de memoria");
                exit(EXIT_FAILURE);
            }
            *valor_ptr = valor;
            instruccion->tipo = iJNZ;
            instruccion->arg1 = (void*) string_a_registro(list_get(lista,1));
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg2 = sizeof(int);
            instruccion->arg2 = valor_ptr;
            instruccion->sizeArg3 = 0;
            break;
        case iIO_GEN_SLEEP:
            valor = atoi(list_get(lista,2));
            if(valor_ptr == NULL){
                perror("error de memoria");
                exit(EXIT_FAILURE);
            }
            *valor_ptr = valor;
            instruccion->tipo = iIO_GEN_SLEEP;
            instruccion->arg1 = valor_ptr;
            instruccion->sizeArg1 = sizeof(int);
            instruccion->sizeArg2 = 0;
            instruccion->sizeArg3 = 0;
            instruccion->interfaz = list_get(lista,1);
            break;
        default :
        break;
    }
    return instruccion;
}

void execute(t_instruccion* instruccion){
    log_ciclo = log_create("Cpu.log", "Instruccion Ejecutada", false, LOG_LEVEL_INFO);
    switch (instruccion->tipo)
    {
    case iSET:
        SET(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->arg1, instruccion->arg2);
        break;
    /*
    case iMOV_IN:
        MOV_IN(*(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    case iMOV_OUT:
        MOV_OUT(*(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    */
    case iSUM:
        SUM(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->arg1, instruccion->arg2);
        break;
    case iSUB:
        SUB(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->arg1, instruccion->arg2);
        break;
    case iJNZ:
        JNZ(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->arg1, instruccion->arg2);
        break;
    /*
    case iRESIZE:
        RESIZE(*(int *)instruccion->arg1);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p", pcb->PID, instruccion->tipo, instruccion->arg1);
        break;
    case iCOPY_STRING:
        COPY_STRING(*(int *)instruccion.arg1);
        break;
    case iWAIT:
        WAIT(instruccion.interfaz);
        break;
    case iSIGNAL:
        SIGNAL(instruccion.interfaz);
        break;
    */
    case iIO_GEN_SLEEP:
        IO_GEN_SLEEP((char*)instruccion->interfaz, *(int *)instruccion->arg1);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->interfaz, instruccion->arg1);        
        break;
    /*
    case iIO_STDIN_READ:
        IO_STDIN_READ(instruccion.interfaz, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    case iIO_STDOUT_WRITE:
        IO_STDOUT_WRITE(instruccion.interfaz, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    
    case iIO_FS_CREATE:
        IO_FS_CREATE(instruccion.interfaz, instruccion.archivo);
        break;
    
    case iIO_FS_DELETE:
        IO_FS_DELETE(instruccion.interfaz, instruccion.archivo);
        break;
    case iIO_FS_TRUNCATE:
        IO_FS_TRUNCATE(instruccion.interfaz, instruccion.archivo, *(registrosCPU *)instruccion.arg1);
        break;
    case iIO_FS_WRITE:
        IO_FS_WRITE(instruccion.interfaz, instruccion.archivo, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2, *(registrosCPU *)instruccion.arg3);
        break;
    case IO_FS_READ:
        iIO_FS_READ(instruccion.interfaz, instruccion.archivo, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2, *(registrosCPU *)instruccion.arg3);
        break;
    */
    case iEXIT:
        EXIT();
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - No tiene parámetros", pcb->PID, instruccion->tipo);
        break;
    default:
        log_error(log_ciclo, "La instruccion es inválida");
        break;
    }
    log_destroy(log_ciclo);
}

int check_interrupt() {
    pthread_mutex_lock(&mutexInterrupt);
    if (hay_interrupcion) {
        hay_interrupcion = 0;
        enviar_pcb(INTERRUPCION);
        pthread_mutex_unlock(&mutexInterrupt);
        return 1;
    } else {
        pthread_mutex_unlock(&mutexInterrupt);
        return 0;
    }
}

void realizar_ciclo_de_instruccion(){
    while (1) {
        char* instruccion_a_decodificar = fetch(pcb, socket_memoria);

        t_instruccion* instruccion_a_ejecutar = decode(instruccion_a_decodificar);
        
        execute(instruccion_a_ejecutar);

        if (check_interrupt()) {
            enviar_pcb(INTERRUPCION);
            break; // Romper el bucle si hay interrupción
        }
        // Verificar condiciones de salida 
        if (instruccion_a_ejecutar->tipo == iEXIT){
            break; // Romper el bucle si el proceso ha finalizado
        }
    }
}


// Funciones auxiliares

registrosCPU string_a_registro(const char* registro) {
    if (strcmp(registro, "PC") == 0) return PC;
    if (strcmp(registro, "AX") == 0) return AX;
    if (strcmp(registro, "BX") == 0) return BX;
    if (strcmp(registro, "CX") == 0) return CX;
    if (strcmp(registro, "DX") == 0) return DX;
    if (strcmp(registro, "EAX") == 0) return EAX;
    if (strcmp(registro, "EBX") == 0) return EBX;
    if (strcmp(registro, "ECX") == 0) return ECX;
    if (strcmp(registro, "EDX") == 0) return EDX;
    if (strcmp(registro, "SI") == 0) return SI;
    if (strcmp(registro, "DI") == 0) return DI;
    // Error si registro no valido
    printf("Error: Registro '%s' no válido.\n", registro);
    exit(1);
}

t_tipoInstruccion string_a_tipo_instruccion (char* ins_char){
    if (strcmp(ins_char, "SET") == 0) return iSET;
    if (strcmp(ins_char, "MOV_IN") == 0) return iMOV_IN;
    if (strcmp(ins_char, "MOV_OUT") == 0) return iMOV_OUT;
    if (strcmp(ins_char, "SUM") == 0) return iSUM;
    if (strcmp(ins_char, "JNZ") == 0) return iJNZ;
    if (strcmp(ins_char, "RESIZE") == 0) return iRESIZE;
    if (strcmp(ins_char, "COPY_STRING") == 0) return iCOPY_STRING;
    if (strcmp(ins_char, "WAIT") == 0) return iWAIT;
    if (strcmp(ins_char, "SIGNAL") == 0) return iSIGNAL;
    if (strcmp(ins_char, "IO_GEN_SLEEP")) return iIO_GEN_SLEEP;
    if (strcmp(ins_char, "IO_STDIN_READ") == 0) return iIO_STDIN_READ;
    if (strcmp(ins_char, "IO_STDOUT_WRITE") == 0) return iIO_STDOUT_WRITE;
    if (strcmp(ins_char, "IO_FS_CREATE") == 0) return iIO_FS_CREATE;
    if (strcmp(ins_char, "IO_FS_DELETE") == 0) return iIO_FS_DELETE;
    if (strcmp(ins_char, "IO_FS_TRUNCATE") == 0) return iIO_FS_TRUNCATE;
    if (strcmp(ins_char, "IO_FS_WRITE") == 0) return iIO_FS_WRITE;
    if (strcmp(ins_char, "IO_FS_READ") == 0) return iIO_FS_READ;
    if (strcmp(ins_char, "EXIT") == 0) return iEXIT;

    fprintf(stderr, "Instruccion desconocida '%s'\n", ins_char);
    exit(EXIT_FAILURE);
}

t_list* dividir_cadena_en_tokens(const char* linea){
    t_list* lista = list_create();
    char* cadena = strdup(linea);
    char* token = strtok(cadena, " ");

    while(token != NULL){
        list_add(lista,token);
        token = strtok(NULL," ");
    }
    free(cadena);
    return lista;
}