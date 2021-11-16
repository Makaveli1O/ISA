/*   *   *   *   *   *   *   *   *  *   *   *   *   *   *   *   *  
                        g l o b a l s . h
This header file is holds constants and global variables, such as
buffers for various use, pop3 constants and openssl related vars.

@author Samuel Líška(xliska20)
*   *   *   *   *   *   *   *      *   *   *   *   *   *   *   **/
//openssl library
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>

#define HELP " Help manual, how to use this program:                                                \
                \n ------------------------------------------------------------                     \
                \n popcl <server> [-p <port>] [-T|-S [-c <certadds>]] [-d] [-n] -a <auth_fie> -o    \
                \n Where:                                                                           \
                \n <server> - is IP address or domain of source (required)                          \
                \n -p is port                                                                       \
                \n -T encrypted connection with pop3s or -S non-encrypted connection STLS           \
                \n -c defines file with certificates                                                \
                \n -C defines file with certificates for SSL/TLS                                    \
                \n -d set program for deletion of messages                                          \
                \n -n read only new messages                                                        \
                \n -a resets authetification, contents of <auth_file> is printed.(required)         \
                \n -o defines output file <out_dir> (required) \n"
#define POP3 "110"              //and TLS
#define POP3S "995"             //POP3s ssl

#define BUFF_SIZE 1024          //size of reading buffer used throughthout the app

BIO *bio;                       //prior to setting up a connection (whenever secured or not) bio object needs to be created
SSL_CTX *ctx;                   //context
SSL *ssl;                       //ssl

char* source = NULL;            //<server>(required)
char* port = NULL;              //<port>(optonal)
int T = 0;                      //with encryption(optional)
int S = 0;                      //without encryption(optional)
int c = 0;                      //flag for -c is being used(optional)
char* cert = '\0';              //certificate for T and S options
int C = 0;                      //flag for -C is being used(optional)
char* certaddr = '\0';          //file containing certificates
int d = 0;                      //flag indicating message deletion(optional)
int n = 0;                      //flag indicating read only new messages(optional)
int a = 0;                      //flag -a is being used that forces authentication(required)
char *auth_file = NULL;         //configuration file
int o = 0;                      //flag that -o is being used(required)
char *out_dir = NULL;           //output directory
char *mail_username = NULL;     //username for email account (found within auth_file)
char *mail_pw = NULL;           //password for email account (found within auth_file)

char buf[BUFF_SIZE];                        //buffer
char pattern[5] = "0000";                   //pattern buffer
char idPattern[14] = "0000000000000";       //messageId buffer
char subjectPattern[10] = "000000000";      //subject buffer