#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

int client;

unsigned short send_pasv();
int send_list();
int download_file(char *remote_file);
int upload_file(char *local_file);
int rename_file(char *cur_file, char *new_file);
int delete_file(char *filename);
int print_working_dir();
int change_working_dir(char *dirname);
int make_dir(char *dirname);
int remove_dir(char *dirname);

int main() 
{
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("172.25.184.180");
    addr.sin_port = htons(21);

    int ret = connect(client, (struct sockaddr *) &addr, sizeof(addr));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    // Nhan xau chao
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    char username[32], password[32];
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);

    username[strlen(username) - 1] = 0;
    password[strlen(password) - 1] = 0;

    sprintf(buf, "USER %s\r\n", username);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    sprintf(buf, "PASS %s\r\n", password);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    if (strncmp(buf, "230 ", 4) == 0) {
        puts("Success.");
    } else {
        puts("Failed.");
        close(client);
        return 1;
    }

    int choice;
    char filename[256], new_filename[256], dirname[256];

    while (1) {
        printf("Choose your action:\n");
        printf("1. Print current Directory\n");
        printf("2. Change Directory\n");
        printf("3. Create Directory\n");
        printf("4. Delete Directory\n");
        printf("5. Download file\n");
        printf("6. Upload file\n");
        printf("7. re-name file\n");
        printf("8. delete file\n");
        printf("0. exit\n");
        printf("Your choice: ");
        scanf("%d", &choice);
        getchar(); // Xóa ký tự newline sau khi nhập số

        if (choice == 1){
            send_list();
            
        } else if (choice == 2){
            printf("Enter Directory: ");
            fgets(dirname, sizeof(dirname), stdin);
            dirname[strlen(dirname) - 1] = 0;
            change_working_dir(dirname);
            
        } else if (choice == 3){
            printf("Enter Directory Name to create: ");
            fgets(dirname, sizeof(dirname), stdin);
            dirname[strlen(dirname) - 1] = 0;
            make_dir(dirname);
            
        } else if (choice == 4){
            printf("Enter Directory Name to delete: ");
            fgets(dirname, sizeof(dirname), stdin);
            dirname[strlen(dirname) - 1] = 0;
            remove_dir(dirname);
            
        }
         else if (choice == 5){
            printf("Enter file to download: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strlen(filename) - 1] = 0;
            download_file(filename);
            
        }
         else if (choice == 6){
            printf("Enter file to upload: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strlen(filename) - 1] = 0;
            upload_file(filename);
            
        }
        else if (choice == 7){
            printf("Enter current file name: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strlen(filename) - 1] = 0;
            printf("Enter new file name: ");
            fgets(new_filename, sizeof(new_filename), stdin);
            new_filename[strlen(new_filename) - 1] = 0;
            rename_file(filename, new_filename);
            
        }
        else if (choice == 8){
            printf("Enter file name to delete: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strlen(filename) - 1] = 0;
            delete_file(filename);
            
        }
        else if (choice == 0){
            close(client);
            return 0;
        } else{
            printf("Inapropriate action.\n");
        }
    }
}

unsigned short send_pasv() {
    char buf[2048];

    send(client, "PASV\r\n", 6, 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        exit(1);
    }

    buf[ret] = 0;


    char *pos = strchr(buf, '(');
    int i1 = atoi(strtok(pos, "(),"));
    int i2 = atoi(strtok(NULL, "(),"));
    int i3 = atoi(strtok(NULL, "(),"));
    int i4 = atoi(strtok(NULL, "(),"));
    int p1 = atoi(strtok(NULL, "(),"));
    int p2 = atoi(strtok(NULL, "(),"));

    return p1 * 256 + p2;
}

int send_list() {
    
    unsigned short port = send_pasv();
    printf("Port: %d\n", port);
 
    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr("172.25.184.180");
    addr_data.sin_port = htons(port);

    int ret = connect(client_data, (struct sockaddr *) &addr_data, sizeof(addr_data));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    send(client, "LIST\r\n", 6, 0);

    char buf[2048];

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    while (1) {
        ret = recv(client_data, buf, sizeof(buf) - 1, 0);
        if (ret <= 0) {
            close(client_data);
            break;
        }

        buf[ret] = 0;
        printf("%s", buf);
    }

    printf("\n");

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    return 0;
}

int download_file(char *remote_file) {
    unsigned short port = send_pasv();
    printf("Port: %d\n", port);

    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr("172.25.184.180");
    addr_data.sin_port = htons(port);

    int ret = connect(client_data, (struct sockaddr *) &addr_data, sizeof(addr_data));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    sprintf(buf, "RETR %s\r\n", remote_file);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    FILE *f = fopen(remote_file, "wb");
    while (1) {
        ret = recv(client_data, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        fwrite(buf, 1, ret, f);
    }
    close(client_data);
    fclose(f);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    return 0;
}

int upload_file(char *local_file) {
    unsigned short port = send_pasv();
    printf("Port: %d\n", port);

    // Mo ket noi du lieu 
    int client_data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr_data;
    addr_data.sin_family = AF_INET;
    addr_data.sin_addr.s_addr = inet_addr("172.25.184.180");
    addr_data.sin_port = htons(port);

    int ret = connect(client_data, (struct sockaddr *) &addr_data, sizeof(addr_data));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    sprintf(buf, "STOR %s\r\n", local_file);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    FILE *f = fopen(local_file, "rb");
    while (1) {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        send(client_data, buf, ret, 0);
    }
    close(client_data);
    fclose(f);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    return 0;
}

int rename_file(char *cur_file, char *new_file) {
    char buf[2048];

    sprintf(buf, "RNFR %s\r\n", cur_file);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    sprintf(buf, "RNTO %s\r\n", new_file);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    return 0;
}

int delete_file(char *filename) {
    char buf[2048];

    sprintf(buf, "DELE %s\r\n", filename);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    return 0;
}

int print_working_dir() {
    char buf[2048];

    send(client, "PWD\r\n", 5, 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    return 0;
}

int change_working_dir(char *dirname) {
    char buf[2048];

    sprintf(buf, "CWD %s\r\n", dirname);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    return 0;
}

int make_dir(char *dirname) {
    char buf[2048];

    sprintf(buf, "MKD %s\r\n", dirname);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    return 0;
}

int remove_dir(char *dirname) {
    char buf[2048];

    sprintf(buf, "RMD %s\r\n", dirname);
    send(client, buf, strlen(buf), 0);

    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    return 0;
}