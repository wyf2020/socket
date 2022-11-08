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

void set_type(char* s,int type,int is_complete);
void set_port(char* s,int port);
char* get_char(char* s);

int main(int argc, char *argv[]) {
  int sockfd, sendbytes, recvbytes;
  char buf[MAXDATASIZE];
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
  printf("%s\n",argv[1]);
  printf("hostent h_name: %s , h_aliases: %s,\
			h_addrtype: %d, h_length: %d, h_addr_list: %s\n",
         host->h_name, *(host->h_aliases), host->h_addrtype, host->h_length,
         *(host->h_addr_list));

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket:");
    exit(1);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SEVPORT);
  serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
  bzero(&(serv_addr.sin_zero), 8);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) ==
      -1) {
    perror("connect:");
    exit(1);
  }
  printf("connect server success.\n");
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
  
  while(1)
  {
    int i;
    memset(buf, 0x00, sizeof(buf));
    int quit=0;
    int option;
    printf("please log in opration: ");
    scanf("%d",&option);
    set_type(buf,option,1);
    char* s=get_char(buf);
    switch (option)
    { 
      case 1:
        
      break;
  case 2:
        gettimeofday(&timestamp, NULL);
        if ((sendbytes = send(sockfd, buf, sizeof(buf), 0)) == -1) {
          perror("send:");
          exit(1);
        }
        gettimeofday(&timestamp_end, NULL);
        printf("sendbytes: %d, cost time: %ld ms\n", sendbytes,
        TIME_DIFF(timestamp_end, timestamp));
        close(sockfd);
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
        if ((recvbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
          perror("recv");
          close(sockfd);
          exit(1);
        }
        printf("Client receive bytes: %d, msg: %s\n", recvbytes, buf);
        time_t host_time;
        time_t* p=(time_t*)s;
        host_time=*p;
        printf("host time: %u\n",host_time);
      break;
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
        if ((recvbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
          perror("recv");
          close(sockfd);
          exit(1);
        }
        printf("Client receive bytes: %d, msg: %s\n", recvbytes, buf);
        printf("hostname :  %s\n",s);
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
        if ((recvbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
          perror("recv");
          close(sockfd);
          exit(1);
        }
        printf("Client receive bytes: %d, msg: %s\n", recvbytes, buf);
        int * t5_p=(int*)(buf+9);
        int total_num=*t5_p;
        struct sockaddr_in * client_p=(struct sockaddr_in*)(buf+13);
        printf("\n ***********************\n");

        for(i=0;i<total_num;i++)
        {
          char IPdotdec[20];
          inet_ntop(AF_INET, &((client_p+i)->sin_addr), IPdotdec, 16);
          IPdotdec[17]='\0';
          printf("NO: %d  port: %u  ip: %s\n",i,(client_p+i)->sin_port,IPdotdec);
        }
    break;
  case 6:
        int no_send;
        printf("please log in the client number: ");
        scanf("%d",&no_send);
        getchar();
        printf("please log in your message: \n");
        int* p_no_send=(int*)(buf+1);
        gets(buf+9);
        gettimeofday(&timestamp, NULL);
        if ((sendbytes = send(sockfd, buf, sizeof(buf), 0)) == -1) {
          perror("send:");
          exit(1);
        }
        gettimeofday(&timestamp_end, NULL);
        printf("sendbytes: %d, cost time: %ld ms\n", sendbytes,
        TIME_DIFF(timestamp_end, timestamp));
        memset(buf, 0x00, sizeof(buf));
        if ((recvbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
          perror("recv");
          close(sockfd);
          exit(1);
        }
        printf("Client receive bytes: %d, msg: %s\n", recvbytes, buf);
        p_no_send=(int*)buf+9;
        no_send=*p_no_send;
        if(no_send==1)
          printf("receive success! \n");
        else
          printf("sent fault!\n");
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