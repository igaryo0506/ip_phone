#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>

#define NUM_THREAD 2

void die(char *);
void *communication(void *arg);

struct data {
    int port;
};

int main(int argc, char ** argv)
{
    pthread_t t[NUM_THREAD];
    struct data d[NUM_THREAD];
    d[0].port = 50000;

    pthread_create(&t[0], NULL, communication, &d[0]);
    pthread_join(t[0], NULL);
    // communication(argv[1]);
}

void *communication(void *arg){
    struct data *pd = (struct data *)arg;
    int ss = socket(PF_INET, SOCK_STREAM, 0);
    if(ss < 0) die("socket");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(pd->port);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(ss, (struct sockaddr *)&addr,sizeof(addr));

    listen(ss,10);

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int s = accept(ss,(struct sockaddr *) &client_addr, &len);

    char * cmdline = "rec -t raw -b 16 -c 1 -e s -r 44100 -";
    FILE * fp = popen(cmdline, "r");

    int N = 1;
    unsigned char data1[N];
    unsigned char data2[N];
    while(1){
        int m = fread(data1, 1, N, fp);
        if (m == -1) die("read");
        if (m > 0){
            int n = send(s, data1, m, 0);
            if (n == -1) { die("send");}
        }

        int l = recv(s, data2, N, 0);
        if (l == -1) die("recv");
        if (l > 0) write(1, data2, l);
    }
    shutdown(s,SHUT_WR);
   close(s);
}

void die(char *s){
    perror(s);
    exit(1);
}