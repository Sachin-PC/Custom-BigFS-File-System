#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <string.h>
#include<sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/msg.h>
#include <mqueue.h>
#include<errno.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include <time.h>
#include <poll.h>

#define MAX_READ 8192
#define MAX_SIZE 2048


void generate_filename(char *p)
{
    int file_fd =0,i=0,len=0,x=0;
    ssize_t numRead,numWrite;
    file_fd = open("File Name/file_name.txt", O_RDWR , S_IRUSR | S_IWUSR);
    char file_nm[MAX_SIZE];
    memset(file_nm, 0, MAX_SIZE * sizeof(char));
    numRead = read(file_fd,file_nm,MAX_SIZE);
    len = strlen(file_nm)-1;    //excluding \n
    for(i = len-1;i>=4;i--)
    {
        x = file_nm[i] - 48;
        if(x != 9)
        {
            x = x+1;
            file_nm[i] = x + 48;
            break;
        }
    }

    if(i == 3)
    {
        file_nm[4]='1';
        for(i=5;i<=len;i++)
        {
            file_nm[i] = '0';
        }
        file_nm[i]='\n';
    }
    else
    {
        for(i=i+1;i<len;i++)
        {
            file_nm[i]='0';
        }
        file_nm[i]='\n';
    }
    lseek(file_fd,0,0);
    write(file_fd,file_nm,strlen(file_nm));
    memset(file_nm, 0, MAX_SIZE * sizeof(char));
    lseek(file_fd,0,0);
    numRead = read(file_fd,file_nm,MAX_SIZE);
    strcpy(p,file_nm);
}





int main()
{

    char cwrkd[MAX_SIZE];
    if(chdir("/home/sachin/Documents/Servers/DataServer1") != 0)
    {
        printf("Error while changing the directory\n");
        exit(0);
    }
    printf("Server's current working directory : %s\n", getcwd(cwrkd, 100));

    int fd , server_socket , client_socket , i=0,fl_fd=0;
    int nfds=0 , maxi = 0 , nready=0 , sockfd=0 , maxfd=0 , chid ,flag=0;
    int client[FD_SETSIZE] , fds[FD_SETSIZE] , is_type_set[FD_SETSIZE] ,operation[FD_SETSIZE];
    fd_set readfds, writefds,allset;
    char buffer[MAX_READ + 1] , server_message[MAX_READ + 1],file_name[MAX_SIZE];
    char recvd_data[MAX_SIZE+1], oup_fl_nm[MAX_SIZE];
    char* fl_nms[FD_SETSIZE];
    ssize_t numRead,numWrite,clilen;
    struct sockaddr_in server_address, cliaddr;
    struct pollfd *pollFd;

    struct timeval timeout;
    struct timeval *pto;

    pto = &timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    server_socket = socket(AF_INET,SOCK_STREAM,0);
    bzero(&server_address,sizeof(server_address));
    server_address.sin_family= AF_INET;
    server_address.sin_port = htons(9001);
    if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr.s_addr) <= 0)
    {
        printf("error\n");
        exit(0);
    }

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    listen(server_socket, 5);

    nfds = server_socket;
    maxi = -1;
    for(i=0;i<FD_SETSIZE ; i++)
    {
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(server_socket , &allset);
    maxfd = server_socket;
    for(;;)
    {
        readfds = allset;
        writefds = allset;
        nfds = maxfd + 1;
        //printf("Waiting for a signal\n");
        nready = select(nfds , &readfds , &writefds, NULL, pto);
        //printf("nready = %d\n",nready);
        clilen = sizeof(cliaddr);
        if(FD_ISSET(server_socket , &readfds))
        {
            client_socket = accept(server_socket, NULL, NULL);
            printf("New client is connected\n");
            for(i=0;i<FD_SETSIZE;i++)
            {
                if(client[i] < 0)
                {
                    client[i] = client_socket;
                    is_type_set[i] = 0;
                    generate_filename(file_name);
                    fl_nms[i] = file_name;
                    //strcpy(fl_nms[i],file_name);
                    //printf("fl_names[%d] = %s",i,fl_nms[i]);
                    //printf("i = %d\n",i);
                    break;
                }
            }
            if(i == FD_SETSIZE)
            {

                printf("Too many Clients\n");
                exit(0);
            }
            FD_SET(client_socket, &allset);
            if(client_socket > maxfd)
            {
                maxfd = client_socket;
            }
            if(i > maxi)
            {
                maxi = i;
            }
            if(--nready <= 0)
            {
                continue;
            }
        }

        for(i=0;i<=maxi;i++)
        {
            memset(recvd_data, 0, (MAX_SIZE+1) * sizeof(char));
            if((sockfd = client[i]) <0)
            {
                continue;
            }
            if(FD_ISSET(sockfd, &readfds))
            {

                if(is_type_set[i] == 0)
                {
                    numRead = read(sockfd,buffer,1);
                    printf("numRead = %ld\t First Message = %s",numRead,buffer);
                    if(numRead == 0)
                    {
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = -1;
                        operation[i]=0;
                        is_type_set[i]=0;
                        close(fds[i]);
                        printf("A client connection is closed\n");
                    }
                    else
                    {
                        is_type_set[i] = 1;
                        if(strcmp(buffer,"1") == 0)
                        {
                            operation[i] = 1;
                        }
                        else if(strcmp(buffer,"2") == 0)
                        {
                            operation[i] = 2;
                        }
                        else
                        {
                            operation[i] =3;
                        }
                    }  
                }
                else
                {   

                    if(operation[i] == 1)   //Writing into Data Server
                    {
                        strcpy(oup_fl_nm,fl_nms[i]);
                        int l = 0;
                        l = strlen(oup_fl_nm);
                        oup_fl_nm[l-1] = '\0';
                        flag = 0;
                        strcat(oup_fl_nm,".txt");
                        //printf("File name = :%sXXX\n",oup_fl_nm);
                        fds[i] = open(oup_fl_nm, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);

                        numWrite = send(sockfd,oup_fl_nm,strlen(oup_fl_nm),0);
                        if(numWrite < 0)
                        {
                            printf("Error in sending data to client\n");
                        }
                        else
                        {
                            //printf("Num Write to file = %ld\n",numWrite);
                        }               

                        while((numRead = read(sockfd,recvd_data,MAX_SIZE)) > 0)
                        {
                            recvd_data[numRead] = '\0';
                            if(flag == 0)
                            {
                                flag = 1;
                            }
                            //printf("NumRead = %ld\n",numRead);
                            numWrite =  write(fds[i],recvd_data,numRead);
                            //printf("Num Write to file = %ld\n",numWrite);
                            //printf("%s\n",recvd_data);
                            memset(recvd_data, 0, (MAX_SIZE+1) * sizeof(char));
                        }
                        if(flag == 0)
                        {
                            close(sockfd);
                            FD_CLR(sockfd, &allset);
                            client[i] = -1;
                            operation[i]=0;
                            is_type_set[i]=0;
                            close(fds[i]);
                            printf("A client connection is closed\n");
                        }
                    }
                    else if(operation[i] == 2)
                    {
                        memset(recvd_data, 0, (MAX_SIZE+1) * sizeof(char));
                        numRead = recv(sockfd,recvd_data,MAX_SIZE,0);
                        if(numRead == 0)
                        {
                            close(sockfd);
                            FD_CLR(sockfd, &allset);
                            client[i] = -1;
                            operation[i]=0;
                            is_type_set[i]=0;
                            close(fds[i]);
                            printf("A client connection is closed\n");
                        }
                        else
                        {
                            recvd_data[numRead]='\0';
                            printf("numRead = %ld\n",numRead);
                            printf("The file data to be read is %sXXX\n",recvd_data);

                            fl_fd = open(recvd_data,O_RDWR,S_IRUSR | S_IWUSR);
                            if(fl_fd < 1)
                            {
                                printf("Error opening a file in data server 1\n");
                                //exit(0);
                            }
                            else
                            {
                                while((numRead = read(fl_fd,buffer,MAX_SIZE)) > 0)
                                {
                                    //printf("YES\n");
                                    buffer[numRead]='\0';
                                    if(FD_ISSET(sockfd, &writefds))
                                    {
                                        //printf("YES2\n");
                                        numWrite = send(sockfd,buffer,strlen(buffer),0);
                                        memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));
                                    }
                                }
                                close(sockfd);
                                FD_CLR(sockfd, &allset);
                                client[i] = -1;
                                operation[i]=0;
                                is_type_set[i]=0;
                                close(fds[i]);
                                printf("A client connection is closed\n");
                            }
                        }

                    }
                    else
                    {
                        memset(recvd_data, 0, (MAX_SIZE+1) * sizeof(char));
                        numRead = recv(sockfd,recvd_data,MAX_SIZE,0);
                        if(numRead > 0)
                        {
                            recvd_data[numRead]='\0';
                            printf("numRead = %ld\n",numRead);
                            printf("The file data to be deleted is %sXXX\n",recvd_data); 
                            if(remove(recvd_data) != 0)
                            {
                                printf("Error deleting the file %s\n",recvd_data);
                            }
                            else
                            {
                                printf("Succesfully deleted file %s\n",recvd_data);
                            } 
                        }
                        else
                        {
                            printf("Unable to read file name from the client\n");
                        }
                        
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = -1;
                        operation[i]=0;
                        is_type_set[i]=0;
                        close(fds[i]);
                        printf("A client connection is closed\n");
                    }
                }
            }
            if(--nready <= 0)
            {
                break;
            }
        }
    }

}
