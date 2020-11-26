#include "../common_function.c"

int getfile(int , char *, int ); 
int putfile(int , char *, int );
int mgetfile(int , char *ptr); 
int mputfile(int , char *ptr); 
int closeConnection(int); 
int Socket(int , int , int ); 

int main(int argc, char **argv) 
{
    // declarations
    
    char buf[1024];
    int option;
    int socket_fd, n, server_port;
    char recvline[MAXLINE + 1];
    struct sockaddr_in serv_address;

    // check if server IP addr given , application port already known
    if (argc != 3) 
    {
        printf("Command format: ./client <Server IP Address> <Server Port number>\n");
        return -1;
    }

    socket_fd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serv_address, sizeof(serv_address));  // set all bits in serveraddr 0
    serv_address.sin_family = AF_INET;       // set server IP address type to IPv4

    /* The htons() function makes sure that numbers are stored in memory in
     * network byte order, which is with the most significant byte first. It
     * will therefore swap the bytes making up the number so that in memory the
     * bytes will be stored in the order
     */
    server_port = atoi(argv[2]);
    serv_address.sin_port = htons(server_port); /* daytime server */

    // Convert and store IP address currently in the presenatation format to
    // binary format
    if (inet_pton(AF_INET, argv[1], &serv_address.sin_addr) <= 0) 
    {
        printf("inet_pton error for %s", argv[1]);
        return -1;
    }

    printf("Connecting...\n");

    // Open a connection on socket socket_fd to peer at serveraddr
    if (connect(socket_fd, (SA *)&serv_address, sizeof(serv_address)) < 0) 
    {
        printf("Could not connect!! Error!!");
        return -1;
    }

    char temp[2];
    recv_cmd(socket_fd, temp, sizeof(temp));
    if (strcmp(temp, "6") == 0) 
    {
        printf("Connection Successful\n");
    } 
    else 
    {
        printf("Connection Unsuccessful\n");
        return 0;
    }

    while(1) 
    {
        // Print options
        printf(
            "\nEnter Command Key\n 1 -> Put \n 2 -> Get \n 3 -> Mput \n 4 -> "
            "Mget \n 5 -> Exit \n \n"
            );

        // Read input option
        scanf("%d", &option);

        if(option == 1)
        {
            printf("Enter Filename: ");
            scanf("%s", buf);
            printf("%s\n", buf);
            if (check_file(buf)) 
            {
                putfile(socket_fd, buf, sizeof(buf));
            } 
            else 
            {
                printf("File %s does not exist\n", buf);
            }
        }
        else if(option == 2)
        {
            printf("Enter Filename: ");
            scanf("%s", buf);
            getfile(socket_fd, buf, sizeof(buf));
        }
        else if(option == 3)
        {
            printf("Enter File extension without using . : ");
            scanf("%s", buf);
            mputfile(socket_fd, buf);
        }
        else if(option == 4)
        {
            printf("Enter File extension without using . : ");
            scanf("%s", buf);
            mgetfile(socket_fd, buf);
        }
        else if(option == 5)
        {
            closeConnection(socket_fd);
            close(socket_fd);
            goto out;
        }
        else 
        {
            printf("Command undefined. \n");
        }
    }
out:
    return 0;
}

int getfile(int fd, char *ptr, int size) 
{
    /* get a file from the server
     * INPUTS :=
     * fd : socket
     * ptr : buffer with filename
     * size : len of filename
     */
    char temp[2] = "2";
    char buff[1024];
    strcpy(buff, ptr);

    /* Check if the desired file exists
     * If exists then ask the user if he wants to overwrite
     * the previous file
     */
    if (check_file(buff)) 
    {
        char option;

    here:
        printf("Do You Wish To Overwrite %s Y/N :", buff);
        scanf("\n%c", &option);

        if (option == 'N' || option == 'n') 
        {
            return 0;
        } 
        else if (option != 'Y' && option != 'y') 
        {
            printf("Option not recognized %c\n", option);
            goto here;
        }
    }

    send_cmd(fd, temp, sizeof(temp));
    printf("Sending Option...\n");

    recv_cmd(fd, temp, sizeof(temp));
    printf("Recieve ACK from Server %s\n", temp);

    send_cmd(fd, buff, sizeof(buff));
    printf("Sending File name : %s\n", buff);

    /* Receive file from the server
     * use ack and messages to communicate with the server
     */
    if (recv_confirm(fd)) 
    {
        printf("File found on server. Confirm Recieved\n");

        send_cmd(fd, temp, sizeof(temp));
        printf("ACK sent\n");
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
        printf("ACK sent\n");

        recv_file(fd, buff, file_size);

        send_cmd(fd, temp, sizeof(temp));
        printf("ACK sent\n");

        recv_cmd(fd, temp, sizeof(temp));
        printf("ACK recieved\n");
    } 
    else 
    {
        printf("The requested file does not exist on the server.\n");
    }
    return 0;
}

int putfile(int fd, char *ptr, int size) 
{
    // upload file to the server
    char temp[2] = "1";
    char buff[1024];
    strcpy(buff, ptr);

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));

    send_cmd(fd, buff, sizeof(buff));
    printf("This is ....%s %ld\n", buff, sizeof(buff));

    /* Check if the desired file exists
     * If exists then ask the user if he wants to overwrite
     * the previous file
     * When the file has no counterpart present on the server then upload
     * the file on the server
     */

    if(!recv_confirm(fd)) 
    {
        

        send_confirm(fd, true);
        recv_cmd(fd, temp, sizeof(temp));
    } 
    else 
    {
        char option;
    recv_confirmation:
        printf(
            "The file already exists on remote host. Do you wish to "
            "overwrite(y/n) "
            ": ");
        scanf("\n%s", &option);

        if (option == 'N' || option == 'n') 
        {
            send_confirm(fd, false);
            recv_cmd(fd, temp, sizeof(temp));
            return 0;
        } 
        else if (option == 'Y' || option == 'y') 
        {
            send_confirm(fd, true);
            recv_cmd(fd, temp, sizeof(temp));
        } 
        else if (option != 'Y' && option != 'y') 
        {
            printf("Option not recognized %c\n", option);
            goto recv_confirmation;
        }
    }

    if (send_file(fd, buff) == 0) 
    {
        recv_cmd(fd, temp, sizeof(temp));
    }

    return 0;
}

int mgetfile(int fd, char *ptr) 
{
    // get all files one by one having a given extension
    char temp[2] = "4";
    char buff[1024];
    strcpy(buff, ptr);

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));
    send_cmd(fd, buff, sizeof(buff));
    while (recv_confirm(fd)) 
    {
        send_cmd(fd, temp, sizeof(temp));
        recv_one_file(fd);
    }

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));
}

int mputfile(int fd, char *ptr) 
{
    // put all files one by one having a given extension
    char temp[2] = "3";
    char buff[1024];
    char dir_path[1024];
    strcpy(buff, ptr);

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));

    /* For each file in the given directory having the given
     * extension upload that file to the server
     */

    DIR *d = opendir(getcwd(dir_path, sizeof(dir_path)));
    struct dirent *dir;
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
                    put_one_file(fd, dir->d_name);
                }
            }
        }
    }
    send_confirm(fd, false);
    recv_cmd(fd, temp, sizeof(temp));
    return 0;
}

int closeConnection(int fd) 
{
    // instructs the server to close the connection
    char temp[2] = "5";
    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));
    return 0;
}

int Socket(int family, int type, int protocol) 
{
    // requests a socket from the OS. If not available,
    // then print error.
    int n;
    if ((n = socket(family, type, protocol)) < 0) 
    {
        printf("error\n");
        exit(0);
    }
    return socket(family, type, protocol);
}


