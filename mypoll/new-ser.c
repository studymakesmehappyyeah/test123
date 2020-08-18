#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <poll.h>
#include <signal.h>
#include "hash.h"
#include "list.h"
#include "aes2.h"
#include <openssl/aes.h>
#include<errno.h>

/*unsigned int SDBMHash(char *str)
{
    unsigned int hash = 0;

    while (*str)
    {
        // equivalent to: hash = 65599*hash + (*str++);
        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
    }

    return (hash & 0x7FFFFFFF);
}*/

enum MSG_METHOD{
	MSG_NULL=0,
	MSG_INSERT,
	MSG_SEARCH

};

struct msg{
	int size;//不包括头部的4个字节 总大小 只有mid+data 向上取整
	int mid;
	char data[0];
};

struct hlist_head hash_arr[16];
#define HASH_MASK 15;
struct hash_node{
	char key[1024];
	char val[1024];
	struct hlist_node hash;
};

struct hlist_head read_arr[16];
struct read_data{
	int fd;  //存储fd
	struct hlist_node node;//根据fd找到这个结构体
	char * str;//存储的数据
	int len; //现在存了多少
	int memlen;//能够存多少 也就是malloc了多少  不够就realloc 体现动态的
};

struct read_data* find_struct(int fd){
	struct hlist_node* post=NULL;
	int num;
	num=BobHash(fd) & HASH_MASK;
	hlist_for_each(post, &read_arr[num])
	{	
		struct read_data*temp=hlist_entry(post, struct read_data, node);
		//如果找到了
		if(temp->fd==fd){
			return temp;
		}
	}
	//到这里 说明没有找到

	printf("dont find fd \n");
	return NULL;

}

int data_enough(struct read_data*temp){
	struct msg* message = (struct msg*)temp->str;
	int size=ntohl(message->size);
	if(size+4>temp->len){
		//printf("mess->size=%d,temp->len=%d\n",size,temp->len);
		printf("dont enough\n");
		return 0;//没有读够
	}else if(size+4==temp->len){
		//printf("mess->size=%d,temp->len=%d\n",size,temp->len);
		printf("enough\n");
		return 1;
	}
}

void insert(struct msg* message){
	FILE * fp;
	printf("这是一条插入的数据\n");
	if(!strstr(message->data,":")){
		printf("这条数据格式不对 key:val\n");
		return;
	}
	int flag=0;

	//检查key是否一致  如果一致 要覆盖
	
	//保存key
	
	char *key_tr;
	char *val_tr;
	char *temp_str;//strtok的第一个参数会被切  所以申请一个临时的字符串
	printf("strlen=%d\n",strlen(message->data));
	temp_str=(char *)malloc(strlen(message->data));
	strcpy(temp_str,message->data);
	key_tr=strtok(temp_str,":");
	val_tr=strtok(NULL,":");
	
	struct hlist_node* pos=NULL;
	int num;
	num=SDBMHash(key_tr) & HASH_MASK;
	
	hlist_for_each(pos, &hash_arr[num])
	{	
		
		struct hash_node*temp=hlist_entry(pos, struct hash_node, hash);
		
		if(!strcmp(temp->key,key_tr)){
			strcpy(temp->val,val_tr);
			flag=1;
			FILE *fp1;
			FILE *fp2;
			char str[1024];
			fp1=fopen("test.txt","r");
			if(fp1==NULL){
				printf("打开文件失败..");
				exit(1);
			}
			fp2=fopen("newtest.txt","w");
			if(fp2==NULL){
				printf("打开文件失败..");
				exit(1);
			}
			while(fgets(str,1024,fp1)){
				if(strncmp(str,temp->key,strlen(temp->key))){ //如果一样 就不写入fp2
					fputs(str,fp2);
				}
			}
			fputs(message->data,fp2);
			fclose(fp1);
			fclose(fp2);								
			remove("test.txt");
			rename("newtest.txt","test.txt");
			break;
		}
	}

	if(!flag){
		if((fp=fopen("test.txt","a"))==NULL){
			perror("open fail..");
			return;
		}
		fputs(message->data,fp);
		fclose(fp);
		//同步加进去
		char *ptr;
		struct hash_node* node=(struct hash_node*)malloc(sizeof(struct hash_node));
		//memset(ptr,0,sizeof(ptr));
		ptr=strtok(message->data,":");
		if(ptr!=NULL){
			memcpy(&node->key,ptr,strlen(ptr));
		}
		ptr=strtok(NULL,":");
		if(ptr!=NULL){
			memcpy(&node->val,ptr,strlen(ptr));
		}
		num=SDBMHash(node->key) & HASH_MASK;
		hlist_add_head(&node->hash, &hash_arr[num]);		
	}	
}

void search(struct read_data*ss,struct msg* message){
	printf("这是一条查询的数据\n");
	int __flag=0;
	struct hlist_node* pos=NULL;
	int num;
	num=SDBMHash(message->data) & HASH_MASK;
	hlist_for_each(pos, &hash_arr[num]){
		struct hash_node*temp=hlist_entry(pos, struct hash_node, hash);
		if(!strcmp(temp->key,message->data)){
			//write(polls[i].fd,temp->val,sizeof(temp->val));
			int ret = poll_write(ss->fd,temp->val,sizeof(temp->val),1);
			__flag=1;
			break;
		}
	}
	if(!__flag){
		char no[1024]="sorry no";
		poll_write(ss->fd,no,strlen(no),1);
	}
}


int  operate_data(struct read_data*temp,void*data){	
	int flag;
	flag=data_enough(temp);
	if(flag==0){
		printf("没有收完,下一次\n");
		return 0;
	}
	char *buf=(char *)temp->str;
	int i;
	
	char *decrypt_buf = NULL;
	decrypt_buf = (char *)malloc(temp->len-4);
	//int res;
	int round=(temp->len-4)/16;
	for(i=0;i<round;i++){
		int cache;
		cache=i*16;
		decrpyt_buf(buf+4+cache,decrypt_buf+cache,temp->len-4);
	}

	char *new_data= (char *)malloc(temp->len);
	memcpy(new_data,data,4);
	memcpy(new_data+4,decrypt_buf,temp->len-4);//前四个字节是size的大小

	
	struct msg* message=(struct msg*)new_data;
	int method=ntohl(message->mid);
	switch(method){
		case MSG_INSERT:
			insert(message);
			break;
			
			
		case MSG_SEARCH:
			search(temp,message);
			break;
			
		default:
			printf("既不是查询也不是插入\n");
			break;
	}
	return temp->len;

	// 解密	

	
}

int poll_read(struct read_data*temp,void*data,int data_len,int sec){
	int ret=read(temp->fd,data,data_len);
	int flag;
	
	if(ret+temp->len>temp->memlen){//大于开辟的内存大小
		int size=0;//看要开辟多少才够
		while(size+temp->memlen<ret+temp->len){
			size+=1024;
		}
		temp->str=(char *)realloc(temp->str,temp->memlen+size);
		temp->memlen=temp->memlen+size;//修改能存的大小
		memcpy(temp->str+temp->len,data,ret);//新读的数据 拷贝
		temp->len=temp->len+ret;//现在存储的大小
		
	}else{
		//直接存
		memcpy(temp->str+temp->len,data,ret);//新读的数据 拷贝
		temp->len=ret+temp->len;
	}


	return ret;
}



int poll_write(int fd, void*data, int data_len, int sec)
{
	if(sec)
	{	
		char *buf=(char *)data;

		int padding_size=0;
		char *after_padding_buf = NULL;
		char *encrypt_buf = NULL;
		after_padding_buf = padding_buf(buf,strlen(buf),&padding_size);
		encrypt_buf = (char *)malloc(padding_size);
		int round=padding_size/16;
		int i;
		for(i=0;i<round;i++){
			int temp;
			temp=i*16;
			encrpyt_buf(after_padding_buf+temp,encrypt_buf+temp, padding_size);
		}

		return write(fd,encrypt_buf,padding_size);
		// 加密
		
	}

	return write(fd,data,data_len);
}

int socket_init()
{
	int sfd;
	int res;
	struct sockaddr_in servaddr;
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd<0){
		perror("socket error...");
		exit(1);
	}

	//设置端口复用
	int on=1;
	res=setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,(void *)&on,sizeof(int));
	//int setsockopt(int sockfd,int level,int optname,const void *optval,socklen_t optlen);
	if(res<0){
		perror("setsockopt error:");
		return -1;
	}
	//设置接收端的超时等待
	//struct timeval timeout = {10,0}; 
	//setsockopt(sfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval));
	memset(&servaddr,0,sizeof(servaddr)); 
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(6666);
	
	res=bind(sfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	if(res<0){
		perror("bind error...");
		exit(1);
	}

	res=listen(sfd,10);
	if(res<0){
		perror("listen error...");
		exit(1);
	}

	return sfd;
}

struct pollfd * poll_init(int sfd){
	struct pollfd *polls = (struct pollfd *)malloc(sizeof(struct pollfd) * 1024);
	int i;
	polls[0].fd=sfd;
	polls[0].events=POLLIN;
	for(i=1;i<1024;i++){
		polls[i].fd=-1;
	}
	return polls;
}


int hash_init(){
	//将文件的数据 读取到hash里面
	FILE * fp;
	char str[1024];
	char *ptr;
	if((fp=fopen("test.txt","r"))==NULL){
		return -1;
	}
	while(fgets(str,1024,fp)){
		struct hash_node* node=(struct hash_node*)malloc(sizeof(struct hash_node));
		
		//memset(ptr,0,sizeof(ptr));
		ptr=strtok(str,":");
		if(ptr!=NULL){
			memcpy(node->key,ptr,strlen(ptr));
		}
		ptr=strtok(NULL,":");
		if(ptr!=NULL){
			memcpy(node->val,ptr,strlen(ptr));
		}
		int num;
		num=SDBMHash(node->key) & HASH_MASK;
		hlist_add_head(&node->hash, &hash_arr[num]);

	}
	
}



void poll_run(struct pollfd *polls,int sfd){
	
	int maxi=0;
	int i,j;
	char buf[1024];
	struct sockaddr_in cliaddr;
	int cfd;
	socklen_t len=sizeof(cliaddr);
	int res,ret;
	while(1){
		res=poll(polls,maxi+1,-1);
		if(res<0){
			perror("poll error..");
			return;
		}
		//有新客户端连接
		if(polls[0].revents & POLLIN){
			cfd=accept(sfd,(struct sockaddr*)&cliaddr,&len);
			printf("连接成功...ip=%s\n",inet_ntoa(cliaddr.sin_addr));
			for(i=1;i<1024;i++){
				if(polls[i].fd==-1){
					polls[i].fd=cfd;
					polls[i].events=POLLIN;

					//创建结构体
					struct read_data* poin=(struct read_data*)malloc(sizeof(struct read_data));
					
					poin->fd=polls[i].fd;
					poin->str=(char *)malloc(1024);
					poin->memlen=1024;
					poin->len=0;
					//存入hash
					int num;
					num=BobHash(poin->fd) & HASH_MASK;
					hlist_add_head(&poin->node, &read_arr[num]);
					
					break;
				}
			}
			if(cfd>maxi)maxi=cfd;
			if(i>=1023){
				perror("polls full...");
				break;
			}
		}
		//客户端有数据的输入
		for(i=1;i<=maxi;i++){
			if(polls[i].revents & POLLIN){
				memset(buf, 0, sizeof(buf));
				//ret=read(polls[i].fd,buf,sizeof(buf));

				//这里先利用fd 找到那个结构体 
				//后面的参数 不应该再出现poll
				struct read_data*ss=find_struct(polls[i].fd);
				if(ss==NULL){
					continue;
				}

				// 读取套接字可读内容
				ret=poll_read(ss,buf,sizeof(buf),1);


				if(ret < 0) {}
				else if(ret==0){
					printf("polls.fd=%d ss.fd=%d\n",polls[i].fd,ss->fd);				
					close(ss->fd);
					ss->fd=-1;
					//将对应的结构体ss         free;
					//还要记得指向null 
					if(ss->str)
					{
						free(ss->str);
						ss->str = NULL;
					}
					free(ss);	
					ss=NULL;
					printf("close...\n");				
				}
				else
				{
					// 具体数据处理操作,返回处理的数据长度
					ret=operate_data(ss,buf);
					//说明没有读取完,继续读
					// 清理read_data中str已处理数据
					
					/*ss->len=0;
					ss->str=(char *)realloc(ss->str,1024);
					//temp->str=(char *)malloc(1024);
					ss->memlen=1024;*/
					if(ret)
					{
						ss->len -= ret;
						memmove(ss->str, ss->str+ret, ss->len);
					}
					//ss->memlen=1024;
					
					
				}

				
			}
		}
	}
}

int main(){
	int sfd;
	//memset(&cache,0,sizeof(cache));
	sfd=socket_init();
	
	hash_init();

	poll_run(poll_init(sfd),sfd);
	
	
	close(sfd);
	return 0;
}


