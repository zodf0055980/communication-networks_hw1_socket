#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h> 
#include <time.h>
#define buffersize 100

int main(int argc, char **argv)
{
    char *protocal = argv[1];
    char *pg_do = argv[2];
    struct hostent *ip = gethostbyname(argv[3]);
    char *port = argv[4];
    char *fname;
    if(argc == 6)
        fname = argv[5];
    char path[buffersize];    
    char buffer[buffersize] = {};

    char message[] = "end";
    char get[] = "get";
    time_t t1;

    int sockfd,cliendfd;
    strcmp(protocal,"tcp") ? (sockfd = socket(AF_INET,SOCK_DGRAM,0)) : (sockfd = socket(AF_INET,SOCK_STREAM,0));
    
    if (strcmp(pg_do,"recv") == 0) {
        struct sockaddr_in serverInfo,clientInfo;
        bzero((char * )&serverInfo,sizeof(serverInfo));

        serverInfo.sin_family = PF_INET;
        serverInfo.sin_addr.s_addr = INADDR_ANY;
        serverInfo.sin_port = htons(atoi(port));
        socklen_t addrlen;
        bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));

        if (strcmp(protocal,"tcp") == 0) {
            listen(sockfd,5);
            cliendfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);

            recv(cliendfd,path,sizeof(path),0);
            send(cliendfd,get,sizeof(get),0);
            int gb;

            FILE *fp=fopen(path,"wb");

            while(1){      
                memset(buffer,0,buffersize);
                ssize_t by = recv(cliendfd,buffer,sizeof(buffer),0);
                if(strcmp(buffer,"end")==0){
                    break;
                }
                fwrite(buffer,sizeof(char),by,fp);
                send(cliendfd,get,sizeof(get),0);

            }
            fclose(fp);
        } else {
            struct sockaddr_in  aad;
            socklen_t aadlen;
            aadlen = sizeof(aad);
            recvfrom(sockfd, path, sizeof(path),0,(struct sockaddr *)&aad, &aadlen);
            FILE *fp=fopen(path,"wb");

            float getp = 0;

            while(1){
                aadlen = sizeof(aad);
                memset(buffer,0,buffersize);
                ssize_t by = recvfrom(sockfd, buffer, sizeof(buffer),0,(struct sockaddr *)&aad,&aadlen);
                getp++;
                if(strcmp(buffer,"end")==0){
                    getp--;
                    memset(buffer,'\0',buffersize);
                    printf("end\n");
                    recvfrom(sockfd, buffer, sizeof(buffer),0,(struct sockaddr *)&aad,&aadlen);
                    float allp = atof(buffer);
                    float rate = (allp-getp)/allp;
                    printf("get = %f , all = %f , UDP loss rate = %f\n",getp,allp,rate);
                    break;
                }
                fwrite(buffer,sizeof(char),by,fp);
            }
            fclose(fp);
        }   
    } else {
		struct sockaddr_in info;
		bzero(&info,sizeof(info));
		info.sin_family=AF_INET;
        bcopy((char *)ip->h_addr, (char *)&info.sin_addr.s_addr, ip->h_length);
		info.sin_port=htons(atoi(port));

        FILE *fp=fopen(fname,"rb");
        fseek(fp,0,SEEK_END);
        long passsize=ftell(fp);
        fseek(fp,0,SEEK_SET);
        long count = (passsize/buffersize) + 1;
        long sendbyte = passsize;
        passsize %= buffersize;

        long five = (count/20) + 1;
        long show = five;
        long showc = 0;
        long ccount=0;

        char *now;
        char ssend[buffersize];
        if (strcmp(protocal,"tcp") == 0) {
            connect(sockfd,(struct sockaddr *)&info,sizeof(info));
            send(sockfd,fname,sizeof(fname),0);
            recv(sockfd,buffer,sizeof(buffer),0);
            
            int b;
            for(b=count;b>0;b--){
                ccount++;
                memset(ssend,0,buffersize);
                if(b == 1) {
                    fread(ssend,sizeof(char),passsize,fp);
                    send(sockfd,ssend,passsize,0);
                    recv(sockfd,buffer,sizeof(buffer),0);
                    send(sockfd,message,sizeof(message),0);
                } else {
                    fread(ssend,sizeof(char),sizeof(ssend),fp);
                    send(sockfd,ssend,sizeof(ssend),0);
                    recv(sockfd,buffer,sizeof(buffer),0);
                }

                if (ccount == show){
                    t1=time(NULL);
                    now = ctime(&t1);
                    showc+=5;
                    show+=five;
                    printf("send %ld percentage , time:%s",showc,now);
                }
            }
 
        } else {
            sendto(sockfd, fname, sizeof(fname),0,(struct sockaddr *)&info, sizeof(info));
            int b;
            for(b=count;b>0;b--){
                ccount++;
                memset(ssend,'\0',buffersize);
                if(b == 1) {
                    fread(ssend,sizeof(char),passsize,fp);
                    sendto(sockfd, ssend, passsize,0,(struct sockaddr *)&info, sizeof(info));
                    sendto(sockfd, message, sizeof(message),0,(struct sockaddr *)&info, sizeof(info));
                    char sn[buffersize];
                    memset(sn,'\0',buffersize);
                    sprintf(sn,"%ld",count);
                    sendto(sockfd, sn, sizeof(sn),0,(struct sockaddr *)&info, sizeof(info));
                } else {
                    fread(ssend,sizeof(char),sizeof(ssend),fp);
                    sendto(sockfd, ssend, sizeof(ssend),0,(struct sockaddr *)&info, sizeof(info));
                }

                if (ccount == show){
                    t1=time(NULL);
                    now = ctime(&t1);
                    showc+=5;
                    show+=five;
                    printf("send %ld percentage , time:%s",showc,now);
                }

             }
        }
    printf("end\n");
    }

    close(sockfd);
	return 0;
}
