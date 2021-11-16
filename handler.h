/*   *   *   *   *   *   *   *   *  *   *   *   *   *   *   *   *  
                        H a n d l e r . h
This header file is responsible for C related problem solving, such
as working with strings, allocating memory, correct freeing memory
directories etc. Also handles input arguments.

@author Samuel Líška(xliska20)
*   *   *   *   *   *   *   *      *   *   *   *   *   *   *   **/
/*std C posix libraries*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>

#include "globals.h"

/*   *   *   *   *   *   *   *   *  *   *   *   *   *   *   *   *   

                        P R O T O T Y P E S

*   *   *   *   *   *   *   *      *   *   *   *   *   *   *   **/
/*functions for argument parsing*/
int argParse(int argc, char *argv[]);                       //parses arguments, and sets global variables accordingly
void createFiles();                                         //creates neccesary files (logs for checking new messages etc.)
int isNumber(char s[]);                                     //finds out whenever given character is number or not
char *strcat3(const char *a, const char *b, const char *c); //concatenance of 3 strings
char *strremove(char *str, const char *sub);                //removes substr from string
void make_directory(const char* name);                      //creates directory with given name
int readFile(char *file);                                   //reads from fileand gets valuable data
char *rtrim(char *str);                                     //trims \n from string
FILE *openFile(char *name, char *mode);                     //creates new @name file with given mode
void closeFile(FILE *name);                                 //closes given file.
void freeAll();                                             //free all global variables

/*   *   *   *   *   *   *   *   *  *   *   *   *   *   *   *   *   

                        D e f i n i t i o n s

*   *   *   *   *   *   *   *      *   *   *   *   *   *   *   **/

/*
    Function that handles parsing arguments. Checks whenever correct
    combination and syntax of arguments are being used.(according to task).
    CAUTION: First option is considered as "source" for program. Others are ignored.
    
    Returns 0 if everything is correct, exits program with 1 otherwise.
*/
int argParse(int argc, char *argv[]){
    char *arg = NULL;
    char *opt;
    int processed = 0; //indicates whenever source was processed or not
    //called without arguments
    if (argc == 1)
    {
        fprintf(stderr,HELP);
        return 1;
    }
    for (int i = 1; i < argc; i++)
    {
        arg = (char*)malloc(strlen(argv[i])*sizeof(char) +1);
        if (arg == NULL)
        {
            fprintf(stderr, "Error: Malloc has failed.\n");
            return 1;
        }
        strcpy(arg, argv[i]);

        //option
        if (arg[0] == '-')
        {
            //these have options after them
            if (arg[1] == 'p' || 
                arg[1] == 'c' || 
                arg[1] == 'C' || 
                arg[1] == 'a' ||
                arg[1] == 'o'   )
            {
                //no optr after arg such as: ./pcap -a
                if (i+1 > argc-1)
                {
                    free(arg);
                    fprintf(stderr, "Error: Wrong arguments input.\n");
                    return 1;
                }
                opt = (char*)malloc(strlen(argv[i+1])*sizeof(char)+1);
                if (opt == NULL)
                {
                    fprintf(stderr, "Error: Malloc has failed.\n");
                    return 1;
                }
                strcpy(opt, argv[i+1]);
                i++; //skip next argv, since it is already processed above
            }

            switch (arg[1])
            {
                case 'p': //port
                    if (isNumber(opt) == 1)
                    {
                        fprintf(stderr, "Error: Invalid port detected.\n");
                        return 1;
                    }else{
                        if (strlen(opt) > 4 || strlen(opt) == 0)
                        {
                            fprintf(stderr, "Error: Invalid port detected.\n");
                            return 1;
                        }else{
                            port = (char*)malloc(strlen(opt)*sizeof(char)+1);
                            if (port == NULL)
                            {
                                fprintf(stderr, "Error: Malloc has failed.\n");
                                return 1;
                            }
                            strcpy(port, opt);
                            free(opt);
                        }
                    }
                    break;
                case 'T':
                    if (S == 1)
                    {
                        fprintf(stderr, "Error: Both S and T parameters used.\n");
                        return 1;
                    }
                    T = 1;
                    break;
                case 'S':
                    if (T == 1)
                    {
                        fprintf(stderr, "Error: Both S and T parameters used.\n");
                        return 1;
                    }
                    S = 1;
                    break;
                case 'c':
                    c = 1;
                    cert = (char*)malloc(strlen(opt)*sizeof(char)+1);
                    if (cert == NULL)
                    {
                        fprintf(stderr, "Error: Malloc has failed.\n");
                        return 1;
                    }
                    strcpy(cert, opt);
                    free(opt);
                    break;
                case 'C':
                    C = 1;
                    certaddr = (char*)malloc(strlen(opt)*sizeof(char)+1);
                    if (certaddr == NULL)
                    {
                        fprintf(stderr, "Error: Malloc has failed.\n");
                        return 1;
                    }
                    strcpy(certaddr, opt);
                    free(opt);
                    break;
                case 'd':
                    d = 1;
                    break;
                case 'n':
                    n = 1;
                    break;
                case 'a':
                    a = 1;
                    auth_file = (char*)malloc(strlen(opt)*sizeof(char)+1);
                    if (auth_file == NULL)
                    {
                        fprintf(stderr, "Error: Malloc has failed.\n");
                        return 1;
                    }
                    strcpy(auth_file, opt);
                    free(opt);
                    break;
                case 'o':
                    o = 1;
                    out_dir = (char*)malloc(strlen(opt)*sizeof(char)+1);
                    if (out_dir == NULL)
                    {
                        fprintf(stderr, "Error: Malloc has failed.\n");
                        return 1;
                    }
                    strcpy(out_dir, opt);
                    free(opt);
                    break;
                }
        }else if(processed == 0){ 
            //source must be processed (1 argument without option)
            source = (char*)malloc(strlen(argv[i])*sizeof(char) + 1);
            if (source == NULL)
            {
                fprintf(stderr, "Error: Malloc has failed.\n");
                return 1;
            }
            strcpy(source, argv[i]);
            processed = 1;
        }
        free(arg);
    }    

    if (source == NULL)
    {
        fprintf(stderr,"Error: Server is not defined.\n");
        return 1;
    }

    //if port is not explicitly declared with -p argument
    //make sure default ports are set properly
    if (port == NULL){
        if(T == 0){
            port = (char*)malloc(strlen(POP3)+1);
            if (port == NULL)
            {
                fprintf(stderr,"Error: Malloc has failed.\n");
                return 1;
            }
            
            strcpy(port, POP3);
        }else{
            port = (char*)malloc(strlen(POP3S)+1);
            if (port == NULL)
            {
                fprintf(stderr,"Error: Malloc has failed.\n");
                return 1;
            }
            
            strcpy(port, POP3S);
        }
    }

    /*check T and S args*/
    if (T == 0 && S == 0)
    {
        /*-c option can be used only when T or S are being used*/

        if (c == 1 || C == 1)
        {
            fprintf(stderr, "Error: Wrong arguments. -c and -C can be used either with -T or -S\n");
            return 1;
        }
        //TODO
        //if neither is used, use enrypted, but does that mean use S?
    }

    if(c == 0 && C == 0){
  
    }

    /*check mandatory arguments -a, -o*/
    if (a == 0)
    {
        fprintf(stderr, "Error: missing mandatory argument. -a\n");
        return 1;
    }
    if (o == 0)
    {
        fprintf(stderr, "Error: missing mandatory argument. -o\n");
        return 1;
    }
    
    return 0;
}

/*
    @strremove
    ---------------
    Removes substring @sub from string @str by moving memory.
    Returns string without given substrs.
*/
char *strremove(char *str, const char *sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

/*
    @strcat3
    ---------------
    Concatenates 3 given strings to one appended.
    @str a is appended by @str b and @str c.
    Returned string must be freed later when not needed.

    Returns pointer to appended string.
*/
char *strcat3(const char *a, const char *b, const char *c) {
    int alen = strlen(a);
    int blen = strlen(b);
    int clen = strlen(c);
    char *res = (char*)malloc(alen + blen + clen + 1);
    if (res == NULL)
    {
        fprintf(stderr, "Error: Malloc has failed.\n");
        exit(1);
    }
    if (res) {
        memcpy(res, a, alen);
        memcpy(res + alen, b, blen);
        memcpy(res + alen + blen, c, clen + 1);
    }

    return res;
}

/*
    @openFile
    ------------------
    Creates new file with @name and @mode.
    Also handles error, exists with return
    code 1 if occures.
*/
FILE *openFile(char *name, char *mode){
    FILE *fp = fopen(name, mode);

    if (fp == NULL)
    {
        freeAll();
        fprintf(stderr, "Error: could not open file %s \n(%s)\n", name, strerror(errno));
        exit(1);
    }

    return fp;
}

/*
    @closeFile
    --------------------
    Closes given file and handles errors.
*/
void closeFile(FILE *name){
    if(fclose(name) != 0){
        fprintf(stderr, "Error: closing file.(%s)\n",strerror(errno));
        freeAll();
        exit(1);
    }
}

/*
    @rtrim
    ----------------
    Trims \n from given string and returns it.
*/
char *rtrim(char *str){
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
        }
    }
    return str;
}

/*
    @make_directory
    -----------------------
    Creates directory with name @name.
*/
void make_directory(const char* name)
{
    //#ifdef __linux__
        mkdir(name, 0777);
    //#else
    //    _mkdir(name);
    //#endif
}

/*  
    @isNumber
    ------------------------------------------------
    Check whenever given stringconrains only numbers.

    Return 0 if its only numbers, 1 otherwise;
*/
int isNumber(char s[]){
    for (int i = 0; i < s[i] != '\0'; i++)
    {
        if (isdigit(s[i]) == 0)//err
        {
            return 1;
        }
    }
    return 0;
}


/*
    @readFile
    ---------------------------------------------
    This function reads from @auth_file file, and checks
    whenever it's inputs are correct (using regex expressions).

    !IMPORTANT! -> records in file must be in exactly this form:
                username = user_example
                password = pass_example
    (spaces between = and words).

    This function also trims line, to only recieve desired username and password,
    which are assigned to global variables @mail_username, and @mail_pw.

    Return 0 on success, 1 if any error occured.
*/
int readFile(char *file){
    FILE *fp = fopen(file, "r");

    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open file %s (%s) \n", file, strerror(errno));

        return 1;
    }
    regex_t regex;
    regex_t passRegex;
    // create regex
    int value = regcomp(&regex, 
        "^username = [^ ]*$", REG_ICASE);   //username
    int userMatch = 0;                      // 0 -> match found , 1 not found
    int passValue = regcomp(&passRegex, 
        "^password = [^ ]*$", REG_ICASE);   //password
    int pwMatch = 0;                        // 0 -> match found , 1 not found
    //error compiling regex
    if (value != 0 || passValue != 0)
    {
        fprintf(stderr, "Error: Regex compilation error.");
        return 1;
    }

    size_t len = 0;
    char *line = NULL;
    int i = 0;
    while ((len = getline(&line, &len, fp)) != -1) {
        i++;
        //remove \n from line
        line = rtrim(line);

        userMatch = regexec(&regex, line, 0, NULL, 0);
        pwMatch = regexec(&passRegex, line, 0 , NULL, 0);
        
        if (userMatch == 1 && pwMatch == 1)
        {
            fprintf(stderr, "Error: auth_file does not contain correct values.");
            return 1;
        }else if (userMatch == 0){
            mail_username = (char*)malloc(strlen(line)*sizeof(char) +1);
            if (mail_username == NULL)
            {
                fprintf(stderr, "Error: Malloc has failed.\n");
                return 1;
            }
            strcpy(mail_username,line);
            //remove "username = " from beginning
            int n = 11; //"username = " -> len 11
            size_t len = strlen(mail_username);
            if(11 > len) n = len;
            memmove(mail_username, mail_username+n, len - n + 1);
        }else if(pwMatch == 0){
            mail_pw = (char*)malloc(strlen(line)*sizeof(char) +1);
            if (mail_pw == NULL)
            {
                fprintf(stderr, "Error: Malloc has failed.\n");
                return 1;
            }
            strcpy(mail_pw, line);
            //remove "password = " from beginning
            int n = 11; //"password = " -> len 11
            size_t len = strlen(mail_pw);
            if(11 > len) n = len;
            memmove(mail_pw, mail_pw+n, len - n + 1);
        }

        if (i > 2){
            fprintf(stderr, "Error: File contains more than 2 records.\n");
            return 1;
        }
        
    }
    // close the file and cleanup
    free(line);
    regfree(&regex);
    regfree(&passRegex);
    fclose(fp);
    //username or password not recognized
    if (mail_username == NULL || mail_pw == NULL)
    {
        fprintf(stderr, "Error: Credentials could not be retrieved from given file.\n");
        return 1;
    }
    
    return 0;
}

/* create downloaded.txt and files.txt */
void createFiles(){
    FILE* downl = openFile("downloaded.txt","a");
    closeFile(downl);
}

/*free all global variables*/
void freeAll(){
    // in case if somehow program tries to free
    // non allocated memory, check it first 
    if (source != NULL)
        free(source);
    if (port != NULL)
        free(port);
    if (certaddr != NULL)
        free(certaddr);
    if (auth_file != NULL)
        free(auth_file);
    if (out_dir != NULL)
        free(out_dir);
    if (mail_pw != NULL)
        free(mail_pw);
    if (mail_username != NULL)
        free(mail_username);
}

