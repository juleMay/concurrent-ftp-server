/* 
 * Net util library
 * Author Erwin Meza Vega <emezav@gmail.com> 
 * Depends on split library
*/

#ifndef NETUTIL_H
#define NETUTIL_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <resolv.h>

typedef struct {
    char request[64];
    char file_name[PATH_MAX];
    unsigned int file_size;
    mode_t mode;
}msg_header;

struct sockaddr_in * server_address(unsigned short port);

struct sockaddr_in * address_by_ip(char * ip, unsigned short port);

struct sockaddr_in * address_by_hostname(char * hostname, unsigned short port);

#endif
