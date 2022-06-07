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

#define NUM_THREAD 3 //今回は3クライアントの通信

void die(char *);
void *communication(void *arg);

//sendするデータのフォーマット
/* typedef struct {
    int num;
    unsigned char dat;
} send_data; */

typedef struct {
    int port;
    int * recved_flag; //0 || 1
    int * sendable_flag; //0 || 1
    unsigned char * recv_data_ptr;
    unsigned char * send_data_ptr;
} arg;



int main(int argc, char ** argv)
{
    //メモリの確保
    unsigned char * send_data_a = (unsigned char *)malloc(sizeof(unsigned char)); //クライアントAにsendする用
    unsigned char * send_data_b = (unsigned char *)malloc(sizeof(unsigned char)); //クライアントBにsendする用
    unsigned char * send_data_c = (unsigned char *)malloc(sizeof(unsigned char)); //クライアントCにsendする用
    unsigned char * recv_data_a = (unsigned char *)malloc(sizeof(unsigned char)); //クライアントAからrecvした用
    unsigned char * recv_data_b = (unsigned char *)malloc(sizeof(unsigned char)); //クライアントBからrecvした用
    unsigned char * recv_data_c = (unsigned char *)malloc(sizeof(unsigned char)); //クライアントCからrecvした用


    //recvを見るフラグの定義
    int recved_flag_a = 0;
    int recved_flag_b = 0;
    int recved_flag_c = 0;

    //send可能かのフラグの定義
    int sendable_flag_a = 0;
    int sendable_flag_b = 0;
    int sendable_flag_c = 0;

    //スレッド、スレッドに渡す引数の定義
    pthread_t t[NUM_THREAD];
    arg d[NUM_THREAD];

    //スレッドAの開始
    d[0].port = 50000;
    d[0].recved_flag = &recved_flag_a;
    d[0].sendable_flag = &sendable_flag_a;
    d[0].send_data_ptr = send_data_a;
    d[0].recv_data_ptr = recv_data_a;
    pthread_create(&t[0], NULL, communication, &d[0]);

    //スレッドBの開始
    d[1].port = 49990;
    d[1].recved_flag = &recved_flag_b;
    d[1].sendable_flag = &sendable_flag_b;
    d[1].send_data_ptr = send_data_b;
    d[1].recv_data_ptr = recv_data_b;
    pthread_create(&t[1], NULL, communication, &d[1]);

    //スレッドCの開始
    d[2].port = 49980;
    d[2].recved_flag = &recved_flag_c;
    d[2].sendable_flag = &sendable_flag_c;
    d[2].send_data_ptr = send_data_c;
    d[2].recv_data_ptr = recv_data_c;
    pthread_create(&t[2], NULL, communication, &d[2]);

    //スレッドの巡回とrecvしたデータをsendするデータに入れる
    while(1)
    {
        if(recved_flag_a == 1){
            send_data_a = recv_data_a;
            recved_flag_a = 0;
        } else {
            send_data_a = 0;
        }

        if(recved_flag_b == 1){
            send_data_b = recv_data_b;
            recved_flag_b = 0;
        } else {
            send_data_b = 0;
        }

        if(recved_flag_c == 1){
            send_data_c = recv_data_c;
            recved_flag_c = 0;
        } else {
            send_data_c = 0;
        }
    }


    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);
    pthread_join(t[2], NULL);
}

void *communication(void *thr_arg){
    //ポートを開いてサーバを起動
    arg * pd = (arg *)thr_arg;
    int ss = socket(PF_INET, SOCK_STREAM, 0);
    if(ss < 0) die("socket");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(pd->port); //引数で渡されたポート番号をソケットに指定
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(ss, (struct sockaddr *)&addr,sizeof(addr));
    listen(ss,10);
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int s = accept(ss,(struct sockaddr *) &client_addr, &len);


    int N = 1;
    unsigned char data1[N];

    while(1){
        int l = recv(s, data1, N, 0);
        pd->recv_data_ptr = data1;
        if (l == -1) die("recv");
        if (l > 0) {
            pd->recved_flag = 1;
        }
    }

    shutdown(s,SHUT_WR);
    close(s);
}

void die(char *s){
    perror(s);
    exit(1);
}