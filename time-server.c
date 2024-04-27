#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

int syntax_check(char buf[]){
    char comd[9];
    if (strlen(buf) < 8) return 0;
    memcpy(comd, buf, 8);
    if (memcmp(comd,"GET_TIME", 8) != 0) return 0;
    char type[11];
    memcpy(type,buf + 9, sizeof(type));
    if (!memcmp(type, "dd/mm/yyyy", 10)) return 1;
    if (!memcmp(type, "dd/mm/yy", 8)) return 2;
    if (!memcmp(type, "mm/dd/yyyy", 10)) return 3;
    if (!memcmp(type, "mm/dd/yy", 8)) return 4;
    return 0;
}

void signalHanlder(int signo) {
    pid_t pid = wait(NULL);
    printf("Child process terminated, pid = %d\n", pid);
}

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
    signal(SIGCHLD, signalHanlder);

    while(1){
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);

        if (fork() == 0)
        {
            close(listener);

            char buf[256];
            char res[256];

            while (1) {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;

                buf[ret] = 0;
                int type = syntax_check(buf);
                if (type == 0){
                    strcpy(res, "Syntax Error, please retry");
                }
                else{
                    time_t currentTime;
                    struct tm *localTime;

                    time(&currentTime);
                    localTime = localtime(&currentTime);

                    char stype[256];
                    if (type == 1) strcpy(stype, "%d/%m/%Y");
                    else if (type == 2)  strcpy(stype, "%d/%m/%y");
                    else if (type == 3) strcpy(stype, "%m/%d/%Y");
                    else if (type == 4) strcpy(stype, "%m/%d/%y");
                    strftime(res, sizeof(res), stype, localTime);
                }
                printf("Received: %s", buf);

                send (client,res,strlen(res),0);
            }

            exit(0);
        }
        close(client);
    }

    return 0;
}
