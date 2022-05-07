/* 
 * Net util library
 * Author Erwin Meza Vega <emezav@gmail.com> 
 * Depends on split library
*/

#include "netutil.h"

struct sockaddr_in * server_address(unsigned short port) {

  struct sockaddr_in * addr;

  addr = malloc(sizeof(struct sockaddr_in));

  if (addr == NULL) {
    return NULL;
  }

  memset(addr, 0, sizeof(struct sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);

  addr->sin_addr.s_addr = INADDR_ANY;


  return addr;

}

struct sockaddr_in * address_by_ip(char * ip, unsigned short port) {
  struct sockaddr_in * addr;

  addr = malloc(sizeof(struct sockaddr_in));

  if (addr == NULL) {
    return NULL;
  }

  memset(addr, 0, sizeof(struct sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);

  if (inet_aton(ip, &addr->sin_addr) == 0) {
    free(addr);
    return NULL;
  }
  return addr;
}

struct sockaddr_in * address_by_hostname(char * hostname, unsigned short port) {
  struct hostent* host;
  struct servent *srv;
  struct sockaddr_in *addr;
  addr = malloc(sizeof(struct sockaddr_in));

  if (addr == NULL) {
    return NULL;
  }

  host = gethostbyname(hostname); //Tomar el nombre del host (del archivo /etc/hosts)
  memset(addr, 0, sizeof(addr));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr =*(long*)host->h_addr_list[0];


  return addr;
}