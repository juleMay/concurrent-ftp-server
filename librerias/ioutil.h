#ifndef IOUTIL_H
#define IOUTIL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <sys/socket.h>

void send_to_socket(int sd, int fd, unsigned int filesize);
void write_from_socket(int sd, int fd, unsigned int filesize);

#endif