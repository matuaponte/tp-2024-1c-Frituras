#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>
#include <utils/serializacion.h>
<<<<<<< HEAD
//Falta agregar hilos para diferentes conexiones 
=======
>>>>>>> main

void iniciarInterfazGenerica(int socket, t_config* config, char* nombre){

    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    t_paquete* paquete = crear_paquete(CONEXION_IOGENERICA);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete ,socket);
    eliminar_paquete(paquete);

    while (1) {
       ssize_t reciv = recibir_operacion(socket);

        if (reciv < 0) {
            exit(-1);
        }
        
        t_buffer* buffer = recibir_buffer(socket);
        int unidades_trabajo = buffer_read_int(buffer);
        sleep(tiempo_pausa * unidades_trabajo);

        t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
        enviar_paquete(paquete ,socket);
        eliminar_paquete(paquete);

    }

}

int main(int argc, char* argv[]) {

    t_config* nuevo_config = config_create("entradasalida.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    int conexion_kernel = crear_conexion(config_get_string_value(nuevo_config,"IP_KERNEL"), config_get_string_value(nuevo_config, "PUERTO_KERNEL"), IO);

    char* tipo = config_get_string_value(nuevo_config,"TIPO_INTERFAZ");

<<<<<<< HEAD
    if(!strcmp(tipo,"IO_GEN_SLEEP")){
        iniciarInterfazGenerica(conexion_kernel, nuevo_config, "Interfaz1");
=======
    if(!strcmp(tipo,"GENERICA")){
        iniciarInterfazGenerica(conexion_kernel, nuevo_config, argv[1]);
>>>>>>> main
    }

    return 0;
}