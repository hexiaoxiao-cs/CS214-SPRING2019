#include <protocol.h>

extern char* ipaddr;
extern int portno;

int create(char*);

int readConfigure();

int configure(char *server_addr, char *port_no);

int handle_error(parsed_response_t *res);