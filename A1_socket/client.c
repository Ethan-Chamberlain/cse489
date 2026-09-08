#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <netdb.h>

#define STDINREADSIZE 10


static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <server-ip> <server-port>\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    char *end = NULL;
    long port_long = strtol(argv[2], &end, 10);
    if (!end || *end != '\0' || port_long <= 0 || port_long > 65535) {
        fprintf(stderr, "Invalid port: %s\n", argv[2]);
        return 1;
    }
    //fprintf(stderr, "args are %s, %s, %s\n", argv[1], argv[2], argv[3]);


    // TODO: Create a TCP socket (AF_INET, SOCK_STREAM).
    // TODO: Populate sockaddr_in with server IP/port.
    // TODO: Connect to the server.
    // TODO: Read from stdin in a loop (read()) and send in chunks.
    // TODO: For each chunk, send the *exact bytes* you read.
    //   - Use send()/write() in a loop to handle partial sends.
    //   - Do NOT add newlines, prefixes, or other formatting.
    // The test harness compares server stdout byte-for-byte with stdin input.

    // TODO: Close the socket before exiting.

        

    


    int status;
    struct addrinfo hints;
    struct addrinfo *res;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
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

    connect(soc, res->ai_addr, res->ai_addrlen);

    int c;
    char buff[STDINREADSIZE];
    char *buf_ptr = buff;
    int read_bytes = -1;

    int len =0;
    int bytes_sent =0;


    while (read_bytes != 0){
        read_bytes = read(STDIN_FILENO, buf_ptr, STDINREADSIZE);

        while (bytes_sent < read_bytes){
            
            bytes_sent += send(soc, (buf_ptr + bytes_sent), (read_bytes - bytes_sent), 0);
            //bytes_sent += write(STDOUT_FILENO, (buf_ptr + bytes_sent), (read_bytes - bytes_sent));

        }
        bytes_sent = 0;
    }
    //fprintf(stdout, "read bytes 0, last read is %x\n", buf_ptr);

    
    
    //fprintf(stdout, "ending client send\n");
    close(soc);
    freeaddrinfo(res);

    return 0;
}
