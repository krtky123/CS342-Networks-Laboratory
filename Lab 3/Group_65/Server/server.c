#include "../common_function.c"

int put(int);
int get(int);
int mput(int);
int mget(int);
int Socket(int , int , int ); 

int main(int argc, char **argv) 
{
    // declarations
    int listenfd, connfd, servport;
    struct sockaddr_in servaddr;
    char buff[MAXLINE];
    char default_dir_path[1024];
    getcwd(default_dir_path, sizeof(default_dir_path));
    time_t ticks;
    char temp[2] = "0";
    int pid;
    int ct = 0;

    if (argc != 2) 
    {
        printf("Command Structure: ./server <Server Port number>\n");
        return -1;
    }

    servport = atoi(argv[1]);
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));  // set all bits in serveraddr 0
    servaddr.sin_family = AF_INET;       // set server IP address type to IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // accept any incoming messages
    servaddr.sin_port = htons(servport); /* daytime server */

    // Give the socket listenfd the local address servaddr
    // (which is sz(serveraddr) bytes long).
    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

    /* Prepare to accept connections on socket listenfd.
     * LISTENQ connection requests will be queued before
     * further requests are refused.
     */
    listen(listenfd, LISTENQ);

    for (;;) {
        printf("\n\nWaiting for Connection\n");
        chdir(default_dir_path);

        /*Await a connection on socket FD.
         * When a connection arrives, open a
         * new socket to communicate with it,
         */
        printf("%d\n", ct);
        int t = waitpid(-1, NULL, WNOHANG);
        if (t > 0) ct--;
        printf("%d\n", t);
        if (ct == ACCEPTQ) 
        {
            printf("WAITING\n");
            connfd = accept(listenfd, (SA *)NULL, NULL);
            strcpy(temp, "7");
            send_cmd(connfd, temp, sizeof(temp));
            close(connfd);
            wait(NULL);
            ct--;
        }
        connfd = accept(listenfd, (SA *)NULL, NULL);

        printf("Connection Successful\n\n");
        ++ct;
        strcpy(temp, "6");
        send_cmd(connfd, temp, sizeof(temp));

        pid = fork();
        if (pid < 0) 
        {
            printf("ERROR in new process creation");
        } 
        else if (pid == 0) 
        {
            // child process
            close(listenfd);
            while (1) {
                char temp[2] = "0";
                recv_cmd(connfd, temp, sizeof(temp));
                printf("Option received\n");
                printf("Option chosen : %s\n", temp);

                send_cmd(connfd, temp, sizeof(temp));
                printf("ACK sent\n");

                char u = temp[0];
                if( u == '1')
                    put(connfd);
                else if( u == '2')
                    get(connfd);
                else if( u == '3')
                    mput(connfd);
                else if( u == '4')
                    mget(connfd);
                else if( u == '5')
                    goto out;
                else
                    printf("Command Not recognized\n");
            }
        out:
            close(connfd);
            return 0;
        }
        else 
        {
            // parent process
            close(connfd);
        }
    }
}


int put(int fd) 
{
    // recieve and upload a file from the client to the server
    char buff[1024];
    char temp[2] = "1";
    recv_cmd(fd, buff, sizeof(buff));
    if (!check_file(buff)) 
    {
        send_confirm(fd, false);
        recv_confirm(fd);
        send_cmd(fd, temp, sizeof(temp));        
    } 
    else 
    {
        send_confirm(fd, true);
        if (!recv_confirm(fd)) 
        {
            send_cmd(fd, temp, sizeof(temp));
            return 0;
        } 
        else 
        {
            send_cmd(fd, temp, sizeof(temp));
        }
    }
    if (!recv_confirm(fd)) 
    {
        printf("File could not be opened\n");
        return 0;
    }
    if (!recv_confirm(fd)) 
    {
        printf("File size too large, try again later\n");
        return 0;
    }
    long int file_size = recv_file_size(fd);
    send_cmd(fd, temp, sizeof(temp));
    recv_file(fd, buff, file_size);
    send_cmd(fd, temp, sizeof(temp));

    return 0;
}

int get(int connfd) 
{
    // send a file to the client from the server
    char buff[1024];
    char temp[2] = "2";

    recv_cmd(connfd, buff, sizeof(buff));
    printf("Requested File : %s\n", buff);

    if (!check_file(buff)) 
    {
        send_confirm(connfd, false);
        printf("Confirmation sent\n");
        printf("Requested File Not Found : %s\n", buff);
    } 
    else 
    {
        send_confirm(connfd, true);
        printf("Confirmation sent\n");

        recv_cmd(connfd, temp, sizeof(temp));
        printf("ACK recieved\n");

        if (send_file(connfd, buff) < 0) 
            return 0;

        recv_cmd(connfd, temp, sizeof(temp));
        printf("ACK recieved\n");

        send_cmd(connfd, temp, sizeof(temp));
        printf("ACK sent\n");
    }

    return 0;
}

int mput(int fd) 
{
    /* Recieve files of a particular file from a client
     * and uplad all of them to the server
     */
    char buff[1024];
    char temp[2] = "3";
    while (recv_confirm(fd)) 
    {
        send_cmd(fd, temp, sizeof(temp));
        recv_put_one_file(fd);
    }
    send_cmd(fd, temp, sizeof(temp));
    return 0;
}

int mget(int fd) 
{
    /* Send all files of an extension to the client
     * Receive the extention from the client
     */
    char buff[1024];
    char dir_path[1024];
    char temp[2] = "4";

    recv_cmd(fd, buff, sizeof(buff));
    DIR *d;
    struct dirent *dir;
    d = opendir(getcwd(dir_path, sizeof(dir_path)));
    if (d) 
    {
        while ((dir = readdir(d))) 
        {
            if (dir->d_type == DT_REG) 
            {
                char extbuf[1024];
                get_filename_ext(dir->d_name, extbuf);
                if (strcmp(extbuf, buff) == 0) 
                {
                    send_confirm(fd, true);
                    recv_cmd(fd, temp, sizeof(temp));
                    send_one_file(fd, dir->d_name);
                }
            }
        }
    }

    send_confirm(fd, false);
    recv_cmd(fd, temp, sizeof(temp));
    send_cmd(fd, temp, sizeof(temp));

    return 0;
}

int Socket(int family, int type, int protocol) 
{
    /* This function requests a socket from the os
     * It exists the application after printing error
     * if the Os cannot provide a socket
     */
    int n;
    if ((n = socket(family, type, protocol)) < 0) {
        printf("error\n");
        exit(0);
    }
    return (n);
}

