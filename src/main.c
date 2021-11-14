#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <Windows.h>
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif

#define REPLY_SIZE 65536

static const char * file_name = "text.gz";

#define LENGTH 0x1000
unsigned char buffer[LENGTH];
void DecodeDecompress(void);
int main (int argc, char *argv[])
{
    int s = -1;
    struct sockaddr_in server;
    char server_reply[REPLY_SIZE] = {0};
    int recv_size = 0;

    #if defined(_WIN32) || defined(_WIN64)    
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        printf("\nError: Windows socket subsytsem could not be initialized. Error Code: %d. Exiting..\n", WSAGetLastError());
        exit(1);
    }
    #endif
    
    //Create a socket
    if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)    {
        printf("Error: Could not create socket: %s. Exiting..\n", strerror(errno));
        exit(1);
    }

    // Fill in server's address
    memset(&server, 0, sizeof(server));
    server.sin_addr.s_addr = inet_addr("20.108.244.219"); 
    server.sin_family = AF_INET;
    server.sin_port = htons(5050);

    // Connect to server
    if (connect(s, (struct sockaddr *)(&server), sizeof(server)) < 0) {
        printf("Error: Could not connect to server: %s. Exiting..\n", strerror(errno));
        exit(1);
    }

    DecodeDecompress();

    send(s, buffer, (int) strlen(buffer), 0);
    shutdown(s, SD_SEND);

    //receive all messages from server
    do{
        recv_size =recv(s, server_reply, REPLY_SIZE, 0);
        if (recv_size > 0)
            printf("%s",server_reply);
        else if (recv_size == 0)
            printf("Connection closed\n");
        else
            printf("recv failed: %d\n", WSAGetLastError());
    }while(recv_size > 0);

    // Close the socket
#if defined(_WIN32) || defined(_WIN64)  
    closesocket(s);
    WSACleanup();
#else
    close(s);
#endif

    exit(0);
}
void DecodeDecompress(void){
    
    gzFile file;
    file = gzopen(file_name, "r");
    if (! file) {
        fprintf (stderr, "gzopen of '%s' failed: %s.\n", file_name,
                 strerror (errno));
            exit (EXIT_FAILURE);
    }
    while (1) {
        int err;
        int bytes_read;
        //unsigned char buffer[LENGTH];
        bytes_read = gzread (file, buffer, LENGTH - 1);
        buffer[bytes_read] = '\0';
        //printf ("%s", buffer);
        if (bytes_read < LENGTH - 1) {
            if (gzeof (file)) {
                break;
            }
            else {
                const char * error_string;
                error_string = gzerror (file, & err);
                if (err) {
                    fprintf (stderr, "Error: %s.\n", error_string);
                    exit (EXIT_FAILURE);
                }
            }
        }
    }
    gzclose (file);
}