/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define NTHREADS 12


// mutex lock
pthread_mutex_t lock_cubby;


typedef struct _thread_data_t {
    char cubbyhole[256];
    int sockfd;
    struct sockaddr_in cli_addr;
    int clilen;
} thread_data_t;


void error(char *msg)
{
    perror(msg);
    exit(1);
}

int fill_return_buffer(int max_size, char buffer[], char string_to_add[], int string_length){
   max_size -= string_length;
   if (max_size >= 0){
        strcat(buffer,string_to_add);
   }
   else{
        bzero(buffer,512);
        strcat(buffer,"to many commands");
   }
   return max_size;    
}


void *accept_connection(void *arg){

    thread_data_t *data = (thread_data_t *)arg;
    int newsockfd, quit, char_counter, char_counter_2, command_counter, length_of_buffer, max_return_buffer_size, length_of_return_buffer, n;
    char buffer[256];
    char command[8];
    char c;
    char look_answer[263];
    char return_buffer[512];

    while (1){
    newsockfd = accept(data->sockfd, (struct sockaddr *) &(data->cli_addr), &(data->clilen));
    if (newsockfd < 0) 
        error("ERROR on accept");

    printf("%d",newsockfd);
    n = write(newsockfd,"!HELLO: Welcome to the Cubbyhole Server! Try 'help' for a list of commands",74);
    quit = 0;
        while (quit == 0){
            char_counter = 0;
            bzero(buffer,256);
            bzero(return_buffer,512);
            max_return_buffer_size = 512;
         

            n = read(newsockfd,buffer,255);
            length_of_buffer = strlen(buffer);
            if (n < 0){error("ERROR reading from socket");}
            else if (n == 0) {break;}
            
            while (char_counter < length_of_buffer){ 
                
                bzero(command,8);
                command_counter = 0;
                while(((buffer[char_counter])!='\0') && ((buffer[char_counter])!='\n') && ((buffer[char_counter])!=' ')){
                   c = buffer[char_counter];
                   if (c>90){
                       c-=32;
                   }
                   command[command_counter] = c;
                   char_counter++;
                   command_counter++;
                }
                

                if (strcmp(command, "QUIT")==0){
                   quit = -1;
                   max_return_buffer_size = fill_return_buffer(max_return_buffer_size, return_buffer, "!QUIT: ok", 9);
                   if (max_return_buffer_size < 0){
                        break;
                   }
                }
                else if (strcmp(command, "PUT")==0){
                   char_counter++;
                   char_counter_2 = 0;
                   // lock mutex
                   pthread_mutex_lock(&lock_cubby);
                   bzero(data->cubbyhole,256);
                   while(((buffer[char_counter])!='\0') && ((buffer[char_counter])!='\n')){
                       c = buffer[char_counter];
                       data->cubbyhole[char_counter_2] = c;
                       char_counter++;
                       char_counter_2++;
                   }
                   pthread_mutex_unlock(&lock_cubby);
                   max_return_buffer_size = fill_return_buffer(max_return_buffer_size, return_buffer, "!PUT: ok", 8);
                   if (max_return_buffer_size < 0){
                        break;
                   }
                }
                else if (strcmp(command, "LOOK")==0){
                   strcpy(look_answer,"!LOOK: ");
                   // lock mutex
                   pthread_mutex_lock(&lock_cubby);
                   if (data->cubbyhole[0] != '\0'){
                       strcat(look_answer, data->cubbyhole);
                   }
                   else {
                       strcat(look_answer, "no message stored");
                   }
                   pthread_mutex_unlock(&lock_cubby);
                   int answer_length = strlen(look_answer);
                   max_return_buffer_size = fill_return_buffer(max_return_buffer_size, return_buffer, look_answer, answer_length);
                   if (max_return_buffer_size < 0){
                        break;
                   }
                }
                else if (strcmp(command, "GET")==0){
                   strcpy(look_answer,"!GET: ");
                   // lock mutex
                   pthread_mutex_lock(&lock_cubby);
                   if (data->cubbyhole[0] != '\0'){
                       strcat(look_answer, data->cubbyhole);
                   }
                   else {
                       strcat(look_answer, "no message stored");
                   }
                   pthread_mutex_unlock(&lock_cubby);
                   int answer_length = strlen(look_answer);
                   max_return_buffer_size = fill_return_buffer(max_return_buffer_size, return_buffer, look_answer, answer_length);
                   if (max_return_buffer_size < 0){
                        break;
                   }
                   bzero(data->cubbyhole,256);
                }
                else if (strcmp(command, "DROP")==0){
                   // lock mutex
                   pthread_mutex_lock(&lock_cubby);
                   if (data->cubbyhole[0] != '\0'){
                       max_return_buffer_size = fill_return_buffer(max_return_buffer_size, return_buffer, "!DROP: ok", 9);
                       if (max_return_buffer_size < 0){
                            break;
                       }
                       bzero(data->cubbyhole,256);
                   }
                   else{
                       max_return_buffer_size = fill_return_buffer(max_return_buffer_size, return_buffer, "!DROP: no message stored", 24);
                       if (max_return_buffer_size < 0){
                            break;
                       }
                   }
                   pthread_mutex_unlock(&lock_cubby);
                }
                else if (strcmp(command, "HELP")==0){
                   char help_comment[] = "PUT < message > Legt eine neue Nachricht < message > im Versteck ab.\nGET Holt die Nachricht aus dem Versteck hervor und zeigt sie an.\nLOOK Zeigt die Nachricht im Versteck an. Sie bleibt aber im Unterschied zu GET dabei im Versteck liegen.\nDROP Verwirft die Nachricht aus dem Versteck ohne sie anzuzeigen.\nHELP Erklaert, wie der Server zu verwenden ist.\nQUIT Beendet die Verbindung zum Server.";
                   max_return_buffer_size = fill_return_buffer(max_return_buffer_size, return_buffer, help_comment, 392);
                   if (max_return_buffer_size < 0){
                        break;
                   }
                }
                else{
                    bzero(return_buffer,512);
                    max_return_buffer_size = fill_return_buffer(max_return_buffer_size, return_buffer, "no valid command. consult HELP", 30);
                    break;
                }
                char_counter++;
                strcat(return_buffer," ");
            }
            length_of_return_buffer = strlen(return_buffer);
            n = write(newsockfd,return_buffer,length_of_return_buffer);
            if (n < 0) error("ERROR writing to socket");
        }
        close(newsockfd);


    }

    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    int sockfd, portno, clilen, thread_counter;
    char cubbyhole[256];
    thread_data_t thr_data;
    pthread_t pth[NTHREADS];

    
    bzero(cubbyhole,256);
    thread_counter = 0;

    struct sockaddr_in serv_addr, cli_addr;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
        sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    /* initialize pthread mutex protecting "shared_x" */
    pthread_mutex_init(&lock_cubby, NULL);
    
    strcpy(thr_data.cubbyhole,cubbyhole);
    thr_data.sockfd = sockfd;
    thr_data.cli_addr = cli_addr;
    thr_data.clilen = clilen;

    for (thread_counter=0; thread_counter < NTHREADS; thread_counter++){
        if (pthread_create(&pth[thread_counter], NULL, accept_connection, &thr_data)){
            fprintf(stderr,"thread not correctly sporned\n");
            exit(1);
        }
    }
    for (thread_counter = 0; thread_counter < NTHREADS; thread_counter++) {
        pthread_join(pth[thread_counter], NULL);
    }

    
    return 0; 
}
