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
#include<math.h>
#include<string.h>
#include <dirent.h>

#define MAX_SIZE 2048
#define BLOCK_SIZE 1024000
#define NUM_OF_SERVERS 3
#define NAME_SERVER_PORT 9000
#define COPYLTB 1
#define COPYBTL 2
#define MOVE 3
#define LS 4
#define CAT 5
#define CREATEMETADATA 6
#define GETMETADATA 7
#define REMOVE 8
#define BTB 9

struct message {
    long mtype;
    char mtext[MAX_SIZE];
};


void getdirectory_and_file_name(char *source_path,char directory_path[MAX_SIZE],char file_name[MAX_SIZE], int x , int is_folder)
{
    int is_root=0;
    printf("Inside get Direcory. Client path = %sXXX\n",source_path);
    int no_of_args=0,i=0;
    memset(directory_path, 0, (MAX_SIZE) * sizeof(char));
    if(source_path[0] == '/')
    {
        is_root = 1;
        strcpy(directory_path,"/");   
    }


    char *folder[100];
    char *a_ptr = strtok(source_path, "/");
    while (a_ptr != NULL)
    {
        folder[no_of_args] = a_ptr;
        a_ptr = strtok(NULL, "/");
        no_of_args++;
    }
    folder[no_of_args]=NULL;
    printf("no_of_args = %d\nis_folder = %d\n",no_of_args,is_folder);
    for(i=0;i<no_of_args;i++)
    {
        printf("%s\n",folder[i]);
    }
    /*if(x == 2)
    {
        strcpy(directory_path,"/");   
    }*/
    if(no_of_args == 0)
    {
        file_name[0]=0;
    }
    else if(no_of_args == 1)
    {
        if(is_folder == 1)
        {
            strcat(directory_path,folder[0]);
            file_name[0]=0;
        }
        else
        {
            if(is_root == 0)
            {
                directory_path[0]=0;
            }
            strcpy(file_name,folder[0]);
        }
    }
    else if(no_of_args >= 2)
    {
        for(i=0;i<no_of_args-2;i++)
        {
            strcat(directory_path,folder[i]);
            strcat(directory_path,"/");
        }
        strcat(directory_path,folder[i]);
        if(is_folder == 1)
        {
            strcat(directory_path,"/");   
            strcat(directory_path,folder[no_of_args-1]);
            file_name[0]=0;
        }
        else
        {
            strcpy(file_name,folder[no_of_args-1]);
        }
    }

}




void delete_tempFiles()
{
    int is_fail =0;
    printf("Deleting the Temporary Files creted\n");
    if (remove("temp1.txt") != 0)
    { 
      printf("Unable to delete temp1.txt"); 
      is_fail = 1;
    }
    if (remove("temp2.txt") != 0)
    {
      printf("Unable to delete temp2.txt"); 
      is_fail = 1;
    }
    if (remove("temp3.txt") != 0)
    {
      printf("Unable to delete temp3.txt"); 
      is_fail = 1;
    }
    if(is_fail == 0)
    {
        printf("All temporary files deleted succesfully\n");
    }
}

int create_directory(char path[MAX_SIZE])
{
    if(path[0] =='/')   //If it is a root path
    {
        if(chdir("/") != 0)
        {
            printf("Error while changing the directory to root\n");
            return -1;
        }
    }


    char cwrkd[MAX_SIZE];
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
        }
    }
    for(i;i<no_of_args;i++)
    {
        //printf("i = %d\n",i);
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

    //printf("Server's current working directory : %s\n", getcwd(cwrkd, 100));
    return 0;
}



int check_file_size(char *file_name , int server_set[NUM_OF_SERVERS])
{
    int fd1=0 , num_of_blcks=0,no_of_sets , rem=0 , i=0;
    ssize_t tot_read=0,file_sz=0;
    char file_data[MAX_SIZE];
    printf("The file name = %sXXX\n",file_name);
    fd1 = open(file_name, O_RDONLY , S_IRUSR | S_IWUSR);
    if(fd1 == -1)
    {
        printf("Error while opening a file\n");
        exit(0);
    }
    while((tot_read = read(fd1,file_data,MAX_SIZE)) > 0)
    {
        file_sz = file_sz + tot_read;
    }
    printf("The total size of the file is = %ldXXX\n",file_sz);


    num_of_blcks = file_sz/BLOCK_SIZE;
    if((file_sz % BLOCK_SIZE) != 0)
    {
        num_of_blcks++;
    }
    printf("Number of Blocks = %d\n",num_of_blcks);
    //printf("num of blocks of 2KB needed is = %ld\n",num_of_blcks);
    no_of_sets = num_of_blcks/NUM_OF_SERVERS;
    for(i=0;i<NUM_OF_SERVERS;i++)
    {
        server_set[i] = no_of_sets*500;
    }
    rem = num_of_blcks%NUM_OF_SERVERS;
    i=0;
    while(rem > 0)
    {
        server_set[i] = server_set[i]+500;
        i++;
        rem--;
    }
    return fd1;
}


void create_temporary_files(int fds[NUM_OF_SERVERS],int k)
{
    int i=0;
    char fname[MAX_SIZE];
    char filenum[MAX_SIZE];
    for(i=0;i<NUM_OF_SERVERS;i++)
    {
        strcpy(fname,"temp");
        sprintf(filenum,"%d",i+1);
        strcat(fname,filenum);
        strcat(fname,".txt");
        //printf("Temporary file name = %s\n",fname);
        fds[i] = open(fname,O_RDWR | O_CREAT | O_TRUNC ,S_IRUSR | S_IWUSR);
        if(fds[i] < 0)
        {
            printf("Error while creating temporary file\n");
            exit(0);
        }
    }
}


int write_into_temporary_files(int fd1 , int fds[NUM_OF_SERVERS] , int server_set[NUM_OF_SERVERS])
{
        int i=0,blocks_to_read=0,file_fd=0,block_count=0;
        char file_data[MAX_SIZE];
        ssize_t tot_read=0,tot_write=0;
        memset(file_data, 0, (MAX_SIZE+1) * sizeof(char));
        lseek(fd1,0,0);
        for(i=0;i<NUM_OF_SERVERS;i++)
        {
            if(server_set[i] == 0)
            {
                printf("returning %d\n",i);
                return i;
            }
            blocks_to_read = server_set[i];
            file_fd = fds[i];
            block_count=0;
            while((tot_read = read(fd1 , file_data , MAX_SIZE)) > 0)
            {
                //printf("total_read = %ld\n",tot_read);
                file_data[tot_read] = '\0';
                //tot_read = tot_read+1;
                tot_write = write(file_fd,file_data,tot_read);
                if(tot_write == -1)
                {
                    printf("Error while writing to a file\n");
                    exit(0);
                }
                //printf("total write = %ld\n",tot_write);
                block_count++;
                if(block_count == blocks_to_read)
                {
                    break;
                }
                memset(file_data, 0, (MAX_SIZE+1) * sizeof(char));
            }
            //printf("Total Blocks written = %d\n",block_count);
        }
        return NUM_OF_SERVERS;
}


int input_command(char client_name[MAX_SIZE], char source_path[MAX_SIZE] , char destination_path[MAX_SIZE], char cat_files[10][500] , int *is_folder)
{
    int len=0,i=0, command_type =0,flag=0;
    char command[MAX_SIZE], cwrkd[MAX_SIZE],fl[MAX_SIZE],directory_path[MAX_SIZE],file_name[MAX_SIZE],src[MAX_SIZE];
    printf("Enter the command\n");
    fgets(command, MAX_SIZE, stdin); 
    len = strlen(command);
    command[len-1]='\0';
    printf("The command is %sXXX\n",command);

    if(strcmp(command,"ls") == 0)
    {
        command_type = 4;
        return command_type;
    }
    int no_of_args=0;
    char *args[100];
    char *a_ptr = strtok(command, " ");
    while (a_ptr != NULL)
    {
        args[no_of_args] = a_ptr;
        a_ptr = strtok(NULL, " ");
        no_of_args++;
    }
    args[no_of_args]=NULL;

    if(no_of_args < 1)
    {
        printf("Please Eneter a valid command\n");
        exit(0);
    }

    strcpy(source_path,args[1]);
    i = strlen(source_path)-1;
    printf("i = %d\t source_path[i] = %cXX\n",i,source_path[i]);
    while(i>=0 && source_path[i] != '/')
    {
        printf("source[%d] = %cXX\n",i,source_path[i]);
        if(source_path[i] =='.')
        {
            flag = 1;
            break;
        }
        i--;
    }

    if(flag == 1)
    {
        *is_folder = 0;
    }
    else
    {
        *is_folder = 1;
    }


    strcpy(src,args[1]);
    //if(strcmp(args[0],"cp") == 0)
    
    //strcpy(source_path,cwrkd);
    

    //return 0;


    for(i=0;i<no_of_args;i++)
    {
        printf("args[%d] = %sXXX\n",i,args[i]);
    }
    if(no_of_args == 1 || no_of_args == 2)
    {
        if(strcmp(args[0],"ls") == 0)
        {
            command_type = 4;
            if(no_of_args == 1)
            {
                //source_path[0]='/';
            }
            else
            {
                strcpy(source_path,args[1]);
            }
            return command_type;
        }
        else if(no_of_args == 2)
        {
            if(strcmp(args[0],"rm") == 0)
            {
                command_type = REMOVE;
                strcpy(source_path,args[1]);
                return command_type;
            }
            else if(strcmp(args[0],"cat") ==0)
            {
                command_type = CAT; 
                strcpy(cat_files[0],args[1]);
                cat_files[1][0]=0;
                return command_type;

            }
            else
            {
                printf("Please Eneter a valid command\n");
                exit(0);                
            }
        }
        else    
        {

            printf("Please Eneter a valid command\n");
            exit(0);
        }
    }
    else if(strcmp(args[0],"ls") == 0)
    {
        command_type = 4;
        strcpy(source_path,args[1]);
        strcpy(destination_path,args[2]);
        return command_type;
    }
    else if(strcmp(args[0],"cp") == 0)
    {
        //printf("It is a cp command\n");
        if(strcmp(args[2],"bigfs") == 0)
        {
            command_type = 1;

            getdirectory_and_file_name(src,directory_path,file_name,2,*is_folder);
            printf("Insied Input command\nThe source path is %s\nThe directory paths is %s\n",source_path,directory_path);
            printf("The file name is %s\n",file_name);  
            if(directory_path[0] == 0 )
            {
                getcwd(directory_path, MAX_SIZE);
            }
            printf("The directory paths is %s\n",directory_path);
            if(chdir(directory_path) != 0)
            {
                printf("Error while changing the directory\n");
                exit(0);
            }
            printf("Server's current working directory : %sXXX\n", getcwd(cwrkd, 100));
            if(*is_folder == 0)
            {
                if(strlen(cwrkd) > 1)
                {
                    strcat(cwrkd,"/");
                }
                strcat(cwrkd,file_name);
            }



            printf("Its a command to copy from local machine to bigfs\n");
            strcpy(source_path,cwrkd);
            strcpy(destination_path,client_name);
            printf("The client path is %s\n",source_path);
            strcat(destination_path,cwrkd);
        }
        else if((strlen(args[1]) >= 6 && (args[1][0] =='/' && args[1][1] =='b' && args[1][2] =='i' && args[1][3] =='g' && args[1][4] =='f' && args[1][5] =='s')) && (strlen(args[2]) >= 6 && (args[2][0] =='/' && args[2][1] =='b' && args[2][2] =='i' && args[2][3] =='g' && args[2][4] =='f' && args[2][5] =='s')))
        {
            printf("It is a command to copy from bigfs to bigfs machine\n");
            command_type = BTB;
            strcpy(source_path,args[1]+7);
            strcpy(destination_path,args[2]+7);
            printf("Sourcec path = %s\nDestination path = %s\n",source_path,destination_path);
            return command_type;
        } 
        else if(strlen(args[2]) >= 6 && (args[2][0] =='/' && args[2][1] =='b' && args[2][2] =='i' && args[2][3] =='g' && args[2][4] =='f' && args[2][5] =='s'))
        {

            if(strlen(args[2]) < 6)
            {
                printf("Command not identidied. Please Enter a valid Command\n");
                exit(0);
            }
            else
            {
                if(args[2][0] =='/' && args[2][1] =='b' && args[2][2] =='i' && args[2][3] =='g' && args[2][4] =='f' && args[2][5] =='s')
                {
                    printf("It is a command to copy from local to bigfs machine\n");
                    command_type = 1;

                    getdirectory_and_file_name(src,directory_path,file_name,2,*is_folder);
                    printf("Insied Input command\nThe source path is %s\nThe directory paths is %s\n",source_path,directory_path);
                    printf("The file name is %s\n",file_name);  
                    if(directory_path[0] == 0 )
                    {
                        getcwd(directory_path, MAX_SIZE);
                    }
                    printf("The directory paths is %s\n",directory_path);
                    if(chdir(directory_path) != 0)
                    {
                        printf("Error while changing the directory\n");
                        exit(0);
                    }
                    printf("Server's current working directory : %sXXX\n", getcwd(cwrkd, 100));
                    if(*is_folder == 0)
                    {
                        if(strlen(cwrkd) > 1)
                        {
                            strcat(cwrkd,"/");
                        }
                        strcat(cwrkd,file_name);
                    }

                    strcpy(source_path,cwrkd);
                    strcpy(destination_path,args[2]+7);
                    printf("Sourcec path = %s\nDestination path = %s\n",source_path,destination_path);
                    return command_type;
                }
                else
                {
                    printf("Please Enter a valid command\n");
                    exit(0);
                }

            }
            return command_type;
        }
        else
        {
            if(strlen(args[1]) < 6)
            {
                printf("Command not identidied. Please Enter a valid Command\n");
                exit(0);
            }
            else
            {
                if(args[1][0] =='/' && args[1][1] =='b' && args[1][2] =='i' && args[1][3] =='g' && args[1][4] =='f' && args[1][5] =='s')
                {
                    printf("It is a command to copy from bigfs to local machine\n");
                    command_type = 2;
                    strcpy(source_path,args[1]+7);
                    strcpy(destination_path,args[2]);
                    printf("Sourcec path = %s\nDestination path = %s\n",source_path,destination_path);
                }
                else
                {
                    printf("Please Enter a valid command\n");
                    exit(0);
                }

            }
        }
    }
    else if(strcmp(args[0],"mv") == 0)
    {
        printf("Its a move command\n");
        command_type = 3;
        strcpy(source_path,args[1]);
        strcpy(destination_path,args[2]);
        return command_type;
    }
    else if(strcmp(args[0],"cat") == 0)
    {
        printf("Its a CAT command\n");
        command_type = CAT;
        //strcpy(cat_files[0],args[1]);
        //strcpy(cat_files[1],args[2]);
        //cat_files[2][0] = 0;
        for(i=0;i<no_of_args-1;i++)
        {
            strcpy(cat_files[i],args[i+1]);
        }
        cat_files[i][0] = 0;
    }
    return command_type;
}

int create_client(char *ip , int port_num)
{
    int network_socket;

    network_socket = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_address;
    bzero(&server_address,sizeof(server_address));
    server_address.sin_family= AF_INET;
    server_address.sin_port = htons(port_num);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");     // htonl(server_address.sin_addr.s_addr);

    int connection_status = connect(network_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    if(connection_status == -1 )
    {
        printf("There was an error making a connection to the remote socket ");
        return connection_status;
        //exit(0);
    }
    return network_socket;
}


void output_file_into_terminal(int req_servers,int fds[NUM_OF_SERVERS])
{
    int i=0;
    ssize_t numWrite=0,numRead=0;
    char buffer[MAX_SIZE+1];

    for(i=0;i<req_servers;i++)
    {
        lseek(fds[i],0,0);          //Point all temporary files to the start of the file to read
    } 

    for(i=0;i<req_servers;i++)
    {
        while((numRead = read(fds[i],buffer,MAX_SIZE)) > 0)
        {
            buffer[numRead] = '\0';
            printf("%s",buffer);
            memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));
        }
    }
    printf("\n\n");
}





void copy_into_file(int req_servers,int fds[NUM_OF_SERVERS],char destination_path[MAX_SIZE])
{
    int i=0,pid=0,fl_id=0,is_dir_status=0,flag=0;
    int status=0;
    ssize_t numWrite=0,numRead=0;
    char buffer[MAX_SIZE+1];
    char cl_path[MAX_SIZE];
    char fl_name[MAX_SIZE],fl_path[MAX_SIZE];
    for(i=0;i<req_servers;i++)
    {
        lseek(fds[i],0,0);          //Point all temporary files to the start of the file to read
    } 
    strcpy(fl_path,destination_path);
    
    if(destination_path[0] == '/' )         //To detiremin if the destination path is from the root of current directory
    {
        flag = 2;           
    }
    else
    {
        flag =1;
    }
    getdirectory_and_file_name(destination_path,cl_path,fl_name,flag,0);
    printf("The directory path is %sXXX\n",cl_path);    //Excluding File Name
    printf("The file name is %sXXX\n",fl_name);

    pid = fork();
    if(pid == 0)
    {
        int x=0,newfl_fd=0;
        x = create_directory(cl_path);
        if(x == -1)
        {
            exit(-1);
        }
        else
        {
            exit(0);
        }
    }
    else
    {
        wait(&status);
        if(!WIFEXITED(status))
        {
            exit(0);
        }

        is_dir_status= WEXITSTATUS(status);
        printf("status = %d\n",is_dir_status);  //Check if any child exited with error
        if(is_dir_status == 255)
        {
            exit(0);    //Error in creating the directory
        }
        printf("The file path is %sXXX\n",fl_path);
        
        fl_id = open(fl_path,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);   //open the desttination file to copy data into
        
        if(fl_id == -1)
        {
            printf("Error opening the destination file\n");
            exit(0);
        }
        memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));
        for(i=0;i<req_servers;i++)
        {
            while((numRead = read(fds[i],buffer,MAX_SIZE)) > 0)
            {
                buffer[numRead] = '\0';
                numWrite = write(fl_id,buffer,numRead);
                if(numWrite < 0)
                {
                    printf("Error writing to destination file\n");
                    exit(0);
                }
                memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));
            }
        }
        printf("File is succesfully copied from Bigfs to Local Machine\n\n");
    }
}



int copy_to_bigfs(int network_socket , int command_type, char client_name[MAX_SIZE] , char source_path[MAX_SIZE], char destination_path[MAX_SIZE], int port_numbers[NUM_OF_SERVERS] , int msqid)
{


    int i=0,fd1=0,req_servers=0,chid=0,child_status=0,is_fail=0;
    int server_set[NUM_OF_SERVERS],fds[NUM_OF_SERVERS];
    ssize_t numRead =0,numWrite =0;
    char *fl_path;
    char dirc[MAX_SIZE];
    char cmd_tp[100],directory_path[MAX_SIZE],file_name[MAX_SIZE],meta_data[MAX_SIZE];
    char cmd_info[3][1000];
    //strcpy(destination_path,"xxxxxx");
    printf("The client file path which is copied to bigfs is %sXXX\n",source_path);
    strcpy(dirc,destination_path);

    getdirectory_and_file_name(destination_path,directory_path,file_name,0,0);
    printf("The directory path is %sXXX\n",directory_path);    //Excluding File Name
    printf("The file name is %sXXX\n",file_name);

    memset(cmd_info, 0, (3000) * sizeof(char));

    //Create the message to be sent to the Name Server
    sprintf(cmd_tp, "%d",command_type); 
    strcpy(cmd_info[0],cmd_tp);
    strcpy(cmd_info[1],directory_path);
    cmd_info[2][0] = 0;

    i=0;
    while(i < 3)
    {
        printf("cmd_inf[%d] = %s\n",i,cmd_info[i]);
        i++;
    }


    //Send Data to the Client[Command to be Executed Info]
    numWrite = send(network_socket, cmd_info, 3000,0);
    if(numWrite < 0)
    {
        printf("Error Sending data to the Name Server\n");
    }

    //Create File Path excluding client name to open the file needed
    //fl_path = directory_path + 7;
    //strcat(fl_path,"/");
    //strcat(fl_path,file_name);
    printf("Local file path to open the file = %s\n",source_path);

    fd1 = check_file_size(source_path,server_set);
    if(fd1 > 0)
    {
        create_temporary_files(fds,0);    //fds holds file descriptors of the created temporary files

        req_servers = write_into_temporary_files(fd1,fds,server_set);  //server set is holding the blocks to be written in the server data

        printf("Number of servers required to store Data = %d\n",req_servers);
        for(i=0;i<req_servers;i++)
        {
            chid = fork();
            if(chid == -1)
            {
                printf("Error which creating a child\n");
                exit(0);
            }
            else if(chid == 0)
            {
                int k=0,blcks_covered=0,port_num=0, sock_fd=0, count=0, nb=0;
                int server_socket,file_fd=0;
                char buffer[MAX_SIZE+1], data_ser_file_name[MAX_SIZE];
                char *ip = "127.0.0.1";

                nb = server_set[i];     //Number of Block should be stored in a Data Server
                port_num = port_numbers[i]; // Port number of a data server
                file_fd = fds[i];   //Temporary File File descriptor

                //Create Connection with Data Servers
                server_socket = create_client(ip,port_num);
                if(server_socket == -1)
                {
                    printf("of Data Server %d\n",i+1);
                    exit(0);
                }
                printf("Succesfully connected to Data Server %d\n",i+1);

                numWrite = write(server_socket,"1",1);
                if(numWrite <= 0)
                {
                    printf("Unable to write the data to the Data Server %d\n",i+1);
                }

                memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));
                lseek(file_fd,0,SEEK_SET);                              //Start Copying the data from the start of temporary file
                while((numRead = read(file_fd,buffer,MAX_SIZE)) >0)
                {
                    buffer[numRead] = '\0';
                    numWrite = write(server_socket,buffer,numRead);
                    if(numWrite <= 0)
                    {
                        printf("Unable to write the data to the Data Server %d\n",i+1);
                    }
                    memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));
                    count++;
                    //printf("Sent %d block to Data Server %d\n",count,i+1);
                }

                //Receive the file name in what data is stored in Data Server
                numWrite = recv(server_socket,data_ser_file_name,MAX_SIZE,0);
                if(numWrite < 0)
                {
                    printf("Error getting the file name from the data server\n");
                    //exit(0);
                }
                else
                {
                    //printf("Writing to message queue\n");
                    data_ser_file_name[numWrite]='\0';
                    struct message ser_msg;
                    strcpy(ser_msg.mtext,data_ser_file_name);
                    ser_msg.mtype=i+1;
                    if (msgsnd(msqid, &ser_msg, sizeof(long) + (strlen(ser_msg.mtext) * sizeof(char)) + 1, 0) == -1)
                    {
                        printf("Error in sending message\n");
                        perror("msgsnd");
                        //return EXIT_FAILURE;
                    }   
                }
                printf("All Data Transfered to Data Server %d.Now,Closing the connection with Data Server %d\n",i+1,i+1);
                close(server_socket);
                exit(0);
            }
        }

        while ((child_status = wait(NULL)) > 0);    //Wait untill all the data server connections are closed
        printf("All the child are terminated\n");

        for(i=0;i<req_servers;i++)
        {
            struct message recvd_msg;
            if(i != 0)
            {
                strcat(meta_data," ");
            }
            if (msgrcv(msqid, &recvd_msg, MAX_SIZE, i+1,0) == -1)
            {
                /*if(errno == 42)
                {
                    //printf("Path has not yet changed\n");
                }
                else*/
                {
                    perror("msgrcv");
                    return EXIT_FAILURE;
                }
            }
            else
            {
                printf("File name of data stored in data server %d is %s\n",i+1,recvd_msg.mtext);
                if(i ==0)
                {
                    strcpy(meta_data,recvd_msg.mtext);
                }
                else
                {
                    strcat(meta_data,recvd_msg.mtext);
                }
            }
        }
        printf("The metadat to be sent to the name server is %sXXX\n",meta_data);


        //Send this Meta data to name server to keep record of it
        strcpy(cmd_info[0],"6");        //6 indicates name server to store the meta_data
        strcpy(cmd_info[1],dirc);
        strcpy(cmd_info[2],meta_data);
        numWrite = send(network_socket, cmd_info, 3000,0);
        if(numWrite <= 0)
        {
            printf("Error writing the meta data into name server\n");
        }
        else
        {
            printf("Succesfully copied data from local machine to bigfs\n");
        }
        delete_tempFiles();
    } 
}



int managebtob(int network_socket,int command_type,int req_servers,int fds[NUM_OF_SERVERS],char destination_path[MAX_SIZE],int port_numbers[NUM_OF_SERVERS],int msqid)
{
    int i=0,chid=0,child_status=0;
    ssize_t numRead=0,numWrite=0;
    char cmd_info[3][1000];
    for(i=0;i<req_servers;i++)
    {
        chid = fork();
        if(chid == -1)
        {
            printf("Error which creating a child\n");
            exit(0);
        }
        else if(chid == 0)
        {
                int k=0,blcks_covered=0,port_num=0, sock_fd=0, count=0, nb=0;
                int server_socket,file_fd=0;
                char buffer[MAX_SIZE+1], data_ser_file_name[MAX_SIZE];
                char *ip = "127.0.0.1";

                //nb = server_set[i];     //Number of Block should be stored in a Data Server
                port_num = port_numbers[i]; // Port number of a data server
                file_fd = fds[i];   //Temporary File File descriptor

                //Create Connection with Data Servers
                server_socket = create_client(ip,port_num);
                if(server_socket == -1)
                {
                    printf("of Data Server %d\n",i+1);
                    exit(0);
                }
                printf("Succesfully connected to Data Server %d\n",i+1);

                numWrite = write(server_socket,"1",1);
                if(numWrite <= 0)
                {
                    printf("Unable to write the data to the Data Server %d\n",i+1);
                }

                memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));
                lseek(file_fd,0,SEEK_SET);                              //Start Copying the data from the start of temporary file
                while((numRead = read(file_fd,buffer,MAX_SIZE)) >0)
                {
                    buffer[numRead] = '\0';
                    numWrite = write(server_socket,buffer,numRead);
                    if(numWrite <= 0)
                    {
                        printf("Unable to write the data to the Data Server %d\n",i+1);
                    }
                    memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));
                    count++;
                    //printf("Sent %d block to Data Server %d\n",count,i+1);
                }

                //Receive the file name in what data is stored in Data Server
                numWrite = recv(server_socket,data_ser_file_name,MAX_SIZE,0);
                if(numWrite < 0)
                {
                    printf("Error getting the file name from the data server\n");
                    //exit(0);
                }
                else
                {
                    //printf("Writing to message queue\n");
                    data_ser_file_name[numWrite]='\0';
                    struct message ser_msg;
                    strcpy(ser_msg.mtext,data_ser_file_name);
                    ser_msg.mtype=i+1;
                    if (msgsnd(msqid, &ser_msg, sizeof(long) + (strlen(ser_msg.mtext) * sizeof(char)) + 1, 0) == -1)
                    {
                        printf("Error in sending message\n");
                        perror("msgsnd");
                        //return EXIT_FAILURE;
                    }   
                }
                printf("All Data Transfered to Data Server %d.Now,Closing the connection with Data Server %d\n",i+1,i+1);
                close(server_socket);
                exit(0);
        }
    }

        while ((child_status = wait(NULL)) > 0);    //Wait untill all the data server connections are closed
        printf("All the child are terminated\n");

        char meta_data[MAX_SIZE];
        for(i=0;i<req_servers;i++)
        {
            struct message recvd_msg;
            if(i != 0)
            {
                strcat(meta_data," ");
            }
            if (msgrcv(msqid, &recvd_msg, MAX_SIZE, i+1,0) == -1)
            {
                /*if(errno == 42)
                {
                    //printf("Path has not yet changed\n");
                }
                else*/
                {
                    perror("msgrcv");
                    return EXIT_FAILURE;
                }
            }
            else
            {
                printf("File name of data stored in data server %d is %s\n",i+1,recvd_msg.mtext);
                if(i ==0)
                {
                    strcpy(meta_data,recvd_msg.mtext);
                }
                else
                {
                    strcat(meta_data,recvd_msg.mtext);
                }
            }
        }
        printf("The metadat to be sent to the name server is %sXXX\n",meta_data);


        //Send this Meta data to name server to keep record of it
        strcpy(cmd_info[0],"9");        //6 indicates name server to store the meta_data
        strcpy(cmd_info[1],destination_path);
        strcpy(cmd_info[2],meta_data);
        printf("cmd_info[0] = %sxxx",cmd_info[0]);
        printf("cmd_info[1] = %sxxx",cmd_info[1]);
        printf("cmd_info[2] = %sxxx",cmd_info[2]);
        numWrite = send(network_socket, cmd_info, 3000,0);
        if(numWrite <= 0)
        {
            printf("Error writing the meta data into name server\n");
        }
        else
        {
            printf("Succesfully copied data from local machine to bigfs\n");
        }
        //delete_tempFiles(); 
}

void copy_to_localmachine(int network_socket,int command_type,char client_name[MAX_SIZE],char source_path[MAX_SIZE],char destination_path[MAX_SIZE] , int port_numbers[NUM_OF_SERVERS], int x)
{
        int i=0,chid =0,child_status=0;
        int fds[NUM_OF_SERVERS];
        char cmd_info[3][1000];
        char cmd_tp[100];
        char meta_data[MAX_SIZE];
        int req_servers = 0;
        ssize_t numRead=0,numWrite=0;
        memset(cmd_info, 0, (3000) * sizeof(char));
        memset(meta_data, 0, (MAX_SIZE) * sizeof(char));

        printf("The bigfs path from which the file has to be copied is %s\n",source_path);
        if(x == 1)
        {
            printf("The client path to which te file has to be copied is %s\n",destination_path);
        }


        //Create the message to be sent to the Name Server
        if(x == 2)
        {
            command_type = CAT;
        }
        sprintf(cmd_tp, "%d",command_type); 
        strcpy(cmd_info[0],cmd_tp);
        strcpy(cmd_info[1],source_path);
        cmd_info[2][0] = 0;

        numWrite = send(network_socket, cmd_info, 3000,0);
        if(numWrite < 0)
        {
            printf("Error Sending data to the Name Server\n");
        }

        numRead = recv(network_socket,meta_data,MAX_SIZE,0);
        if(numRead <= 0)
        {
            printf("Error reading meta data from name server\n");
            exit(0);
        }
        meta_data[numRead]='\0';
        printf("The meta data received from the name server is %s\n",meta_data);

        create_temporary_files(fds,0);

        int no_of_args=0;
        char *serv_fl_nams[100];
        char *a_ptr = strtok(meta_data, " ");
        while (a_ptr != NULL)
        {
            serv_fl_nams[no_of_args] = a_ptr;
            a_ptr = strtok(NULL, " ");
            no_of_args++;
        }
        serv_fl_nams[no_of_args]=NULL;
        req_servers = no_of_args;
        for(i=0;i<no_of_args;i++)
        {
            printf("Data server %d has required data in file %sXXX\n",i+1,serv_fl_nams[i]);
        }

        for(i=0;i<req_servers;i++)
        {
            chid = fork();
            if(chid == -1)
            {
                printf("Error which creating a child\n");
                exit(0);
            }
            else if(chid == 0)
            {
                int port_num=0,server_socket=0;
                char buffer[MAX_SIZE+1], data_ser_file_name[MAX_SIZE];
                char *ip = "127.0.0.1";

                port_num = port_numbers[i];
                //printf("port_num = %d\n",port_num);

                //Create Connection with Data Servers
                server_socket = create_client(ip,port_num);
                if(server_socket == -1)
                {
                    printf("of Data Server %d\n",i+1);
                    exit(0);
                }
                printf("Succesfully connected to Data Server %d\n",i+1);

                numWrite = send(server_socket,"2",1,0);         //Send information of command type[Read or write to data server] to data server
                if(numWrite <= 0)
                {
                    printf("Error writing to data server %d\n",i+1);
                    close(server_socket);
                    exit(0);
                }
                //printf("numWrite = %ld\n",numWrite);
                printf("File name needed from Data Server %d is %sXXX\n",i+1,serv_fl_nams[i]);
                numWrite = send(server_socket,serv_fl_nams[i],strlen(serv_fl_nams[i]),0);
                if(numWrite <= 0)
                {
                    printf("Error writing to data server %d\n",i+1);
                    close(server_socket);
                    exit(0);
                }
                //printf("numWrite = %ld\n",numWrite);

                while((numRead = recv(server_socket,buffer,MAX_SIZE,0)) > 0)
                {
                    buffer[numRead]='\0';
                    //printf("NumRead = %ld\n",numRead);
                    //printf("FDS[%d] = %d\n",i,fds[i] );
                    numWrite =  write(fds[i],buffer,numRead);
                    if(numWrite <= 0)
                    {
                        printf("Error writing into a temporary file %d\n",i+1);
                        close(server_socket);
                        exit(0);
                    }
                    //printf("Num Write to file = %ld\n",numWrite);
                    //printf("%s\n",recvd_data);
                    memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));   
                }
                printf("All Data needed is received from Data Server %d.Now,Closing the connection with Data Server %d\n",i+1,i+1);
                close(server_socket);
                exit(0);
            }
        }

        while((child_status =wait(NULL)) != -1);
        printf("All Data Server connections are closed\n");

        if(x == 1)
        {
            copy_into_file(req_servers,fds,destination_path);       //Copy the files fetched from data server into local machine as one single file
            delete_tempFiles();
        } 
        else if(x == 2)
        {
            output_file_into_terminal(req_servers,fds);
        }
}

void perform_ls(int network_socket,int command_type, char source_path[MAX_SIZE], char destination_path[MAX_SIZE])
{
    
    char cmd_info[3][1000];
    char cmd_tp[100],buffer[MAX_SIZE+1];
    ssize_t numRead=0,numWrite=0;

    printf("The path for which ls has to be executed is %s\n",source_path);

    memset(cmd_info, 0, (3000) * sizeof(char));

    sprintf(cmd_tp, "%d",command_type); 
    strcpy(cmd_info[0],cmd_tp);
    strcpy(cmd_info[1],source_path);
    strcpy(cmd_info[2],destination_path);
    numWrite = send(network_socket, cmd_info, 3000,0);
    if(numWrite < 0)
    {
        printf("Error Sending data to the Name Server\n");
        exit(0);
    }
    memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));  
    printf("Output of ls command is:\n"); 
    while((numRead = recv(network_socket,buffer,MAX_SIZE,0)) > 0)
    {
        printf("%s",buffer);
        memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));   
    }

}

void perform_move(int network_socket,int command_type, char source_path[MAX_SIZE], char dest_path[MAX_SIZE])
{
    int req_servers=0,i=0;
    char cmd_info[3][1000];
    char cmd_tp[100],buffer[MAX_SIZE+1],meta_data[MAX_SIZE+1];
    ssize_t numRead=0,numWrite=0;

    printf("The path for which mv has to be executed is %s\n",source_path);

    memset(cmd_info, 0, (3000) * sizeof(char));

    sprintf(cmd_tp, "%d",command_type); 
    strcpy(cmd_info[0],cmd_tp);
    strcpy(cmd_info[1],source_path);
    strcpy(cmd_info[2],dest_path);
    numWrite = send(network_socket, cmd_info, 3000,0);
    if(numWrite < 0)
    {
        printf("Error Sending data to the Name Server\n");
        exit(0);
    }
}

void perform_remove(int network_socket,int command_type, char source_path[MAX_SIZE], int port_numbers[NUM_OF_SERVERS])
{

    int chid=0,child_status=0,i=0,req_servers=0;
    char cmd_info[3][1000];
    char cmd_tp[100],buffer[MAX_SIZE+1],meta_data[MAX_SIZE+1];
    ssize_t numRead=0,numWrite=0;

    printf("The path for which ls has to be executed is %s\n",source_path);

    memset(cmd_info, 0, (3000) * sizeof(char));

    sprintf(cmd_tp, "%d",command_type); 
    strcpy(cmd_info[0],cmd_tp);
    strcpy(cmd_info[1],source_path);
    cmd_info[2][0]=0;
    
    numWrite = send(network_socket, cmd_info, 3000,0);
    if(numWrite < 0)
    {
        printf("Error Sending data to the Name Server\n");
        exit(0);
    }

    numRead = recv(network_socket, meta_data , MAX_SIZE,0);
    if(numRead <= 0)
    {
        printf("There was an error fetching meta data from the name server\n");
        exit(0);
    }
    meta_data[numRead]='\0';
    printf("The metadata received from the name server is %s\n",meta_data);

    int no_of_args=0;
    char *serv_fl_nams[100];
    char *a_ptr = strtok(meta_data, " ");
    while (a_ptr != NULL)
    {
        serv_fl_nams[no_of_args] = a_ptr;
        a_ptr = strtok(NULL, " ");
        no_of_args++;
    }
    serv_fl_nams[no_of_args]=NULL;
    req_servers = no_of_args;
    for(i=0;i<no_of_args;i++)
    {
        printf("Data server %d has required data in file %sXXX\n",i+1,serv_fl_nams[i]);
    }

    for(i=0;i<req_servers;i++)
    {
            chid = fork();
            if(chid == -1)
            {
                printf("Error which creating a child\n");
                exit(0);
            }
            else if(chid == 0)
            {
                int port_num=0,server_socket=0;
                char buffer[MAX_SIZE+1], data_ser_file_name[MAX_SIZE];
                char *ip = "127.0.0.1";

                port_num = port_numbers[i];
                //printf("port_num = %d\n",port_num);

                //Create Connection with Data Servers
                server_socket = create_client(ip,port_num);
                if(server_socket == -1)
                {
                    printf("of Data Server %d\n",i+1);
                    exit(0);
                }
                printf("Succesfully connected to Data Server %d\n",i+1);

                numWrite = send(server_socket,"3",1,0);         //Send information of command type[Read or write to data server] to data server
                if(numWrite <= 0)
                {
                    printf("Error writing to data server %d\n",i+1);
                    close(server_socket);
                    exit(0);
                }
                //printf("numWrite = %ld\n",numWrite);
                printf("File name to be deleted from Data Server %d is %sXXX\n",i+1,serv_fl_nams[i]);
                numWrite = send(server_socket,serv_fl_nams[i],strlen(serv_fl_nams[i]),0);
                if(numWrite <= 0)
                {
                    printf("Error writing to data server %d\n",i+1);
                    close(server_socket);
                    exit(0);
                }
                
                //Later Implement confirmation

                //printf("numWrite = %ld\n",numWrite);
                //memset(buffer, 0, (MAX_SIZE+1) * sizeof(char)); 

                /*numRead = recv(server_socket,buffer,MAX_SIZE,0);
                if(numRead ==0)
                {
                    printf("Unable to receive data from Data server %d\n",i+1);
                    exit(0);
                }
                else
                {
                    buffer[numRead]='\0';
                    if(buffer[0] == '0')
                    memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));   
                }*/
                printf("All Data needed is received from Data Server %d.Now,Closing the connection with Data Server %d\n",i+1,i+1);
                close(server_socket);
                exit(0);
            }
    }

    while((child_status =wait(NULL)) != -1);
    printf("All Data Server connections are closed\n");


}

void manage_folders(int network_socket,int command_type,char client_name[MAX_SIZE],char source_path[MAX_SIZE],char destination_path[MAX_SIZE],int port_numbers[NUM_OF_SERVERS],int msqid)
{
    int flag=0,i=0,j=0,k=0;
    struct dirent *de;
    char *file_list[100],*folder_list[100];
    char *fl;
    printf("The given folder is %s\n",source_path);
    /*if(command_type != COPYLTB || command_type != COPYBTL)
    {
        if(chdir("/home/sachin/Documents/Servers/Name Server") != 0)
        {
            printf("Error while changing the directory to Name Server root\n");
            exit(0);
        }
    }*/

    char cwrkd[MAX_SIZE];
    printf("Server's current working directory : %s\n", getcwd(cwrkd, 1000));
    if(chdir(source_path) != 0)
    {
        printf("Error while changing the directory to source path\n");
        exit(0);
    }
    printf("Server's current working directory : %s\n", getcwd(cwrkd, 1000));
    DIR *dr = opendir(cwrkd);
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        exit(0); 
    } 
    else
    {
        printf("Server's current working directory : %s\n", getcwd(cwrkd, 100));
    }
    while ((de = readdir(dr)) != NULL) 
    {
        fl = de->d_name;
        i=0;
        flag=0;
        if(strlen(fl) == 1)
        {
            if(fl[0] !='.')
            {
                folder_list[j++] = fl;
            }
        }
        else if(strlen(fl) == 2)
        {
            if(fl[0] != '.' && fl[1] != '.')
            {
                folder_list[j++] = fl;
            }
        }
        else
        {
            for(i=0;i<strlen(fl);i++)
            {
                if(fl[i] == '.')
                {
                    flag = 1;
                }
            }
            if(flag == 1)
            {
                file_list[k++] = fl;
            }
            else
            {
                folder_list[j++] = fl;
            }
        }
        //printf("%s\n", de->d_name); 
    }

    printf("The files are\n");
    for(i=0;i<k;i++)
    {
        printf("%s\n",file_list[i]);
    }

    printf("\n\nThe folders are\n");
    for(i=0;i<j;i++)
    {
        printf("%s\n",folder_list[i]);
    }

    char new_src_path[MAX_SIZE];
    char new_destination_path[MAX_SIZE];
    memset(new_src_path, 0, (MAX_SIZE) * sizeof(char));
    memset(new_destination_path, 0, (MAX_SIZE) * sizeof(char));
    printf(" k = %d\n",k);
    for(i=0;i<k;i++)
    {
        printf("i = %d\n",i);
        strcpy(new_src_path,source_path);
        strcat(new_src_path,"/");
        strcat(new_src_path,file_list[i]);
        strcpy(new_destination_path,destination_path);
        strcat(new_destination_path,"/");
        strcat(new_destination_path,file_list[i]);
        printf("src = %sXX\n",new_src_path);
        printf("destination_path = %s\n",new_destination_path);
        
        if(command_type == COPYLTB)
        {
            copy_to_bigfs(network_socket,command_type,client_name,new_src_path,new_destination_path,port_numbers,msqid);
        }
        else if(command_type == COPYBTL)
        {
            copy_to_localmachine(network_socket,command_type,client_name,new_src_path,new_destination_path,port_numbers,1);
        }
        else if(command_type == MOVE)
        {
            perform_move(network_socket,command_type,new_src_path,new_destination_path);
        }
        else if(command_type == REMOVE)
        {
            perform_remove(network_socket,command_type,new_src_path,port_numbers);
        }
        //copy_to_bigfs(network_socket,command_type,client_name,new_src_path,new_destination_path,port_numbers,msqid);
        memset(new_src_path, 0, (MAX_SIZE) * sizeof(char));
        memset(new_destination_path, 0, (MAX_SIZE) * sizeof(char));
    }
  
    closedir(dr);

}

void copy_btob(int network_socket , int command_type, char client_name[MAX_SIZE] , char source_path[MAX_SIZE], char destination_path[MAX_SIZE], int port_numbers[NUM_OF_SERVERS] , int msqid, int x)
{
    int i=0,chid =0,child_status=0;
        int fds[NUM_OF_SERVERS];
        char cmd_info[3][1000];
        char cmd_tp[100];
        char meta_data[MAX_SIZE];
        int req_servers = 0;
        ssize_t numRead=0,numWrite=0;
        memset(cmd_info, 0, (3000) * sizeof(char));
        memset(meta_data, 0, (MAX_SIZE) * sizeof(char));

        printf("The bigfs path from which the file has to be copied is %s\n",source_path);
        if(x == 1)
        {
            printf("The client path to which te file has to be copied is %s\n",destination_path);
        }

        command_type = 2;
        //Create the message to be sent to the Name Server
        if(x == 2)
        {
            command_type = CAT;
        }
        sprintf(cmd_tp, "%d",command_type); 
        strcpy(cmd_info[0],cmd_tp);
        strcpy(cmd_info[1],source_path);
        cmd_info[2][0] = 0;

        numWrite = send(network_socket, cmd_info, 3000,0);
        if(numWrite < 0)
        {
            printf("Error Sending data to the Name Server\n");
        }

        numRead = recv(network_socket,meta_data,MAX_SIZE,0);
        if(numRead <= 0)
        {
            printf("Error reading meta data from name server\n");
            exit(0);
        }
        meta_data[numRead]='\0';
        printf("The meta data received from the name server is %s\n",meta_data);

        create_temporary_files(fds,0);

        int no_of_args=0;
        char *serv_fl_nams[100];
        char *a_ptr = strtok(meta_data, " ");
        while (a_ptr != NULL)
        {
            serv_fl_nams[no_of_args] = a_ptr;
            a_ptr = strtok(NULL, " ");
            no_of_args++;
        }
        serv_fl_nams[no_of_args]=NULL;
        req_servers = no_of_args;
        for(i=0;i<no_of_args;i++)
        {
            printf("Data server %d has required data in file %sXXX\n",i+1,serv_fl_nams[i]);
        }

        for(i=0;i<req_servers;i++)
        {
            chid = fork();
            if(chid == -1)
            {
                printf("Error which creating a child\n");
                exit(0);
            }
            else if(chid == 0)
            {
                int port_num=0,server_socket=0;
                char buffer[MAX_SIZE+1], data_ser_file_name[MAX_SIZE];
                char *ip = "127.0.0.1";

                port_num = port_numbers[i];
                //printf("port_num = %d\n",port_num);

                //Create Connection with Data Servers
                server_socket = create_client(ip,port_num);
                if(server_socket == -1)
                {
                    printf("of Data Server %d\n",i+1);
                    exit(0);
                }
                printf("Succesfully connected to Data Server %d\n",i+1);

                numWrite = send(server_socket,"2",1,0);         //Send information of command type[Read or write to data server] to data server
                if(numWrite <= 0)
                {
                    printf("Error writing to data server %d\n",i+1);
                    close(server_socket);
                    exit(0);
                }
                //printf("numWrite = %ld\n",numWrite);
                printf("File name needed from Data Server %d is %sXXX\n",i+1,serv_fl_nams[i]);
                numWrite = send(server_socket,serv_fl_nams[i],strlen(serv_fl_nams[i]),0);
                if(numWrite <= 0)
                {
                    printf("Error writing to data server %d\n",i+1);
                    close(server_socket);
                    exit(0);
                }
                //printf("numWrite = %ld\n",numWrite);

                while((numRead = recv(server_socket,buffer,MAX_SIZE,0)) > 0)
                {
                    buffer[numRead]='\0';
                    //printf("NumRead = %ld\n",numRead);
                    //printf("FDS[%d] = %d\n",i,fds[i] );
                    numWrite =  write(fds[i],buffer,numRead);
                    if(numWrite <= 0)
                    {
                        printf("Error writing into a temporary file %d\n",i+1);
                        close(server_socket);
                        exit(0);
                    }
                    //printf("Num Write to file = %ld\n",numWrite);
                    //printf("%s\n",recvd_data);
                    memset(buffer, 0, (MAX_SIZE+1) * sizeof(char));   
                }
                printf("All Data needed is received from Data Server %d.Now,Closing the connection with Data Server %d\n",i+1,i+1);
                close(server_socket);
                exit(0);
            }
        }

        while((child_status =wait(NULL)) != -1);
        printf("All Data Server connections are closed\n");

        if(x == 1)
        {
            managebtob(network_socket,command_type,req_servers,fds,destination_path,port_numbers,msqid);       //Copy the files fetched from data server into local machine as one single file
            delete_tempFiles();
        } 
}

int main(int argc, char* argv[])
{
    int network_socket,command_type=0,flags=0,i=0,num_files=0,is_folder=0;
    int server_set[NUM_OF_SERVERS] , port_numbers[NUM_OF_SERVERS] ,fds[NUM_OF_SERVERS];
    size_t numRead=0,numWrite=0;
    char client_name[MAX_SIZE] = "client1";
    char file_name[MAX_SIZE],source_path[MAX_SIZE],destination_path[MAX_SIZE],directory_path[MAX_SIZE],cwrkd[MAX_SIZE];
    char *name_server_ip = "127.0.0.1";
    char cat_files[10][500];
    char cmd_info[3][1000];
    port_numbers[0]=9001;
    port_numbers[1]=9002;
    port_numbers[2]=9003;


    if(chdir("/home/sachin") != 0)
    {
        printf("Error while changing the directory\n");
        exit(0);
    }
    printf("Local Machines current working directory : %s\n", getcwd(cwrkd, 100));

    int msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    flags=0;
    printf("This is %s\n",client_name);

    //Connect with the Name server
    network_socket = create_client(name_server_ip , NAME_SERVER_PORT);
    if(network_socket == -1)
    {
        printf("of Name Server\n");
        exit(0);
    }
    printf("Connected to Name Server\n");


    //Clear Data in these character arrays
    memset(source_path, 0, (MAX_SIZE) * sizeof(char));
    memset(destination_path, 0, (MAX_SIZE) * sizeof(char));
    memset(directory_path, 0, (MAX_SIZE) * sizeof(char));
    memset(file_name, 0, (MAX_SIZE) * sizeof(char));


    //Take command as input
    command_type =input_command(client_name,source_path, destination_path,cat_files,&is_folder);
    printf("command type = %d\n",command_type);
    printf("Is folder = %d\n",is_folder);
    if(chdir("/home/sachin") != 0)
    {
        printf("Error while changing the directory\n");
        exit(0);
    }
    printf("Local Machines current working directory : %s\n", getcwd(cwrkd, 100));


    if(command_type == LS)
    {
        perform_ls(network_socket,command_type,source_path,destination_path);
    }
    else if(command_type == CAT)
    {
        num_files=0;
        printf("The files to be CAT are:\n");
        while(cat_files[num_files][0] != 0)
        {
            printf("%s\t\t %ldXX\n",cat_files[num_files],strlen(cat_files[num_files]));
            num_files++;
        }
        for(i=0;i<num_files;i++)
        {
            copy_to_localmachine(network_socket,command_type,client_name,cat_files[i],destination_path,port_numbers,2);
        }

    }
    else if(is_folder == 1)
    {
        if(command_type == MOVE)
        {
            strcpy(cmd_info[0],"11");
            strcpy(cmd_info[1],source_path);
            strcpy(cmd_info[2],destination_path);
            numWrite = send(network_socket, cmd_info, 3000,0);
            if(numWrite < 0)
            {
                printf("Error Sending data to the Name Server\n");
                exit(0);
            }
        }
        else
        {
            manage_folders(network_socket,command_type,client_name,source_path,destination_path,port_numbers,msqid);
        }

    } 
    else if(command_type == COPYLTB)
    {
        copy_to_bigfs(network_socket,command_type,client_name,source_path,destination_path,port_numbers,msqid);
    }
    else if(command_type == COPYBTL)
    {
        copy_to_localmachine(network_socket,command_type,client_name,source_path,destination_path,port_numbers,1);
    }
    else if(command_type == BTB)
    {
        copy_btob(network_socket,command_type,client_name,source_path,destination_path,port_numbers,msqid,1);
    }
    else if(command_type == MOVE)
    {
        perform_move(network_socket,command_type,source_path,destination_path);
    }
    else if(command_type == REMOVE)
    {

        perform_remove(network_socket,command_type,source_path,port_numbers);
    }

    printf("Closing Name Server\n");
    close(network_socket);
}
