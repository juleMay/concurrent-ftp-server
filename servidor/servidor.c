#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#include "netutil.h"
#include "split.h"
#include "ioutil.h"

#define MAX_CLIENTS 100
#define MAX_FILES 50

sem_t clients_sem;
sem_t cli_arr_mutex;
sem_t f_arr_mutex;

typedef struct {
    char file_name[PATH_MAX];
    sem_t file_mutex;
}file_state;

typedef struct {
    file_state *file_array[MAX_FILES];
    int size;
}file_state_array;

int client_array[MAX_CLIENTS];
file_state_array *fs_array;


void sig_handler(int SIGNO);

int get_next_client();

int is_file_free(char *);

void close_client(int);

void close_server();

file_state *get_file_state(char *);

void *connection_handler(void *);



int main(int argc, char *argv[])
{
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        printf("No es posible manejar la señal SIGINT\n");
    }

    struct sockaddr_in *addr;
    int sd;
    int client_sd;

    if (argc != 2)
    {
        fprintf(stderr, "Debe especificar el puerto a escuchar\n");
        exit(EXIT_FAILURE);
    }

    /* Creando el socket con el servidor */
    sd = socket(PF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        printf("No es posible crear el socket\n");
        exit(EXIT_FAILURE);
    }

    /* Obtener una direccion y un puerto para escuchar */
    addr = server_address(atoi(argv[1]));

    /* Enlazar el socket a la direccion */
    if (bind(sd, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) < 0)
    {
        printf("Operacion bind fallida\n");
        exit(EXIT_FAILURE);
    }

    /* Escuchando el socket abierto */
    if (listen(sd, 10) < 0)
    {
        printf("No es posible escuchar al socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando por el puerto: %s\n", argv[1]);

    sem_init(&clients_sem, 0, MAX_CLIENTS);
    sem_init(&cli_arr_mutex, 0, 1);
    sem_init(&f_arr_mutex, 0, 1);
    
    memset(client_array,0,MAX_CLIENTS);
    fs_array = malloc(sizeof(file_state_array));
    fs_array->size = 0;

    /* Se pasa a atender todas las peticiones hasta que se finalice el proceso */
    while (1)
    {
        printf("Esperando conexiones...\n");

        /* Bloquearse esperando que un cliente se conecte */
        client_sd = accept(sd, 0, 0);
        if (client_sd < 0)
        {
            printf("No es posible aceptar al cliente\n");
            continue;
        }
        printf("Cliente %d conectado\n", client_sd);

        pthread_t pt;
        int *pclient = malloc(sizeof(int));
        *pclient = client_sd;

        int index = get_next_client();
        
        sem_wait(&clients_sem);
        sem_wait(&cli_arr_mutex);
        client_array[index] = client_sd;
        sem_post(&cli_arr_mutex);
        
        pthread_create(&pt, NULL, connection_handler, pclient);
    }
}

int get_next_client(){
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (client_array[i] == 0){
            return i;
        }
    }
    return -1;
}

void close_client(int client){
    
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (client == client_array[i])
        {
            close(client);
            client_array[i] = 0;
            break;
        }
    }
    sem_post(&clients_sem);

}

void close_server(){
    int i = 0;
    sem_wait(&cli_arr_mutex);

    while (client_array[i] != 0 && i < MAX_CLIENTS){
        close_client(client_array[i]);
        i++;
    }

    sem_post(&cli_arr_mutex);
}

file_state *get_file_state(char *file_name)
{
    file_state *fs = 0;

    sem_wait(&f_arr_mutex);

    for (int i = 0; i < fs_array->size; i++)
    {
        if (strcmp(file_name, fs_array->file_array[i]->file_name) == 0)
        {
            fs = fs_array->file_array[i];
        }
    }

    if (fs_array->size < MAX_FILES && fs == 0 && file_name != 0)
    {
        fs = malloc(sizeof(file_state));
        sem_init(&fs->file_mutex, 0, 1);
        strcpy(fs->file_name, file_name);
        fs_array->file_array[fs_array->size++] = fs;
    }

    sem_post(&f_arr_mutex);
    return fs;
}

void *connection_handler(void *client_socket)
{
    struct stat stats;
    int fd;

    char path[PATH_MAX];

    msg_header *header;

    int nread;
    int client_sd = *(int *)client_socket;

    free(client_socket);

    while (1)
    {
        /* Recibe una peticion de enviar o recibir un archivo del cliente */
        header = malloc(sizeof(msg_header));
        nread = read(client_sd, header, sizeof(msg_header));
        if (nread <= 0)
        {
            break;
        }

        printf("Leido del proceso cliente %d (%d bytes): %s %s\n", client_sd, nread, header->request, header->file_name);

        /* Se inicializan las variables con 0 */
        fd = 0;
        memset(path, 0, PATH_MAX);

        /* Se crea el path para guardar y recuperar archivos */
        strcpy(path, "./archivos/");
        strcat(path, header->file_name);

        file_state *fs = get_file_state(header->file_name);
        if (fs == 0){
            continue;
        }

        sem_wait(&fs->file_mutex);

        /* Se recibe un archivo desde el cliente */
        if (strcmp(header->request, "send") == 0)
        {
            printf("Archivo recibido (%d), guardando en: %s\n", header->file_size, path);
            /**
             * Se abre el archivo en modo lectura/escritura
             * se crea si no existe o se sobre escribe
             * se dan permisos de escritura y lectura
             */
            fd = open(path, O_RDWR | O_CREAT | O_TRUNC, header->mode);

            /* Se comprueba que se pueda crear y guardar el archivo */
            if (fd < 0)
            {
                printf("No es posible guardar el archivo\n");
                continue;
            }

            /* Se lee por partes (del socket) el contenido del archivo */
            write_from_socket(client_sd, fd, header->file_size);

            /* Se cierra el archivo */
            close(fd);

            printf("El archivo fue guardado exitosamente\n");
        }
        else if (strcmp(header->request, "recv") == 0)
        {

            /* Se verifica la existencia del archivo mediante stat() */
            if (stat(path, &stats) != 0)
            {
                printf("No se pudo encontrar el archivo\n");
                printf("Compruebe la ruta y/o nombre del archivo: %s\n", path);
            }

            /* Recupera el tamano del archivo mediante stat*/
            header->file_size = stats.st_size;
            header->mode = stats.st_mode;

            /* Envia al cliente el tamano del archivo */
            send(client_sd, header, sizeof(msg_header), 0);

            /* Comprueba si el archivo este vacio */
            if (stats.st_size <= 0)
            {
                continue;
            }

            /* Abre el archivo */
            fd = open(path, O_RDONLY);

            /* comprueba que se pueda abrir el archivo */
            if (fd <= 0)
            {
                continue;
            }

            printf("Enviando archivo (%d): %s\n", header->file_size, header->file_name);

            /* Lee por partes el contenido del archivo y lo envia al cliente */
            send_to_socket(client_sd, fd, header->file_size);

            /* Se cierra el archivo */
            close(fd);


            printf("Archivo enviado al cliente: %d\n", client_sd);
        }
        else
        {
            printf("No es posible reconocer el comando\n");
        }
            sem_post(&fs->file_mutex);
    }
    printf("Conexion terminada\n");

    /* Cerrar la conexion */
    sem_wait(&cli_arr_mutex);
    close_client(client_sd);
    sem_post(&cli_arr_mutex);

    return 0;
}

void sig_handler(int SIGNO)
{
    if (SIGNO == SIGINT)
    {
        printf("\nTerminando el proceso del servidor\n");
        close_server();
        exit(EXIT_SUCCESS);
    }
}