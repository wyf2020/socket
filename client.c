#include <error.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define SEVPORT 2891
#define MAXDATASIZE (1024 * 5)


#define TIME_DIFF(t1, t2)                                                      \
  (((t1).tv_sec - (t2).tv_sec) * 1000 + ((t1).tv_usec - (t2).tv_usec) / 1000)

struct packet
{
  char type;
  int port;
  int sequence_num;
  char data[4800];
};

struct arg_struct {
    int connect_fd;
};

struct node {
  int no;
  struct sockaddr_in sockaddr;
};

void set_type(char* s,int type,int is_complete);
void set_port(char* s,int port);
char* get_char(char* s);
void print_menu(void);

char message[MAXDATASIZE] = {};
int valid_bit = 0;
int recvbytes_global;
pthread_mutex_t valid_bit_mutex;
pthread_t thread_id;


void receive_thread(void *arg_struct) {
  int connect_fd = ((struct arg_struct*)arg_struct)->connect_fd;
  while(1) {
    char buf[MAXDATASIZE] = {};
    // printf("recv_loop\n");
    pthread_mutex_lock(&valid_bit_mutex);
    if(valid_bit == -1) {
      valid_bit = 0;
      pthread_mutex_unlock(&valid_bit_mutex);
      pthread_exit(0);
    }
    pthread_mutex_unlock(&valid_bit_mutex);

    int recvbytes;
    if ((recvbytes = recv(connect_fd, buf, MAXDATASIZE, 0)) == -1) {
      perror("recv");
      close(connect_fd);
      exit(1);
    }
    int opt = *buf - '0';
    if(opt == 8) { // 指示类型
      int sent_no = *((int*)(buf+1));
      printf("INFO from No %d:%s", sent_no, buf+5);
    }else if(opt > 0 && opt <= 7) { // 响应类型
      pthread_mutex_lock(&valid_bit_mutex);
      valid_bit = 1;
      recvbytes_global = recvbytes;
      memcpy(message, buf, sizeof(buf));
      pthread_mutex_unlock(&valid_bit_mutex);
    }
   }
}

int main(int argc, char *argv[]) {
  int sockfd, sendbytes, recvbytes;
  char buf[MAXDATASIZE] = {};
  struct hostent *host;
  struct sockaddr_in serv_addr;
  struct timeval timestamp;
  struct timeval timestamp_end;
  if (argc < 2) {
    fprintf(stderr, "Please enter the server's hostname!\n");
    exit(1);
  }

  if ((host = gethostbyname(argv[1])) == NULL) {
    perror("gethostbyname:");
    exit(1);
  }
  // printf("%s\n",argv[1]);
  // printf("hostent h_name: %s , h_aliases: %s,\
	// 		h_addrtype: %d, h_length: %d, h_addr_list: %s\n",
  //        host->h_name, *(host->h_aliases), host->h_addrtype, host->h_length,
  //        *(host->h_addr_list));
  print_menu();
  while(1) {
    int i;
    memset(buf, 0x00, sizeof(buf));
    int quit=0;
    int option;
    printf("please log in opration: ");
    scanf("%d",&option);
    set_type(buf,option,1);
    char* s=get_char(buf);
    switch (option) { 
      case 1:
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
          perror("socket:");
          exit(1);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SEVPORT);
        // serv_addr.sin_addr.s_addr = inet_addr("192.168.43.94");
        serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
        bzero(&(serv_addr.sin_zero), 8);
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) ==-1) {
          perror("connect:");
          exit(1);
        }
        printf("connect server successfully.\n");
        struct arg_struct args;
        args.connect_fd = sockfd;
        pthread_create(&thread_id, NULL, (void*)&receive_thread, (void*)&args);
        break;
      case 2:
        pthread_mutex_lock(&valid_bit_mutex);
        valid_bit = -1;
        pthread_mutex_unlock(&valid_bit_mutex);
        gettimeofday(&timestamp, NULL);
        if ((sendbytes = send(sockfd, buf, sizeof(buf), 0)) == -1) {
          perror("send:");
          exit(1);
        }
        gettimeofday(&timestamp_end, NULL);
        printf("sendbytes: %d, cost time: %ld ms\n", sendbytes,
        TIME_DIFF(timestamp_end, timestamp));
        close(sockfd);
        printf("close connection successfully.\n");
        break;
      case 3:
        gettimeofday(&timestamp, NULL);
        if ((sendbytes = send(sockfd, buf, sizeof(buf), 0)) == -1) {
          perror("send:");
          exit(1);
        }
        gettimeofday(&timestamp_end, NULL);
        printf("sendbytes: %d, cost time: %ld ms\n", sendbytes,
        TIME_DIFF(timestamp_end, timestamp));
        memset(buf, 0x00, sizeof(buf));
        while(valid_bit!=1);
        pthread_mutex_lock(&valid_bit_mutex);
        valid_bit = 0;
        printf("Client receive bytes: %d, msg: %s\n", recvbytes, message);
        char time_str[255] = {};
        strftime(time_str, sizeof(time_str), "%F %H:%M:%S", localtime((time_t*)(message+9)));
        printf("host time: %s\n",time_str);
        pthread_mutex_unlock(&valid_bit_mutex);
        break;
      case 4:
        gettimeofday(&timestamp, NULL);
        if ((sendbytes = send(sockfd, buf, sizeof(buf), 0)) == -1) {
          perror("send:");
          exit(1);
        }
        gettimeofday(&timestamp_end, NULL);
        printf("sendbytes: %d, cost time: %ld ms\n", sendbytes,
        TIME_DIFF(timestamp_end, timestamp));
        memset(buf, 0x00, sizeof(buf));
        while(valid_bit!=1);
        pthread_mutex_lock(&valid_bit_mutex);
        valid_bit = 0;
        printf("Client receive bytes: %d, msg: %s\n", recvbytes, message);
        printf("hostname :  %s\n",(message+9));
        pthread_mutex_unlock(&valid_bit_mutex);
        break;
      case 5:
        gettimeofday(&timestamp, NULL);
        if ((sendbytes = send(sockfd, buf, sizeof(buf), 0)) == -1) {
          perror("send:");
          exit(1);
        }
        gettimeofday(&timestamp_end, NULL);
        printf("sendbytes: %d, cost time: %ld ms\n", sendbytes,
        TIME_DIFF(timestamp_end, timestamp));
        memset(buf, 0x00, sizeof(buf));
        while(valid_bit!=1);
        pthread_mutex_lock(&valid_bit_mutex);
        valid_bit = 0;
        printf("Client receive bytes: %d, msg: %s\n", recvbytes, message);
        int * t5_p=(int*)(message+9);
        int total_num=*t5_p;
        struct node * client_p=(struct node*)(message+13);
        printf("\n************************\n");
        for(i=0;i<total_num;i++)
        {
          char IPdotdec[20];
          inet_ntop(AF_INET, &((client_p+i)->sockaddr.sin_addr), IPdotdec, 16);
          IPdotdec[17]='\0';
          printf("NO: %d  port: %u  ip: %s\n",(client_p+i)->no,(client_p+i)->sockaddr.sin_port,IPdotdec);
        }
        printf("\n************************\n");
        pthread_mutex_unlock(&valid_bit_mutex);
        break;
      case 6:
        int no_send;
        printf("please log in the client number: ");
        scanf("%d",&no_send);
        getchar();
        printf("please log in your message: \n");
        *(int*)(buf+1) = no_send;
        fgets(buf+9, MAXDATASIZE-9, stdin);
        gettimeofday(&timestamp, NULL);
        if ((sendbytes = send(sockfd, buf, sizeof(buf), 0)) == -1) {
          perror("send:");
          exit(1);
        }
        gettimeofday(&timestamp_end, NULL);
        printf("sendbytes: %d, cost time: %ld ms\n", sendbytes,
        TIME_DIFF(timestamp_end, timestamp));
        memset(buf, 0x00, sizeof(buf));
        while(valid_bit!=1);
        pthread_mutex_lock(&valid_bit_mutex);
        valid_bit = 0;
        printf("Client receive bytes: %d, msg: %s\n", recvbytes, message);
        if(*((int*)(message+9)) == 1) {
          printf("send msg to no.%d success! \n", no_send);
        }else {
          printf("send msg to no.%d failed! \n", no_send);
        }
        pthread_mutex_unlock(&valid_bit_mutex);
        break;
      case 7:
        gettimeofday(&timestamp, NULL);
        if ((sendbytes = send(sockfd, buf, sizeof(buf), 0)) == -1) {
          perror("send:");
          exit(1);
        }
        gettimeofday(&timestamp_end, NULL);
        printf("sendbytes: %d, cost time: %ld ms\n", sendbytes,
        TIME_DIFF(timestamp_end, timestamp));
        quit=1;
        break;
      default:
        printf("wrong option !\n");
        break;
    }
    if(quit) break;
    }
    close(sockfd);
}

void set_type(char* s,int type,int is_complete)
{
  if(type<-7||type>7)
  {
    printf("error option");
    exit(1);
  }
    if(is_complete==0)
      type=-type;
    *s='0'+type;
}

void set_port(char* s,int port)
{
  int * p=(int*)(s+1);
  *p=port;
}

char* get_char(char* s)
{
  return s+9;
}

void print_menu(void) {
  printf("  +-------------------------------------+\n");
	printf("  |    login 1~7 to select functions    |\n");
  printf("  +-------------------------------------+\n");
  printf("  | 1. connect                          |\n");
  printf("  | 2. close                            |\n");
  printf("  | 3. getServerTime                    |\n");
  printf("  | 4. getServerName                    |\n");
  printf("  | 5. activeList                       |\n");
  printf("  | 6. send                             |\n");
  printf("  | 7. exit                             |\n");
  printf("  +-------------------------------------+\n");
}