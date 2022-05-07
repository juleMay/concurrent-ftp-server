#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "netutil.h"
#include "split.h"
#include "ioutil.h"

int main(int argc, char *argv[])
{
    struct sockaddr_in *addr;
    int sd;
    int fd;

    char comando[PATH_MAX + 64];

    split_list *sp;
    char path[PATH_MAX];

    msg_header *header;

    struct stat stats;

    int nread;

    /* Primero se obtiene la direccion IP del host a conectar */
    if (argc < 3)
    {
        printf("\nFaltan parametros de llamada.\nUso:\n\n%s nombre_host\n\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Creando el socket con el servidor */
    sd = socket(PF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        printf("No es posible crear el socket\n");
        exit(EXIT_FAILURE);
    }

    /* Obtener la direccion del servidor */
    addr = address_by_ip(argv[1], atoi(argv[2]));

    /* Conectarse al servidor */
    if (connect(sd, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) != 0)
    {
        printf("No es posible conectarse al servidor\n");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server\n");

    /* Recibir comandos del usuario */
    while (1)
    {
        memset(comando, 0, PATH_MAX + 64);
        /* Leer el comando por entrada estandar */
        printf("> ");
        fgets(comando, PATH_MAX + 64, stdin);

        sp = split(comando, " \r\n");
        if (sp->count == 0)
        {
            // siguiente iteracion
            continue;
        }
        printf("Comando ingresado por el usuario: %s\n", comando);

        fd = 0;
        memset(path, 0, PATH_MAX);

        if (strcmp(sp->parts[0], "exit") == 0)
        {
            // rompe el ciclo
            break;
        }
        else if (sp->count != 2)
        {
            continue;
        }

        /* Fichero por defecto archivos */
        strcpy(path, "./archivos/");
        strcat(path, sp->parts[1]);

        /* Inicializa el header del mensaje a enviar al servidor */
        header = malloc(sizeof(msg_header));

        if (strcmp(sp->parts[0], "send") == 0)
        {
            /* Se verifica la existencia del archivo mediante stat() */
            if (stat(path, &stats) != 0)
            {
                printf("No se pudo encontrar el archivo\n");
                printf("Compruebe la ruta y/o nombre del archivo: %s\n", path);
                continue;
            }

            /* Comprueba que el archivo no este vacio */
            if (stats.st_size <= 0)
            {
                printf("El archivo esta vacio\n");
                continue;
            }

            /* Crea el header para enviar al servidor */
            strcpy(header->request, "send");         // peticion
            strcpy(header->file_name, sp->parts[1]); // nombre del archivo
            header->file_size = stats.st_size;       // tamano del archivo
            header->mode = stats.st_mode;            // permisos del archivo

            /* Envia la peticion send al servidor */
            send(sd, header, sizeof(msg_header), 0);

            /* Abre el archivo en modo lectura */
            fd = open(path, O_RDONLY);

            /* Se comprueba que se pueda abrir el archivo */
            if (fd < 0)
            {
                printf("No es posible abrir el archivo\n");
                continue;
            }

            /* Lee por partes el contenido del archivo y lo envia al servidor */
            printf("Enviando archivo (%d): %s\n", header->file_size, header->file_name);
            send_to_socket(sd, fd, stats.st_size);

            /* Cierra el archivo */
            close(fd);
            printf("Archivo enviado al servidor\n");
        }
        else if (strcmp(sp->parts[0], "recv") == 0)
        {
            /* Crea el header para enviar al servidor */
            strcpy(header->request, "recv");         // peticion
            strcpy(header->file_name, sp->parts[1]); // nombre del archivo

            /* Informa al servidor que necesita recibir un archivo */
            send(sd, header, sizeof(msg_header), 0);

            /* Recibe el tamano del archivo solicitado */
            nread = read(sd, header, sizeof(msg_header));
            if (nread <= 0 || header->file_size <= 0)
            {
                printf("No es posible recibir el archivo\n");
                continue;
            }

            printf("Archivo recibido (%d), guardando en: %s\n", header->file_size, path);
            /**
             * Se abre el archivo en modo lectura/escritura
             * se crea si no existe o se sobre escribe
             * se asignan los mismos permisos del archivo original
             */
            fd = open(path, O_RDWR | O_CREAT | O_TRUNC, header->mode);

            /* Se comprueba que se pueda crear y/o escribir el archivo */
            if (fd < 0)
            {
                printf("No es posible guardar el archivo\n");
                continue;
            }

            /* Se lee por partes (del socket) el contenido del archivo */
            write_from_socket(sd, fd, header->file_size);

            /* Se cierra el archivo */
            close(fd);
            printf("El archivo fue guardado exitosamente\n");
        }
        else
        {
            printf("Comando no valido!\n");
            continue; // siguiente iteracion
        }
    }

    printf("Connection terminated\n");
    close(sd);
}
