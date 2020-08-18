#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include "aes2.h"
enum MSG_METHOD{
	MSG_NULL=0,
	MSG_INSERT,
	MSG_SEARCH,

};

struct msg{
	int size;//不包括头部的4个字节 总大小 只有mid+data 向上取整
	int mid;
	char data[1024];
};
int poll_read(int fd, void*data, int data_len, int sec)
{
	int ret = read(fd,data,data_len);
	if(sec)
	{	
		char *buf=(char *)data;
	
		char *decrypt_buf = NULL;
		/*printf("传输到服务器的数据%s\n",buf);
		int i;
		for(i=0;i<res;i++){
			printf("%02x ",(unsigned char)buf[i]);
		}
		printf("\n");*/
		decrypt_buf = (char *)malloc(ret);

		int round=ret/16;
		int i;
		for(i=0;i<round;i++){
			int temp;
			temp=i*16;
			decrpyt_buf(buf+temp,decrypt_buf+temp,ret);
		}
	

		printf("解密的数据=%s\n",decrypt_buf);
		// 解密
	}

	return ret;
}


int poll_write(int fd, void*data, int data_len, int sec)
{
	
	if(sec)
	{	
		char *buf=(char *)data;
		/*struct msg* m = (struct msg*)data;
		printf("%d\n",ntohl(m->size));*/
		//   m->size 是正确的
		int padding_size=0;
		char *after_padding_buf = NULL;
		char *encrypt_buf = NULL;
		after_padding_buf = padding_buf(buf+4,data_len-4,&padding_size);
		printf("pading-size=%d\n",padding_size);
		int i;
		for(i=0;i<padding_size;i++){
			printf("%02x ",(unsigned char)after_padding_buf[i]);
		}
		printf("\n");
		//printf_buff(after_padding_buf,padding_size);
		encrypt_buf = (char *)malloc(padding_size);
		int round=padding_size/16;
		for(i=0;i<round;i++){
			int temp;
			temp=i*16;
			encrpyt_buf(after_padding_buf+temp,encrypt_buf+temp, padding_size);
		}
		//int i;
		//打印加密后的
		for(i=0;i<padding_size;i++){
			printf("%02x ",(unsigned char)encrypt_buf[i]);
		}
		printf("\n");
		write(fd,buf,4);
		write(fd,encrypt_buf,padding_size-6);
		return write(fd,encrypt_buf + padding_size-6,6);
		//return write(fd,encrypt_buf,padding_size);
		// 加密 
	}

	return write(fd,data,data_len);
}


int main(){

	int sfd,res;
	char buf[1024];
	char search_buf[1024];
	char res_buf[1024];
	int i,j;
	struct sockaddr_in servaddr; 
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd<0){
		perror("socket eror...");
		return 0;
	}

	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	servaddr.sin_port=htons(6666);
	res=connect(sfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	if(res<0){
		perror("conn error...");
		return 0;
	}
	struct msg message;
	while(1){
		fgets(buf,1024,stdin);
		if(!strncmp(buf,"insert",6)){
			printf("客户端想要插入\n");
			int temp_len=strlen(buf+6);
			printf("%d\n",temp_len);
			message.mid=htonl(MSG_INSERT);
			printf("htonl-mid=%d\n",message.mid);
			//sprintf(message->data,"%s",buf+6);
			memcpy(message.data,buf+6,temp_len);
			printf("data=%s\n",message.data);

			//这里size  应该上取整到最近的16的倍数
			int __num=(temp_len+sizeof(int))/16;
			
			message.size=htonl(16*(__num+1));
			printf("htonl.size%d\n",message.size);
			//write(sfd,buf,sizeof(int)+strlen(message->data));
			int i;
			for(i=0;i<2*sizeof(int)+temp_len;i++){
				printf("%02x ",(unsigned char)message.data[i]);
			}
			printf("\n");
			poll_write(sfd,&message,2*sizeof(int)+temp_len,1);
		}else if(!strncmp(buf,"search",6)){
			printf("客户端想要查询\n");
			i=0;j=0;
			while(i<strlen(buf)-1){
				search_buf[j++]=buf[i++];
			}
			search_buf[j]='\0';
			message.mid=htonl(MSG_SEARCH);
			sprintf(message.data,"%s",search_buf+6);
			int temp_len=strlen(search_buf+6);
			
			//这里size  应该上取整到最近的16的倍数
			int __num=(temp_len+sizeof(int))/16;
			
			printf("size==%d\n",16*(__num+1));
			message.size=htonl(16*(__num+1));
			//write(sfd,buf,sizeof(int)+strlen(message->data));
			poll_write(sfd,&message,2*sizeof(int)+temp_len,1);
			
			//res=read(sfd,res_buf,sizeof(res_buf));
			poll_read(sfd,res_buf,sizeof(res_buf),1);

			
		}else{
			printf("格式不符合,请重新输入");
		}
	}



	
}

