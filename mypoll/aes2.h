
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <openssl/aes.h>

unsigned char* str2hex(char *str) {//עÊ1
	//16���Ƶ��ִ�ת����32byte�洢����

    unsigned char *ret = NULL;
    int str_len = strlen(str);
    int i = 0;
    assert((str_len%2) == 0);
    ret = (char *)malloc(str_len/2);
    for (i =0;i < str_len; i = i+2 ) {
        sscanf(str+i,"%2hhx",&ret[i/2]);
    }
	//printf("ret=%s\n",ret);
    return ret;
}
/*#define KEY_BLOCK_BIT 33

unsigned char key[KEY_BLOCK_BIT] = "E10ADC3949BA59ABBE56E056F20F883E";*/


char *padding_buf(char *buf,int size, int *final_size) {//עÊ2
    char *ret = NULL;
    int pidding_size = AES_BLOCK_SIZE - (size % AES_BLOCK_SIZE);
    int i;
    *final_size = size + pidding_size;
    ret = (char *)malloc(size+pidding_size);
    memcpy( ret, buf, size);
    if (pidding_size!=0) {
        for (i =size;i < (size+pidding_size); i++ ) {
            ret[i] = 0;
        }
    }
    return ret;
}
void printf_buff(char *buff,int size) {
    int i = 0;
    for (i=0;i<size;i ++ ) {
        printf( "%02X ", (unsigned char)buff[i] );
        if ((i+1) % 8 == 0) {
            printf("\n");
        }
    }
    printf("\n\n\n\n");
}
void encrpyt_buf(char *raw_buf, char *encrpy_buf, int len ) {
    AES_KEY aes;
   // unsigned char *key = str2hex("8cc72b05705d5c46f412af8cbed55aad667b02a85c61c786def4521b060265e8");  //��������32���ֽ�
    unsigned char *key ="667b02a85c61c786def4521b060265e8";

    //unsigned char *iv = str2hex("667b02a85c61c786def4521b060265e8");
    AES_set_encrypt_key(key,256,&aes);//עÊ3
    AES_ecb_encrypt(raw_buf,encrpy_buf,&aes,AES_ENCRYPT);
    //AES_cbc_encrypt(raw_buf,*encrpy_buf,len,&aes,iv,AES_ENCRYPT);
   // free(key);
    //free(iv);
}
/*int encrpyt_buf(char* str_in, char** str_out,int len)
{
  	//����Ƿ��� ���� KEY ����  ����1ΪNULL���˳�
    if (!str_in || !key || !str_out) return 0;
    
    //��ȡ����
    char aes_encode_temp[1024]; 
    strcpy(aes_encode_temp,str_in);

 	
    //ͨ���Լ�����Կ���һ��aes��Կ�Թ��������ʹ��
    AES_KEY aes;
	
    if (AES_set_encrypt_key((unsigned char*)key, 256, &aes) < 0)//256��ʾ32λ�ַ���Կ
    {
    	printf("en set error...\n");
        return 0;
    }
 
    //���ܽӿڣ�ʹ��֮ǰ��õ�aes��Կ
    AES_ecb_encrypt((unsigned char*)aes_encode_temp, (unsigned char*)*str_out, &aes,AES_ENCRYPT);
	return 1;
}
*/
void decrpyt_buf(char *raw_buf, char *encrpy_buf, int len ) {
    AES_KEY aes;
   	//unsigned char *key = str2hex("8cc72b05705d5c46f412af8cbed55aad667b02a85c61c786def4521b060265e8");
	unsigned char *key ="667b02a85c61c786def4521b060265e8";
    //unsigned char *iv = str2hex("667b02a85c61c786def4521b060265e8");
    AES_set_decrypt_key(key,256,&aes);
	AES_ecb_encrypt(raw_buf,encrpy_buf,&aes,AES_DECRYPT);
    //AES_cbc_encrypt(raw_buf,*encrpy_buf,len,&aes,iv,AES_DECRYPT);
    //free(key);
    //free(iv);
}
/*int decrpyt_buf(char* str_in, char** str_out,int len)
{
    if (!str_in || !key || ! str_out)    return 0; 
 

    //ͨ���Լ�����Կ���һ��aes��Կ�Թ��������ʹ�ã�128��ʾ16�ֽ�
    AES_KEY aes; 

    if (AES_set_decrypt_key((unsigned char*)key, 256, &aes) < 0)//�ɹ�����0
    {
    	printf("de set error...\n");
        return 0;
    }
    
    char aes_encode_temp[1024]; 
    strcpy(aes_encode_temp,str_in);
  
    
    //����ǽ��ܽӿڣ�ʹ��֮ǰ��õ�aes��Կ
    AES_ecb_encrypt((unsigned char*)aes_encode_temp, *str_out, &aes,  AES_DECRYPT);
	return 1;
}
*/
/*int main(int argn, char *argv[] ) {
    char *raw_buf = NULL;
    char *after_padding_buf = NULL;
    int padding_size = 0;
    char *encrypt_buf = NULL;
    char *decrypt_buf = NULL;
    // 1
    raw_buf = (char *)malloc(17);
    memcpy(raw_buf,"life's a struggle",17);
    printf("------------------raw_buf\n");
    printf_buff(raw_buf,17);
    // 2
    after_padding_buf = padding_buf(raw_buf,17,&padding_size);
    printf("------------------after_padding_buf\n");
    printf_buff(after_padding_buf,padding_size);
    // 3
    encrypt_buf = (char *)malloc(padding_size);
    encrpyt_buf(after_padding_buf,&encrypt_buf, padding_size);
    printf("------------------encrypt_buf\n");
    printf_buff(encrypt_buf,padding_size);
    // 4
    decrypt_buf = (char *)malloc(padding_size);
    decrpyt_buf(encrypt_buf,&decrypt_buf,padding_size);
    printf("------------------decrypt_buf\n");
    printf_buff(decrypt_buf,padding_size);
    free(raw_buf);
    free(after_padding_buf);
    free(encrypt_buf);
    free(decrypt_buf);
    return 0;
}*/
