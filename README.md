# concurrent-ftp-server

Aplicación en c para transferir archivos de forma bidireccional entre cliente y servidor. Usa semáforos para permitir conexiones concurrentes de múltiples clientes al servidor.

## Build
Para compilar la aplicación, en el directorio raíz del proyecto, utilice el comando:
`make`

## Clean
Para limpiar el proyecto, en el directorio raíz, utilice el comando:
make clean

## Run
1. Para correr el servidor, en el directorio servidor, utilice el comando:
`./servidor` *`<puerto>`*

2.  Para correr el cliente, en el directorio cliente, utilice el comando:
`./cliente ` *`<ip de servidor>` `<puerto>`*


## Comandos del Cliente
Envíar archivos al servidor desde la carpeta archivos del cliente:
> send "nombre de archivo"

Solicitar archivos al servidor, se guardan en la carpeta archivos del cliente:
> recv "nombre de archivo"

Terminar el proceso cliente:
> exit
