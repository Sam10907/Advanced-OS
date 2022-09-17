#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;
int main(int argc, char *argv[]){
    //輸入連線資訊
    printf("Please input your name: ");
    char name[15]="";
    scanf("%s",name);
    printf("Please input your group: ");
    char group[20] = "";
    scanf("%s",group);
    char c;
    scanf("%c",&c);
    //設定socket資訊
    int fd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in srv;
    srv.sin_family=AF_INET;
    srv.sin_port=htons(1234);
    srv.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(connect(fd,(struct sockaddr*) &srv,sizeof(srv)) < 0){ //連線失敗
        perror("connect error\n");
        exit(1);
    }else{
        if(send(fd,name,strlen(name),0) < 0){ //傳送使用者名稱
            perror("send error");
            exit(1);
        }
        usleep(3000);
        if(send(fd,group,strlen(group),0) < 0){ //傳送群組名稱
            perror("send error1");
            exit(1);
        }
        printf("Please input command: ");
        char *command = NULL;
        size_t bufsize = 100;
        size_t counts;
        while((counts = getline(&command,&bufsize,stdin)) != EOF){
            send(fd,command,strlen(command),0); //傳送指令
            char opera[15] = ""; //操作
            char file[15] = ""; //檔案
            char *key = strtok(command," \t\n");
            strcpy(opera,key);
            if(!strcmp(opera,"exit")){
                break;
            }
            key = strtok(NULL," \t\n");
            strcpy(file,key);
            if(!strcmp(opera,"create")){ //create
                memset(command,0,bufsize);
                printf("Please input command: ");
                continue;
            }
            else if(!strcmp(opera,"read")){ //read
                FILE *f = fopen(file,"w");
                char buf[1024] = "";
                int nbytes;
                while((nbytes = recv(fd,buf,sizeof(buf),0)) >= 0 ){
                    if(!strcmp(buf,"eof")) break; //檔案下載結束
                    if(!strcmp(buf,"You are not allowed to read.")){
                        printf("%s\n",buf);
                        break;
                    }
                    fwrite(buf,sizeof(char),strlen(buf),f);
                    memset(buf,0,sizeof(buf));
                }
                fclose(f);
            }
            else if(!strcmp(opera,"write")){ //write
                char message[50] = "";
                recv(fd,message,sizeof(message),0);
                if(!strcmp(message,"yes")){ //准許修改檔案
                    printf("Start modify:\n");
                    FILE *f = fopen(file,"w");
                    if(f == NULL){
                        perror("fopen error");
                        exit(1);
                    }
                    char *content = NULL;
                    size_t bufsize = 100;
                    size_t counts;
                    while((counts = getline(&content,&bufsize,stdin)) != EOF){
                        if(!strcmp(content,"exit\n")) break; //輸入exit時退出修改
                        //send(fd,content,strlen(content),0);
                        fwrite(content,sizeof(char),strlen(content),f);
                    }
                    fclose(f);
                    f = fopen(file,"r");
                    while(!feof(f)){
                        char buf[1024] = "";
                        int fread_nbytes=fread(buf,sizeof(char),sizeof(buf),f);
                        if((send(fd,buf,fread_nbytes,0)) < 0){ //傳送失敗
                            perror("send error\n");
                            exit(1);
                        }
                    }
                    fclose(f);
                    char msg[5] = "eof";
                    usleep(3000);
                    send(fd,msg,strlen(msg),0); //傳送eof告知server檔案修改結束
                }else{ //不允許修改檔案
                    printf("%s\n",message);
                }
            }
            else if(!strcmp(opera,"changemode")){ //changemode
                memset(command,0,bufsize);
                printf("Please input command: ");
                continue;
            }
            memset(command,0,bufsize);
            printf("Please input command: ");
        }
    }
}