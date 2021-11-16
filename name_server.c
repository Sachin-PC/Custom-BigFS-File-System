#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/msg.h>
#include<mqueue.h>
#include<errno.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<sys/ipc.h>

#include <time.h>
#include <poll.h>


#define MAX_READ 2048
#define MAX_SIZE 2048

#define UPDATEMETADATA 6
#define REMOVE 8


void getdirectory_and_file_name(char *src_path,char dir_path[MAX_SIZE],char file_nm[MAX_SIZE], int x)
{
    printf("Inside get Direcory. Client path = %sXXX\n",src_path);
    int no_of_args=0,i=0;
    char *fldr[100];
    char *a_ptr = strtok(src_path, "/");
    while (a_ptr != NULL)
    {
        fldr[no_of_args] = a_ptr;
        a_ptr = strtok(NULL, "/");
        no_of_args++;
    }
    fldr[no_of_args]=NULL;
    for(i=0;i<no_of_args;i++)
    {
        printf("%s\n",fldr[i]);
    }
    memset(dir_path, 0, (MAX_SIZE) * sizeof(char));
    if(x == 2)
    {
        strcpy(dir_path,"/");   
    }
    for(i=0;i<no_of_args-2;i++)
    {
        strcat(dir_path,fldr[i]);
        strcat(dir_path,"/");
    }
    strcat(dir_path,fldr[i]);
    strcpy(file_nm,fldr[no_of_args-1]);

}



int check_client(int sockfd , char *bfr)
{
    int check=0;
    printf("The client name is %s\n",bfr);
    char cwrkd[MAX_SIZE];
    if(chdir(bfr) != 0)
    {
        printf("Error while changing the directory\n");
        check = mkdir(bfr,0777);
        if(check == -1)
        {
            printf("Error creating a directory\n");
            exit(0);
        }
        else
        {
            printf("Client directory created\n");
            if(chdir(bfr) != 0)
            {
                printf("Cannto change to newly created directory\n");
                return -1;
            }
        }
    }
    printf("Server's current working directory : %s\n", getcwd(cwrkd, 100));
    return 0;
}

int create_directory(int sockfd, char path[100])
{
    char cwrkd[100];
    int no_of_args=0,i=0;
    char *folder[100];
    char *a_ptr = strtok(path, "/");
    while (a_ptr != NULL)
    {
        folder[no_of_args] = a_ptr;
        a_ptr = strtok(NULL, "/");
        no_of_args++;
    }
    folder[no_of_args]=NULL;
    for(i=0;i<no_of_args;i++)
    {
        printf("%s\n",folder[i]);
    }

    for(i=0;i<no_of_args;i++)
    {
        if(chdir(folder[i]) != 0)
        {
            printf("Error while changing the directory to %s\n",folder[i]);
            break;
            //exit(0);
        }
    }
    for(i;i<no_of_args;i++)
    {
        printf("i = %d\n",i);
        if (mkdir(folder[i], 0777) == -1)
        {
            printf("Error creating direcory\n");
            return -1;
        }
        else
        {
            if(chdir(folder[i]) != 0)
            {
                printf("Not able to chage the directory\n");
                return -1;
            }
        }
    }

    printf("Server's current working directory : %s\n", getcwd(cwrkd, 100));
    return 0;
}


int perform_copy(int sockfd , char cmd_directory[MAX_SIZE])
{
    int x=0;
    char cwrkd[100];
    x = create_directory(sockfd,cmd_directory);
    if(x == -1)
    {
        //close(sockfd);
        return -1;
    }
    else
    {
        if(chdir("/home/sachin/Documents/Servers/Name Server") != 0)
        {
            printf("Error while changing back the  directory to Name Server\n");
            exit(0);
        }
        else
        {
            printf("Chander directory syccesfully to %s\n",getcwd(cwrkd,100));
        }
                                
    }
    return 0;
}


int create_metadata(char f_path[MAX_SIZE] , char meta_data[MAX_SIZE])
{
    printf("Creating a Meta Data File\n");
    int tfd = 0,n=0;
    ssize_t numWrite=0;
    printf("f_path = %sXXX\n",f_path);
    tfd = open(f_path,O_RDWR | O_CREAT | O_TRUNC ,S_IRUSR | S_IWUSR);
    if(tfd == -1)
    {
        printf("Unable to open file to write meta data\n");
        return -1;
    }
    else
    {
        numWrite =  write(tfd,meta_data,strlen(meta_data));
        if(numWrite <= 0)
        {
            printf("Unable to write meta data into a file\n");
            return -1;
        }
    }
    return 0;
}

int fetch_metadata(char f_path[MAX_SIZE] , char buffer[MAX_SIZE])
{
    printf("Fetching the Meta Data File\n");
    int tfd = 0,n=0;
    ssize_t numRead=0;
    printf("Meta Data file path = %s\n",f_path);
    tfd = open(f_path,O_RDWR ,S_IRUSR | S_IWUSR);
    if(tfd == -1)
    {
        printf("Unable to open file to fetch meta data\n");
        return -1;
        //exit(0);
    }
    else
    {
        numRead =  read(tfd,buffer,MAX_SIZE);
        if(numRead <= 0)
        {
            printf("Theres was error reading the meta data\n");
            return -1;
            //exit(0);
        }
        else
        {
            buffer[numRead]='\0';
            //printf("numRead = %ld\n",numRead);
            printf("The meta data is %sXXX\n",buffer);
        }
    }
    return 0;
}




void perform_ls(int sockfd ,int p[2], char arg1[MAX_SIZE], char arg2[MAX_SIZE])
{
    int chid=0;
    ssize_t numRead=0,numWrite=0;

    printf("Trying to execute ls\n");
    chid =0;

    chid = fork();
    if(chid == 0)
    {   
        //char arg[MAX_SIZE];
        int x=0;
        char *lsts[10];
        //strcpy(arg,command_info[1]);
        lsts[0]="ls";
        if(arg1[0] == 0)
        {
            lsts[1]=NULL;
        }
        else
        {
            lsts[1]=arg1;
        }
        if(arg2[0] == 0)
        {
            lsts[2]=NULL;
            if((strlen(arg1) == 1) && arg1[0] =='/')
            {
                lsts[1]=NULL;
            } 
        }
        else
        {
            if((strlen(arg2) == 1) && arg2[0] =='/')
            {
                lsts[2]=NULL;
            } 
            else
            {
                lsts[2]=arg2;
            }
        }
        lsts[3]=NULL;

        printf("lsts[0] = %s\n",lsts[0]);
        printf("lsts[1] = %s\n",lsts[1]);
        printf("lsts[2] = %s\n",lsts[2]);
        close(p[0]);
        close(1);
        dup2(p[1],1);
        close(p[1]);

        x = execvp(lsts[0],lsts);
        printf("Error executing ls\n");
        exit(0);
    }
    else
    {
        wait(NULL);
    }
}

void perform_moveFolder(char arg1[MAX_SIZE], char arg2[MAX_SIZE])
{
    int chid=0;
    ssize_t numRead=0,numWrite=0;

    printf("Trying to execute Move over folder\n");
    chid =0;

    chid = fork();
    if(chid == 0)
    {   
        //char arg[MAX_SIZE];
        int x=0;
        char *lsts[10];
        //strcpy(arg,command_info[1]);
        lsts[0]="mv";
            
        lsts[1]=arg1;
        lsts[2]=arg2;
        lsts[3]=NULL;
        printf("lsts[0] = %s\n",lsts[0]);
        printf("lsts[1] = %s\n",lsts[1]);
        printf("lsts[2] = %s\n",lsts[2]);
        x = execvp(lsts[0],lsts);
        printf("Error executing ls\n");
        exit(0);
    }
    else
    {
        wait(NULL);
    }
}

void perform_move(int sockfd ,char source[MAX_SIZE], char destination[MAX_SIZE])
{
    int fd1=0,fd2=0;
    ssize_t numRead=0,numWrite=0;
    char dir_path[MAX_SIZE],file_nm[MAX_SIZE],bfr[MAX_SIZE+1];
    char file_p[MAX_SIZE],src_fl[MAX_SIZE];
    char cwrkd[MAX_SIZE];

    printf("Executing Move operation\n");
    printf("Sorec path = %sXXX\n",source);
    printf("Destination path = %sXXX\n",destination);

    strcpy(file_p,destination); //Becuase when destination is passed to other function, It will be changed
    strcpy(src_fl,source);
    fd1 = open(source,O_RDWR ,S_IRUSR | S_IWUSR);
    if(fd1 < 0)
    {
        printf("Error opening the source file\n");
    }
    else
    {
        getdirectory_and_file_name(destination,dir_path,file_nm,0);
        printf("Directory path = %sXXX\n",dir_path);
        printf("file name = %sXXX\n",file_nm);

        create_directory(sockfd,dir_path);

        if(chdir("/home/sachin/Documents/Servers/Name Server") != 0)
        {
            printf("Error while changing the directory\n");
            exit(0);
        }

        printf("Dest path = %sXXX\n",file_p);
        fd2 = open(file_p,O_RDWR | O_CREAT | O_TRUNC ,S_IRUSR | S_IWUSR);
        if(fd2 < 0)
        {
            printf("Error creating the destination file\n");
        }
        else
        {
            while((numRead = read(fd1,bfr,MAX_SIZE)) > 0)
            {
                bfr[numRead]='\0';
                numWrite = write(fd2,bfr,numRead);
                if(numWrite == -1)
                {
                    printf("Error writing into the dstination file\n");
                    return;
                }
            }
            if (remove(src_fl) != 0)
            { 
              printf("Unable to delete Source file\n"); 
              return;
            }
            printf("Succesfully Moved the file to the give destination\n");
        }
    }
}


int main()
{
    int fd , server_socket , client_socket , i=0 ,x=0,y=0,chid=0;
    int nfds=0 , maxi = 0 , nready=0 , sockfd=0 , maxfd=0;
    int client[FD_SETSIZE],client_dir_check[FD_SETSIZE];
    fd_set readfds, writefds,allset;
    char buffer[MAX_READ + 1] , server_message[MAX_READ + 1],client_name[MAX_SIZE+1];
    char *clts_nms[FD_SETSIZE];
    char command_info[3][1000];
    ssize_t numRead,numWrite,clilen;
    struct sockaddr_in server_address, cliaddr;
    struct pollfd *pollFd;
    struct timeval timeout;
    struct timeval *pto;

    pto = &timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;


    char cwrkd[MAX_SIZE];
    if(chdir("/home/sachin/Documents/Servers/Name Server") != 0)
    {
        printf("Error while changing the directory\n");
        exit(0);
    }
    printf("Server's current working directory : %s\n", getcwd(cwrkd, 100));


    server_socket = socket(AF_INET,SOCK_STREAM,0);
    bzero(&server_address,sizeof(server_address));
    server_address.sin_family= AF_INET;
    server_address.sin_port = htons(9000);
    if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr.s_addr) <= 0)
    {
        printf("Error Converting IP address from presentation to network from\n");
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
                    client_dir_check[i]=0;
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
            if((sockfd = client[i]) <0)
            {
                continue;
            }
            if(FD_ISSET(sockfd, &readfds))
            {
                    memset(command_info, 0, (3000) * sizeof(char));
                    memset(buffer,0,(MAX_SIZE)*sizeof(char));
                    numRead = recv(sockfd, command_info, 3000, MSG_DONTWAIT);
                    //numRead = read(sockfd,command_info,1000);
                    //printf("The data received by client with socket id is %s\n",buffer);
                    //printf("numread = %ld\n",numRead);
                    if(numRead == 0)
                    {
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = -1;
                        printf("A client connection is closed\n");
                    }
                    else
                    {
                        printf("command_info[0] = %sXXX\n",command_info[0]);
                        printf("command_info[1] = %sXXX\n",command_info[1]);
                        printf("command_info[2] = %sXXX\n",command_info[2]);
                        if(strcmp(command_info[0],"1")==0)
                        {
                            y = perform_copy(sockfd , command_info[1]);
                            if(y == -1)
                            {    
                                close(sockfd);
                                FD_CLR(sockfd, &allset);
                                client[i] = -1;
                                printf("A client connection is closed\n");
                            }
                        }
                        else if((strcmp(command_info[0],"2") == 0) || (strcmp(command_info[0],"8") == 0) || (strcmp(command_info[0],"5") == 0))
                        {
                            y = fetch_metadata(command_info[1],buffer);
                            if(y == -1)
                            {
                                close(sockfd);
                                FD_CLR(sockfd, &allset);
                                client[i] = -1;
                                printf("A client connection is closed\n");
                            }
                            else
                            {    
                                printf("The meta data to be sent back to the client is %s\n",buffer);
                                if(FD_ISSET(sockfd, &writefds))
                                {
                                    //printf("Yes, Inside isset of writefds\n");
                                    numWrite = send(sockfd,buffer,strlen(buffer),MSG_DONTWAIT);
                                    if(numWrite == 0)
                                    {
                                        printf("Error sending meta data to client\n");
                                    }
                                    else
                                    {
                                        if((strcmp(command_info[0],"8") == 0))
                                        {
                                            printf("numWrite of meta data = %ld\n",numWrite);
                                            if(remove(command_info[1]) != 0)
                                            {
                                                printf("Unable to delete meta_data\n");
                                            }
                                            else
                                            {
                                                printf("meta data delete succesfully");
                                            }
                                        }
                                        else
                                        {
                                            printf("Meta Data Sent succesfully\n");
                                        }
                                    }
                                }
                                else
                                {
                                    printf("client with sockfd %d is not writable\n",sockfd);
                                }

                                /*if((strcmp(command_info[0],"2") == 0))
                                {
                                    close(sockfd);
                                    FD_CLR(sockfd, &allset);
                                    client[i] = -1;
                                    printf("A client connection is closed\n");
                                }*/
                            }
                        }
                        else if(strcmp(command_info[0],"3")==0)
                        {
                            perform_move(sockfd ,command_info[1],command_info[2]);
                            /*close(sockfd);
                            FD_CLR(sockfd, &allset);
                            client[i] = -1;
                            printf("A client connection is closed\n");*/
                        }
                        else if(strcmp(command_info[0],"4")==0)
                        {
                                int p[2];
                                pipe(p);
                                
                                perform_ls(sockfd,p,command_info[1],command_info[2]);
                                
                                memset(buffer,0,(MAX_SIZE)*sizeof(char));
                                close(p[1]);
                                while((numRead = read(p[0],buffer,MAX_SIZE)) > 0)
                                {
                                    if(FD_ISSET(sockfd, &writefds))
                                    {
                                        numWrite = send(sockfd,buffer,strlen(buffer),MSG_DONTWAIT);
                                        if(numWrite == 0)
                                        {
                                            printf("Error sending ls data to client\n");
                                        }
                                    }
                                }

                                close(sockfd);
                                FD_CLR(sockfd, &allset);
                                client[i] = -1;
                                printf("A client connection is closed\n");
                        }
                        else if(strcmp(command_info[0],"6")==0)
                        {
                            y = create_metadata(command_info[1],command_info[2]);
                            
                            //In Future, For all errors on Name server side, Send ack to the client
                            if(y == -1)
                            {
                                printf("Creating MetaData Failed\n");
                            }
                            else
                            {
                                printf("MetaData File created Succesfully\n");
                            }
                        }
                        else if(strcmp(command_info[0],"9")==0)
                        {
                            //managebbcpy(command_info[1])
                            char dir_p[MAX_SIZE],fl_n[MAX_SIZE],des[MAX_SIZE];
                            strcpy(des,command_info[1]);
                            getdirectory_and_file_name(des,dir_p,fl_n,0);
                            printf("Directory path = %sXXX\n",dir_p);
                            printf("file name = %sXXX\n",fl_n);

                            create_directory(sockfd,dir_p);

                            if(chdir("/home/sachin/Documents/Servers/Name Server") != 0)
                            {
                                printf("Error while changing the directory\n");
                                exit(0);
                            }

                            //printf("Dest path = %sXXX\n",file_p);

                            y = create_metadata(command_info[1],command_info[2]);
                            
                            //In Future, For all errors on Name server side, Send ack to the client
                            if(y == -1)
                            {
                                printf("Creating MetaData Failed\n");
                            }
                            else
                            {
                                printf("MetaData File created Succesfully\n");
                            }
                        }
                        else if(strcmp(command_info[0],"11")==0)
                        {
                            printf("About to perform move over folder");
                            perform_moveFolder(command_info[1],command_info[2]);
                        }
                    }
                if(--nready <= 0)
                {
                    break;
                }
            }
        }
    }

}
