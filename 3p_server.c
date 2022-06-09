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
#include <string.h>

#define NUM_THREAD 3 //今回は3クライアントの通信
#define LENGTH 1000 //送受信するデータの基本単位

void die(char *);
void *communication(void *arg);
void patrol_thread_two_client(
    int * recved_flag_1,
    int * recved_flag_2,
    int * sendable_flag_1,
    int * sendable_flag_2,
    unsigned char * send_data_1,
    unsigned char * send_data_2,
    unsigned char * recv_data_1,
    unsigned char * recv_data_2
);

//sendするデータのフォーマット
/* typedef struct {
    int num;
    unsigned char dat;
} send_data; */

typedef struct {
    int port;
    int * connect_flag; //0 || 1
    int * recved_flag; //0 || 1
    int * sendable_flag; //0 || 1
    unsigned char * recv_data_ptr;
    unsigned char * send_data_ptr;
} arg;



int main()
{
    write(1,"begin",5);
    //メモリの確保
    unsigned char * send_data_a = (unsigned char *)malloc(sizeof(unsigned char) * LENGTH); //クライアントAにsendする用
    unsigned char * send_data_b = (unsigned char *)malloc(sizeof(unsigned char) * LENGTH); //クライアントBにsendする用
    unsigned char * send_data_c = (unsigned char *)malloc(sizeof(unsigned char) * LENGTH); //クライアントCにsendする用
    unsigned char * recv_data_a = (unsigned char *)malloc(sizeof(unsigned char) * LENGTH); //クライアントAからrecvした用
    unsigned char * recv_data_b = (unsigned char *)malloc(sizeof(unsigned char) * LENGTH); //クライアントBからrecvした用
    unsigned char * recv_data_c = (unsigned char *)malloc(sizeof(unsigned char) * LENGTH); //クライアントCからrecvした用

    write(1,"malloc done",11);

    // sendするdataが空の時にセグフォならないようにする
    send_data_a[0] = '\0';
    send_data_b[0] = '\0';
    send_data_c[0] = '\0';

    //接続しているかどうかのフラグの定義
    int connect_flag_a = 0;
    int connect_flag_b = 0;
    int connect_flag_c = 0;


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
    d[0].connect_flag = &connect_flag_a;
    d[0].recved_flag = &recved_flag_a;
    d[0].sendable_flag = &sendable_flag_a;
    d[0].send_data_ptr = send_data_a;
    d[0].recv_data_ptr = recv_data_a;
    pthread_create(&t[0], NULL, communication, &d[0]);
    write(1,"thread A begin",14);

    //スレッドBの開始
    d[1].port = 49990;
    d[1].connect_flag = &connect_flag_b;
    d[1].recved_flag = &recved_flag_b;
    d[1].sendable_flag = &sendable_flag_b;
    d[1].send_data_ptr = send_data_b;
    d[1].recv_data_ptr = recv_data_b;
    pthread_create(&t[1], NULL, communication, &d[1]);
    write(1,"thread B begin",14);

    //スレッドCの開始
    d[2].port = 49980;
    d[2].connect_flag = &connect_flag_c;
    d[2].recved_flag = &recved_flag_c;
    d[2].sendable_flag = &sendable_flag_c;
    d[2].send_data_ptr = send_data_c;
    d[2].recv_data_ptr = recv_data_c;
    pthread_create(&t[2], NULL, communication, &d[2]);
    write(1,"thread C begin",14);

    //スレッドの巡回とrecvしたデータをsendするデータに入れる
    while(1)
    {
        if(connect_flag_a + connect_flag_b + connect_flag_c == 3){
            if(recved_flag_a == 1){
                *send_data_a = (*recv_data_b + *recv_data_c) / 2;
                recved_flag_a = 0;
                sendable_flag_a = 1;
                write(1,"send A",6);
            } else {
                *send_data_a = 0;
                sendable_flag_a = 1;
                write(1,"send else A",11);
            }

            if(recved_flag_b == 1){
                *send_data_b = (*recv_data_a + *recv_data_c) / 2;
                recved_flag_b = 0;
                sendable_flag_b = 1;
            } else {
                *send_data_b = 0;
                sendable_flag_b = 1;
            }

            if(recved_flag_c == 1){
                *send_data_c = (*recv_data_a + *recv_data_b) / 2;
                recved_flag_c = 0;
                sendable_flag_c = 1;
            } else {
                *send_data_c = 0;
                sendable_flag_c = 1;
            }
        }else if(connect_flag_a + connect_flag_b + connect_flag_c == 2){
            if(connect_flag_a & connect_flag_b){
                patrol_thread_two_client(
                    &recved_flag_a,
                    &recved_flag_b,
                    &sendable_flag_a,
                    &sendable_flag_b,
                    send_data_a,
                    send_data_b,
                    recv_data_a,
                    recv_data_b
                );
            } else if(connect_flag_a & connect_flag_c) {
                patrol_thread_two_client(                                        
                    &recved_flag_a,
                    &recved_flag_c,
                    &sendable_flag_a,
                    &sendable_flag_c,
                    send_data_a,
                    send_data_c,
                    recv_data_a,
                    recv_data_c
                );
            } else {                                
                patrol_thread_two_client(                                        
                    &recved_flag_b,
                    &recved_flag_c,
                    &sendable_flag_b,
                    &sendable_flag_c,
                    send_data_b,
                    send_data_c,
                    recv_data_b,
                    recv_data_c
                );
            }
            
        }else if (connect_flag_a + connect_flag_b + connect_flag_c == 1){
            memcpy(send_data_a, recv_data_a, LENGTH);
            write(1,"a\n",2);
            recved_flag_a = 0;
            sendable_flag_a = 1;
        }
        
    }

    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);
    pthread_join(t[2], NULL);
}

void patrol_thread_two_client(
    int * recved_flag_1,
    int * recved_flag_2,
    int * sendable_flag_1,
    int * sendable_flag_2,
    unsigned char * send_data_1,
    unsigned char * send_data_2,
    unsigned char * recv_data_1,
    unsigned char * recv_data_2
)
{
    if(*recved_flag_1 == 1){
        memcpy(send_data_1,recv_data_2,LENGTH);                    
        *recved_flag_1 = 0;
        *sendable_flag_1 = 1;
    } else {
        *send_data_1 = 0;
        *sendable_flag_1 = 1;
    }

    if(*recved_flag_2 == 1){
        memcpy(send_data_2,recv_data_1,LENGTH);
        *recved_flag_2 = 0;
        *sendable_flag_2 = 1;
    } else {
        *send_data_2 = 0;
        *sendable_flag_2 = 1;
    }
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
    *pd->connect_flag = 1;

    while(1){
        //データを受け取って、受け取れたらその値を、受け取れなかったら0をメモリに格納
        int l = recv(s, pd->recv_data_ptr, 1000, 0);
        // write(1,pd->recv_data_ptr,l);
        if (l == -1){
            die("recv");
        } else if (l == 0) {
            *pd->recv_data_ptr = 0;
            *pd->recved_flag = 1;
        } else if (l > 0) {
            *pd->recved_flag = 1;
        }

        // sendable_flagが1ならsendする
        if (*pd->sendable_flag == 1) {
            // write(1,pd->send_data_ptr, 1000);
            int n = send(s, pd->send_data_ptr, 1000, 0);
            if (n == -1) die("send");
            *pd->sendable_flag = 0;    
        }
    }
    shutdown(s,SHUT_WR);
    close(s);
    *pd->connect_flag = 0;
}

void die(char *s)
{
    perror(s);
    exit(1);
}