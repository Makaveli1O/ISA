## ISA - Network Applications and network Administration
# Variant POP3 client with TLS support							

Author: Samuel Líška (xliska20)

# ==Implementation==
Project is created using C language (C99 standard) on Ubuntu system running OpenSSL 1.1.1j, with testing on OpenSSL 1.1.1l - freebsd (BUT eva server). Closer look at implementation is in Manual.pdf file. Program is POSIXLY CORRECT.

# ==Files==
**Globals.h** - Header source file, which holds constants and global variables, such as buffes for varius use, pop3 
constants and openssl related variables. 

**Handler.h** - Header source file that is responsible for C language reated problems. Various things are solved here such as hanlding strings, working with files and parsing arguments. Some memory work is done as well.

**Popcl.c** - Main source file. Actually represents POP3 client. All communications with server if done within this file.

**Makefile** - Default makefile that run compiler.

Compiled with : *-std=gnu99 -Werror -pedantic -g -lssl -lcrypto* !

# ==Usage==
**popcl** *<server> [-p <port>] [-T|-S [-c <certfile>] [-C <certadds>]] [-d] [-n] -a <auth_fie> -o <out_dir>*
Where: 
    <server> - is IP address or domain of source (required) 
    -p *is port*
    -T *enforces encrypted connection with pop3s or -S non-encrypted connection STLS*
    -S *upgrades non-crypted connection to STLS crypted variant*
    -c *defines file with certificates*
    -C *defines folder with certificates for SSL/TLS*
    -d *set program for deletion of messages*
    -n *read only new messages*
    -a *authetification file <auth_file> credentials (required)*
    -o *defines output file <out_dir> (required) \n"*
