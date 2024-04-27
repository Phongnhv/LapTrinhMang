#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

const int maxBuf = 1024;

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(7000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    for (int i = 0; i < 8; i++) {
        if (fork() == 0) {
            char buf[maxBuf];
            while (1) {
                int client = accept(listener, NULL, NULL);

                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0) {
                    close(client);
                    continue;
                }

                if (ret < sizeof(buf))
                    buf[ret] = 0;
                printf("%s\n", buf);

                char msg[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello World</h1></body></html>";
                send(client, msg, strlen(msg), 0);

                close(client);
            }
            exit(0);
        }
    }

    getchar();
    killpg(0, SIGKILL);

    return 0;
}