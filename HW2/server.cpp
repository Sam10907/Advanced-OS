#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
using namespace std;
struct capability_obj{
    char name[15];
    char file[15];
    char permission[8];
};
void set_capability_obj(struct capability_obj*,char*,char*,char*);
int download_file(int,char,char*);
int upload_file(int,char,char*,char*);
void* serve_client(void *data);
vector<struct capability_obj> owner;
vector<struct capability_obj> aos;
vector<struct capability_obj> cse;
vector<struct capability_obj> other;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
int main(int argc, char *argv[]){
    //建立socket資訊
    int fd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in srv,cli;
    socklen_t clilen=sizeof(cli);
    srv.sin_family=AF_INET;
    srv.sin_port=htons(1234);
    srv.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(fd,(struct sockaddr*) &srv,sizeof(srv)) < 0){
        perror("bind error\n");
        exit(1);
    }else{ //開始監聽
        if(listen(fd,5) < 0){
            perror("listen error\n");
            exit(1);
        }else{
            vector<pthread_t> t;
            int index = 0;
            while(1){
                int newfd;
                newfd=accept(fd,(struct sockaddr*) &cli,&clilen); //連線成功時會回傳新的socket file descriptor
                if(newfd < 0){ //連線失敗
                    perror("accept error\n");
                    exit(1);
                }else{
                    pthread_t th;
                    t.push_back(th);
                    pthread_create(&t[index],NULL,serve_client,&newfd);
                    
                    //shutdown(newfd,SHUT_RDWR);
                    //shutdown(fd,SHUT_RDWR);
                }
                index++;
            }
            for(int i = 0 ; i < t.size() ; i++){
                pthread_join(t[i],NULL);
            }
        }
    }
}
void set_capability_obj(struct capability_obj* obj,char* name,char* file,char* permission){
    strcpy(obj -> name,name);
    strcpy(obj -> file,file);
    strcpy(obj -> permission,permission);
}
int download_file(int fd,char r,char* filename){
    if(r == 'r'){
        int nbytes;
        pthread_mutex_lock(&mutex1);
        FILE *f = fopen(filename,"r");
        if(f == NULL){
            perror("fopen error");
            exit(1);
        }
        while(!feof(f)){ //開始傳送檔案
            char buf[1024] = "";
            int fread_nbytes=fread(buf,sizeof(char),sizeof(buf),f);
            if((nbytes=send(fd,buf,fread_nbytes,0)) < 0){ //傳送失敗
                    perror("send error\n");
                    exit(1);
            }
        }
        char msg[5] = "eof"; //傳送結束時傳eof告知client
        usleep(3000);
        send(fd,msg,strlen(msg),0);
        fclose(f);
        pthread_mutex_unlock(&mutex1);
    }
    else if(r == '-'){
        char msg[50] = "You are not allowed to read.";
        send(fd,msg,strlen(msg),0);
    }
    return 0;
}
int upload_file(int fd,char w,char *per,char *file){ //client連線至server端修改檔案
    if(w == 'w'){
        pthread_mutex_lock(&mutex1);
        char msg[5] = "yes"; //告知client准許寫檔
        send(fd,msg,strlen(msg),0);
        if(per[0] == 'o'){ //覆寫
            int f = open(file,O_RDWR | O_TRUNC,0666);
            if(f < 0){
                perror("open error");
                exit(1);
            }
            int nbytes;
            char buf[1024] = "";
            while ((nbytes = recv(fd,buf,sizeof(buf),0)) >= 0){
                if(!strcmp(buf,"eof")) break;
                write(f,buf,strlen(buf));
                memset(buf,0,sizeof(buf));
            }
            close(f);
        }
        else if(per[0] == 'a'){ //插入
            int f = open(file,O_RDWR | O_APPEND,0666);
            if(f < 0){
                perror("open error");
                exit(1);
            }
            int nbytes;
            char buf[1024] = "";
            while ((nbytes = recv(fd,buf,sizeof(buf),0)) >= 0){
                if(!strcmp(buf,"eof")) break;
                write(f,buf,strlen(buf));
                memset(buf,0,sizeof(buf));
            }
            close(f);
        }
        pthread_mutex_unlock(&mutex1);
    }
    else if(w == '-'){
        char msg[50] = "You are not allowed to write.";
        send(fd,msg,strlen(msg),0);
    }
    return 0;
}
void* serve_client(void *data){
    int newfd1 = *(int*) data;
    printf("connect success\n");
    char name[15] = ""; //名字
    char group[20] = ""; //群組
    char command[100] = ""; 
    recv(newfd1,name,sizeof(name),0);
    recv(newfd1,group,sizeof(group),0);
    while(1){
        recv(newfd1,command,sizeof(command),0);
        if(!strcmp(command,"exit\n")) break;
        char opera[15] = ""; //操作
        char file[15] = ""; //檔案
        char permission[8] = ""; //權限設定
        char *key = strtok(command," \t\n");
        strcpy(opera,key);
        key = strtok(NULL," \t\n");
        strcpy(file,key);
        if(strcmp(opera,"read")){ //read不需第二個參數
            key = strtok(NULL," \t\n");
            strcpy(permission,key);
        }
        if(!strcmp(opera,"create")){ //create
            pthread_mutex_lock(&mutex);
            struct capability_obj obj;
            set_capability_obj(&obj,name,file,permission); //設定capability obj
            owner.push_back(obj); //插入owner capability list
            if(!strcmp(group,"AOS-students")){
                struct capability_obj obj;
                set_capability_obj(&obj,name,file,&permission[2]);
                aos.push_back(obj); //插入AOS capability list
            }
            else if(!strcmp(group,"CSE-students")){
                struct capability_obj obj;
                set_capability_obj(&obj,name,file,&permission[2]);
                cse.push_back(obj); //插入CSE capability list
            }
            struct capability_obj obj1;
            set_capability_obj(&obj1,name,file,&permission[4]);
            other.push_back(obj1); //插入other capability list
            int fd = open(file,O_CREAT | O_RDWR | O_TRUNC | O_APPEND,0666);
            if(fd < 0){
                perror("open error");
                exit(1);
            }
            close(fd);
            pthread_mutex_unlock(&mutex);
        }
        else if(!strcmp(opera,"read")){ //read
            int own = 1;
            for(int i = 0 ; i < owner.size() ; i++){ //check owner capability list
                if(!strcmp(owner[i].name,name) && !strcmp(owner[i].file,file)){
                    download_file(newfd1,owner[i].permission[0],owner[i].file); //下載檔案
                    own = 0;
                    break;
                }
            }
            if(own){
                if(!strcmp(group,"AOS-students")){
                    int b = 1;
                    for(int i = 0 ; i < aos.size() ; i++){ //check AOS group capability list
                        if(!strcmp(aos[i].file,file)){ 
                            b = download_file(newfd1,aos[i].permission[0],aos[i].file);
                            break;
                        }
                    }
                    if(b){
                        for(int i = 0 ; i < other.size() ; i++){ //check other group capability list
                            if(!strcmp(other[i].file,file)){
                                b = download_file(newfd1,other[i].permission[0],other[i].file);
                                break;
                            }
                        }
                    }
                    if(b){ //如果AOS group capability list size為0
                        char msg[50] = "You are not allowed to read.";
                        send(newfd1,msg,strlen(msg),0);
                    }
                }
                else if(!strcmp(group,"CSE-students")){
                    int b = 1;
                    for(int i = 0 ; i < cse.size() ; i++){ //check CSE group capability list
                        if(!strcmp(cse[i].file,file)){
                            b = download_file(newfd1,cse[i].permission[0],cse[i].file);
                            break;
                        }
                    }
                    if(b){
                        for(int i = 0 ; i < other.size() ; i++){ //check other group capability list
                            if(!strcmp(other[i].file,file)){
                                b = download_file(newfd1,other[i].permission[0],other[i].file);
                                break;
                            }
                        }
                    }
                    if(b){ //如果CSE group capability list size為0
                        char msg[50] = "You are not allowed to read.";
                        send(newfd1,msg,strlen(msg),0);
                    }
                }
                else{
                    int b = 1;
                    for(int i = 0 ; i < other.size() ; i++){ //check other group capability list
                        if(!strcmp(other[i].file,file)){
                            b = download_file(newfd1,other[i].permission[0],other[i].file);
                            break;
                        }
                    }
                    if(b){
                        char msg[50] = "You are not allowed to read.";
                        send(newfd1,msg,strlen(msg),0);
                    }
                }
            }
        }
        else if(!strcmp(opera,"write")){ //write
            int own = 1;
            for(int i = 0 ; i < owner.size() ; i++){ //check owner capability list
                if(!strcmp(owner[i].name,name) && !strcmp(owner[i].file,file)){
                    upload_file(newfd1,owner[i].permission[1],permission,owner[i].file);
                    own = 0;
                    break;
                }
            }
            if(own){
                if(!strcmp(group,"AOS-students")){
                    int b = 1;
                    for(int i = 0 ; i < aos.size() ; i++){ //check AOS group capability list
                        if(!strcmp(aos[i].file,file)){
                            b = upload_file(newfd1,aos[i].permission[1],permission,aos[i].file);
                            break;
                        }
                    }
                    if(b){
                        for(int i = 0 ; i < other.size() ; i++){ //check other group capability list
                            if(!strcmp(other[i].file,file)){
                                b = upload_file(newfd1,other[i].permission[1],permission,other[i].file);
                                break;
                            }
                        }
                    }
                    if(b){ //如果AOS group capability list size為0
                        char msg[50] = "You are not allowed to write.";
                        send(newfd1,msg,strlen(msg),0);
                    }
                }
                else if(!strcmp(group,"CSE-students")){
                    int b = 1;
                    for(int i = 0 ; i < cse.size() ; i++){ //check CSE group capability list
                        if(!strcmp(cse[i].file,file)){
                            b = upload_file(newfd1,cse[i].permission[1],permission,cse[i].file);
                            break;
                        }
                    }
                    if(b){
                        for(int i = 0 ; i < other.size() ; i++){ //check other group capability list
                            if(!strcmp(other[i].file,file)){
                                b = upload_file(newfd1,other[i].permission[1],permission,other[i].file);
                                break;
                            }
                        }
                    }
                    if(b){ //如果CSE group capability list size為0
                        char msg[50] = "You are not allowed to write.";
                        send(newfd1,msg,strlen(msg),0);
                    }
                }
                else{
                    int b = 1;
                    for(int i = 0 ; i < other.size() ; i++){ //check other group capability list
                        if(!strcmp(other[i].file,file)){
                            b = upload_file(newfd1,other[i].permission[1],permission,other[i].file);
                            break;
                        }
                    }
                    if(b){
                        char msg[50] = "You are not allowed to read.";
                        send(newfd1,msg,strlen(msg),0);
                    }
                }
            }
        }
        else if(!strcmp(opera,"changemode")){ //changemode
            pthread_mutex_lock(&mutex2);
            for(int i = 0 ; i < owner.size() ; i++){
                if(!strcmp(owner[i].name,name)){
                    if(!strcmp(owner[i].file,file)){
                        strcpy(owner[i].permission,permission);
                    }
                    if(!strcmp(group,"AOS-students")){
                        for(int i = 0 ; i < aos.size() ; i++){
                            if(!strcmp(aos[i].file,file) && !strcmp(aos[i].name,name)){ //更改AOS group讀寫這個檔案的權限
                                strcpy(aos[i].permission,&permission[2]);
                            }
                        }
                    }
                    else if(!strcmp(group,"CSE-students")){
                        for(int i = 0 ; i < cse.size() ; i++){
                            if(!strcmp(cse[i].file,file) && !strcmp(cse[i].name,name)){ //更改CSE group讀寫這個檔案的權限
                                strcpy(cse[i].permission,&permission[2]);
                            }
                        }
                    }
                    for(int i = 0 ; i < other.size() ; i++){
                        if(!strcmp(other[i].file,file) && !strcmp(other[i].name,name)){ //更改other group讀寫這個檔案的權限
                            strcpy(other[i].permission,&permission[4]);
                        }
                    }
                }
            }
            pthread_mutex_unlock(&mutex2);
        }
        memset(command,0,100);
    }
    pthread_exit(NULL);
}