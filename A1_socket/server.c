#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdbool.h>


#define READFROMSOCSIZE 20

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <listen-port>\n", prog);
}

int main(int argc, char *argv[]) {
    int errno;
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    char *end = NULL;
    long port_long = strtol(argv[1], &end, 10);
    if (!end || *end != '\0' || port_long <= 0 || port_long > 65535) {
        fprintf(stderr, "Invalid port: %s\n", argv[1]);
        return 1;
    }
   

    // TODO: Create a TCP listen socket (AF_INET, SOCK_STREAM).
    // TODO: Set SO_REUSEADDR on the listen socket.
    // TODO: Bind the socket to INADDR_ANY and the given port.
    // TODO: Listen with a small backlog (e.g., 5-10).
   
    

    // TODO: Accept clients in an infinite loop.
    //   - For each client, read in chunks until EOF.
    //   - For each chunk, write those *exact bytes* to stdout.
    //     Use write(STDOUT_FILENO, ...) in a loop to handle partial writes.
    //   - Do NOT use printf/fputs or add separators/newlines/prefixes.
    //   - The test harness compares server stdout byte-for-byte with client input.
    // TODO: Handle EINTR and other error cases as specified.

    // TODO: Close the listen socket before exiting.

    //fprintf(stderr, "args are %s\n", argv[1]);

    int status;
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    if ((status = getaddrinfo(INADDR_ANY, argv[1], &hints, &res)) != 0) {
        fprintf(stderr, "gai errorL %s\n", gai_strerror(status));
        exit(1);
    } 

    try_soc_again:
    int soc = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (soc == -1){
        fprintf(stderr,"error making socket, %s\n", strerror(errno));

        if (errno == EINTR) goto try_soc_again;
        exit(1);
    }

    try_bind_again:
    int binderr = bind(soc, res->ai_addr, res->ai_addrlen);

    if (binderr == -1){
        fprintf(stderr,"error binding socket, %s\n", strerror(errno));

        if (errno == EINTR) goto try_bind_again;
        exit(1);
    }

    int yes=1;
    setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);


    listen(soc, 10);

    struct sockaddr_storage rec_addr;
    socklen_t addr_size;
    int rec_soc;
    //char *test = "temp 122212222\n";



    while (true){

        addr_size = sizeof rec_addr;
        rec_soc = accept(soc, (struct sockaddr *)&rec_addr, &addr_size);

        char readBuff[READFROMSOCSIZE];
        char *buff_ptr = readBuff;
        int read = -1;

        while (read != 0){
            read = recv(rec_soc, buff_ptr, READFROMSOCSIZE, 0);
            int wrote = 0;
            //*(buff_ptr + read) = '\0';

            //fprintf(stdout, "read %d bytes\n", read);
            while (wrote < read){
                char *curr_str_ptr = (buff_ptr + wrote);
                wrote += write(STDOUT_FILENO, curr_str_ptr, (read - wrote));
                //fprintf(stdout, "\nsize of last print is %ld\n", strlen(curr_str_ptr));
            }

        }
        
        //write(STDOUT_FILENO, test, strlen(test));

        //fprintf(stdout, "closing connection socket\n");
        close(rec_soc);
    }


    //return 0;
}
