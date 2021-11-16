# Custom-BigFS-File-System

Custom-made file system “bigfs” is created as described below. The purpose of this file system is to store large files on multiple systems.

In this, a file system is created where the data is distributed among different data servers.
The client connects to the name server and gets the data servers details.
Depending on the Command, the client communicates with both name server and data server or only name server.

The operations performed in this are: 
1. Copy from Local Machine to Bigfs,
2. Copy from Bigfs to Local,
3. Copy from Bigfs to Bigfs
4. move
5. remove
6. cat
7. ls

1-> For copy from Local to Bigfs, the client gets the file, divides into blocks of 1 MB and copies the chunks into different data servers.
After writing the received data into a file, data servers send back the file name to the client. Client waits until all the data servers have sent back the files and the metadata is sent to the name server, where it is stored in the desired location.

2-> For copy from Bigfs to local machine, the metadata is fetched from the name server and the file names are extracted. The respective file names are sent to the data servers and those files are retreived to the client, where it is copied into.

3-> For copy from bigfs to bigfs, it merges the operations of copy from bigfs to local and copy from local to bigfs

4-> For the "move" command, the client communicates only with the name server. It just moves the metadata from source to destination in the data server.

5-> For the "remove" command, the client fetches the metadata file form the name server, obtains the files and deletes those files in the data server as well as metadata in the name server.

6-> For the "cat" command, the metadata is fetched from the name server and the obtained files are fetched from the respective data servers and then those data are presented in the standard output.

7-> For the "ls" command, the client communicates only with the Name server. It output all the files and folders present in a particular directory in the name server.
