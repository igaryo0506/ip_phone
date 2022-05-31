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


void die(char *);

int main(int argc, char ** argv){
    if(argc == 2){
        // server
        int ss = socket(PF_INET, SOCK_STREAM, 0);
        if(ss < 0){die("socket");}

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(atoi(argv[1]));
        addr.sin_addr.s_addr = INADDR_ANY;
        bind(ss, (struct sockaddr *)&addr,sizeof(addr));

        listen(ss,10);

        struct sockaddr_in client_addr;
        socklen_t len = sizeof(struct sockaddr_in);
        int s = accept(ss,(struct sockaddr *) &client_addr, &len);

        char * cmdline = "rec -t raw -b 16 -c 1 -e s -r 44100 -";
        FILE * fp = popen(cmdline, "r");

        int N = 256;
        unsigned char data1[N];
        unsigned char data2[N];
        while(1){
            int m = fread(data1, 1, N, fp);
            if (m == -1) { die("read");}
            if (m > 0){
                int n = send(s, data1, m, 0);
                if (n == -1) { die("send");}
            }

            int l = recv(s, data2, N, 0);
            if (l == -1) { die("recv"); }
            if (l > 0){
                write(1, data2, l);
            }
        }
        shutdown(s,SHUT_WR);
        close(s);
    }else if(argc == 3){
        // client
        int s = socket(PF_INET, SOCK_STREAM, 0);

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(argv[1]);
        addr.sin_port = htons(atoi(argv[2]));
        int ret = connect(s,(struct sockaddr*)&addr, sizeof(addr));
        if(ret != 0){
            die("connect");
        }

        char * cmdline = "rec -t raw -b 16 -c 1 -e s -r 44100 -";
        FILE * fp = popen(cmdline, "r");

        // FILE * fp = fopen("phone.c","r");

        int N = 1;
        unsigned char data1[N];
        unsigned char data2[N];
        
        while(1){
            int m = fread(data1, 1, N,fp);
            if(m == -1){
                die("read");
            }
            if(m > 0){
                int l = send(s,data1,m,0);
                if (l < 0){ die("send");}
            }
            ssize_t n = recv(s,data2,N,0);
            write(1,data2,n);
        }
        shutdown(s,SHUT_WR);
    }
}

void die(char *s){
    perror(s);
    exit(1);
}