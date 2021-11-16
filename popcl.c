/*   *   *   *   *   *   *   *   *  *   *   *   *   *   *   *   *  
                        p o p c l . c
Main source code representing pop3 client. All communication with 
server is done within this file. In other file are handled C rel-
evant problems. This client does non-crypted as well as TLS and SSL
crypted communications.

@author Samuel Líška(xliska20)
*   *   *   *   *   *   *   *      *   *   *   *   *   *   *   **/

/*   *   *   *   *   *   *   *   *  *   *   *   *   *   *   *   *   

                        P R O T O T Y P E S

*   *   *   *   *   *   *   *      *   *   *   *   *   *   *   **/

#include "handler.h"    //c relevant problems

int establishConnection();                                  //starts comm. and establish connection
int loginPop3(char* username, char*password);               //login to pop3 server
void closePop3();                                           //closes pop3 session
int writeBIO(char *str);                                    //writing to bio socket with failure check
int readBIO();                                              //reading response from bio socket
int checkResponse();                                        //handles response from poop3 server 
int retrieveMails();                                        //list pop3 command to retrieve number of messages 
int deleteMails();                                          //deletes messages on server (-d)
char *getMsgId(char *cmd);                                  //scans through msg and retrieves its ID
int readRetr(int readCount);                                //read buffer and write it to @bio output file
void writeBuffer(FILE *out);                                //writes content from buffer to file
FILE* createMailFile(char *outputName);                     //creates new bio output(file) which is mail being written to
int firstIntWithinPOP3(char *str);                          //find first integer within string
void appendPattern(int mode, int len, char ch);             //appends searching pattern
char* retrieveMessageId(char *cmd);                         //retrieves message-id from header and saves it in downloaded.txt file
char *retrieveIntel(char *ptrn, char endingChar, char* cmd, int mode);  //retrieves informations from msg
int saveIntoFile(char *filename, char *msg, int type);      //saves retrieved msgid to file
int messageIdCheck(char *file, char *msg);                  //check whenever message id is already in file or not
int recordWithinFile(char *f, char *filename);              //check whenever record is within file
int setupSecuredConnection(char *conn);                     //setup context, verify certificates etc. (TLS,SSL con)

int main(int argc, char *argv[])
{
    
    //check input arguments
    if (argParse(argc, argv) != 0){
        freeAll();
        exit(1);
    }
    //create output directory
    make_directory(out_dir);
    createFiles();
    
    //read authetification intel and assign to @mail_username and @mail_pw
    if(readFile(auth_file) != 0){
        freeAll();
        exit(1);
    }
    //establish connection
    if (establishConnection(source, port) != 0){
        freeAll();
        exit(1);
    }
    //login to pop3 server with proper creditials
    if (loginPop3(mail_username, mail_pw) != 0){
        fprintf(stderr, "Error: Logging in to pop3 server has failed.");
        freeAll();
        exit(1);
    } 
    int messages = 0;
    //delete flag
    if(d == 1){
        messages = deleteMails();
    }else{
    //retrieve mails, and save them to files accordingly
        
        if ((messages = retrieveMails()) < 0 ){
            fprintf(stderr, "Error: Error within retrieving mails. \n");
            freeAll();
            exit(1);
        }
    }

    //DELETED
    if (d == 1)
    {
        if (n == 1){
            printf("Smazáno %d nových zpráv.\n",messages);
        }else{
            printf("Smazáno %d zpráv.\n",messages);
        }
    }else{
    //DOWNLOADED
        if (n == 1)
        {
            printf("Staženo %d nových zpráv.\n",messages);
        }else{
            printf("Staženo %d zpráv.\n",messages);
        }
    }
    

    closePop3();
    freeAll();

    return 0;
}

/*
    @recordWithinFile
    --------------------
    Checke whenever @record is within file @f, so 
    program can skip processing it later.
    Return 1 if  record is found within file, 0 if not.
*/
int recordWithinFile(char *f, char *record){
    FILE* file = openFile(f, "r");
    char c;
    char *word = NULL;
    int i = 0;
    while ( (c = fgetc(file)) != EOF)
    {
        i++;
        word = realloc(word, sizeof(char) * i);
        if (c == '\n')  
        {
            word[i-1] = '\0';
            //found given record within file
            if (strcmp(word, record) == 0)
            {
                closeFile(file);
                free(word);
                return 1;
            }else{//cointinue searching
                
                i = 0;
                continue;
            }
        }
        word[i-1] = c;
    }
    closeFile(file);
    free(word);
    return 0;
}

/*
    @deleteMails
    --------------------
    Handles -d argument. Deletes mails in the mailbox(not downloaded ones).
    When combined with -n, only new messages are deleted.

    Returns number of deleted messages.
*/
int deleteMails(){
    int deleted = 0; //number of downloaded messages
    char *output_path;
    char number[12]; //64 bit long is being used
    FILE *out = NULL;
    //LIST cmd
    writeBIO("stat\r\n");
    readBIO();
    //number of messages within mailbox (from stat cmd)
    int numMessages = firstIntWithinPOP3(buf);
    if (numMessages < 0)
    {
        fprintf(stderr, "Error: Retrieving number of messages with firstIntWithinPOP3 func.\n");
        return -1;
    }
    //delete command
    for(int i = 1; i <= numMessages; i++){
        sprintf(number, "%d", i);
        //delete only new
        if(n == 1){
            char * cmd = "retr ";
            cmd = strcat3(cmd, number, "\r\n");
            //write retr "x" cmd to pop3 server
            writeBIO(cmd);
            readBIO(); //need its name to create file
            char *msgid = getMsgId(cmd);
            int old = messageIdCheck("downloaded.txt", msgid);

            //!old = new
            if(old == 0){
                memset(cmd, 0, strlen(cmd));
                cmd = "dele ";
                cmd = strcat3(cmd, number, "\r\n");
                writeBIO(cmd);
                readBIO();
                deleted++;
            }
            free(msgid);
        // -n is not set -> delete all
        }else{
            
            char * cmd = "dele ";
            cmd = strcat3(cmd, number, "\r\n");
            writeBIO(cmd);
            readBIO();
            deleted ++;
        }
    }
    return deleted;
}

/*
    @retrieveMails
    -----------------------
    Performs retr "x" command to retrieve mails. First "stat" cmd is performed to
    find out how many messages to retrieve. Next one by one each message is being processed
    until ending o fmulti-line response is found.

    Returns number of downloade messages on success, -1 if error occured.
*/

int retrieveMails(){
    int downloaded = 0; //number of downloaded messages
    char *output_path;
    char number[12]; //64 bit long is being used
    FILE *out = NULL;
    //LIST cmd
    writeBIO("stat\r\n");
    readBIO();
    //number of messages within mailbox (from stat cmd)
    int numMessages = firstIntWithinPOP3(buf);
    if (numMessages < 0)
    {
        fprintf(stderr, "Error: Retrieving number of messages with firstIntWithinPOP3 func.\n");
        return -1;
    }
    //RETR "x" cmd
    for(int i = 1; i <= numMessages; i++){
        //reset pattern buffer when new message is found since old \r\n.\r\n could be found here
        memset(pattern, 0, strlen(pattern));
        int writtenInMsgs = 0;
        //convert counter to string 
        sprintf(number, "%d", i);
        char * cmd = "retr ";
        cmd = strcat3(cmd, number, "\r\n");
        //write retr "x" cmd to pop3 server
        writeBIO(cmd);
        readBIO();
        char *msgid = getMsgId(cmd);
 
        free(cmd);
        if (msgid != NULL )
        {
            //if(out != NULL) closeFile(out);
            writtenInMsgs = messageIdCheck("downloaded.txt", msgid);    //flag if msgid is present already

            output_path = strcat3(out_dir,"/",msgid);
            //OLD FILE file already exists -> skip and /n is set
            if(writtenInMsgs == 1 && n == 1 )
            {
                free(output_path);
                free(msgid);
                continue;
            //NEW FILE
            }else{
                //check if file already exist(without -n mode)
                if( access(output_path, F_OK ) == 0){
                    free(output_path);
                    free(msgid);
                    continue;
                }
                //create it
                out = openFile(output_path, "a");
                //add to files.txt
                saveIntoFile("downloaded.txt",msgid, 0);
            }
            free(msgid);
        }else{//Output name is null
            continue;
        }

        int first = 1;
        int found = 0;
        int readCount = 1; //number of reading same message (if its too long, or sent within multiple packets)
        do{
            int written = 0;    //already written flag
            if(first != 1){  //skip first ( we've read that when creating output file in createMailFile)
                readCount++;
                readBIO();
            }

            if (n == 1)
            {
                //0 -> new message, or same message but it continues
                if(writtenInMsgs == 0 || writtenInMsgs == 1 && readCount > 1){
                    writeBuffer(out);
                    written = 1;
                    if (readCount == 1)
                    {
                        downloaded++;
                    }
                    
                }
            // -n is not being used, download all that mailbox has
            }

            //read and write to file
            found = readRetr(readCount);
            if(written == 0){
                writeBuffer(out);
                if(readCount == 1){
                    downloaded++;
                }
            }
            first = 0;
        }while(found != 1);
        free(output_path);
    }
    if (out != NULL)
    {
        closeFile(out);
    }
    return downloaded;
}

/*
    @createMailFile
    ---------------------------
    Create file within @out_dir direction, with subject name of email,
    retrieved by @getSubjName function.

    @Returns newly created output file, or exit with code 1, indicating error
    followed with error message printed on stderr. If file already exsts, return NULL.
*/
FILE *createMailFile(char *outputName){
    char *output_name = strcat3(out_dir,"/",outputName);
    //create bio output file
    FILE *out = openFile(output_name, "a");
    if(out == NULL){
        fprintf(stderr, "Error: file could not be created. \n");
        exit(1);
    }
    free(output_name);
    return out;
}

/*
    @readRetr
    ---------------------
    This function reads BIO socket chat by char. It is called after
    reading response from retr "x" command being send to pop3 server.
    First line is skipped, since it is +OK / -ERR response, which is
    being handled by readBIO function.
    Other characters are written in given output.
    \r\n.\r\n is considered as end. and .CRLF is not part of response!
    mode -> 1- look for "\r\n.\r\n"
            2- look for "Message-ID: <"
            3- look for "Subject: "
*/
int readRetr(int readCount){  
    int firstLine = 1;
    for (size_t i = 0; i < strlen(buf); i++)
    {
        //skip first line +OK ....
        if (firstLine == 1 && readCount == 1)
        {
            if (buf[i] == '\n')
            {
                firstLine = 0;
            }
            continue;
        }

        //search for \r\n.\r\n
        appendPattern(1, 5, buf[i]);
        if (strcmp(pattern,"\r\n.\r\n") == 0)
            return 1;
        

    }
    return 0;
}


/*
    @writeBuffer
    -----------------------
    Writes content of the buffer @buf into
    the file. This function also handle removing unneccesary parts
    of response such as +OK .... and \r\n.\r\n at the end of response
    which is not considered as part of response.

    Also handles bytestuffing.
*/

void writeBuffer(FILE *out){

    // end of response is divided
    if (buf[0] == '.' || buf[1] == '\r' || buf[2] == '\n')
    {
        return;
    }
    
    int merge = 0;
    
    //check for +OK xxx octets ... to remove
    regex_t regex;
    regex_t regex2;
    //OK response trim
    int value = regcomp(&regex,"+OK ", REG_ICASE);
    //if \r\n.[a-zA-Z][0-9]* -> bytestuffed
    int bytestuffing = regcomp(&regex2, "\r\n\\.\\.", REG_ICASE);

    if (value != 0 || bytestuffing != 0)
    {
        fprintf(stderr, "Error: Regex compilation error.\n");
        exit(1);
    }

    int match; // 0 -> match found , 1 not found
    match = regexec(&regex, buf, 0, NULL, 0);
    int bytestuffed = regexec(&regex2, buf, 0 ,NULL , 0);

    regfree(&regex);
    regfree(&regex2);

    if (strcmp(pattern, "\r\n.\r\n") == 0)
    {
        //ending is only in one answer
        if (strstr(buf, "\r\n.\r\n") != NULL)
        {
            strremove(buf,".\r\n");
        }
    }
    
    //bytestuffed
    if(bytestuffed == 0){
        strremove(buf, "..");
    }

    //+ok .. found
    if (match == 0)
    {
        char *str;
        //should be first line
        //skip it
        if((str = strstr(buf,"\r\n")) != NULL){
            if(fprintf(out, "%s", str+strlen("\r\n")) == -1){
                fprintf(stderr, "Error: writing to out bio.");
                exit(1);
            }
        }
    }else{
        //Print regulary those where answer OK isnt present
        if(fprintf(out, "%s", buf) == -1){
            fprintf(stderr, "Error: writing to out bio.");
            exit(1);
        }
    }
}

/*
    @appendPattern
    -----------------------
    Appends string, which is constantly being checked for presence of
    "\r\n.\r\n" which indicates end of POP3 multi-line message.

    @mode ->    1 - end of message "\r\n.\r\n"
                2 - "Message-ID: <"
                3 - "Subject: " 
*/
void appendPattern(int mode, int len, char ch){
    
    for (int i = 0; i < len; i++)
    {
        if (i == 0){ //this char needs to be terminated (most left)
            continue;
        }else if(i == len -1){
            switch (mode)
            {
            case 1: //pattern
                pattern[i-1] = pattern[i];
                pattern[i] = ch;
                break;
            
            case 2: //messageID
                idPattern[i-1] = idPattern[i];
                idPattern[i] = ch;
                break;
            case 3: //subject
                subjectPattern[i-1] = subjectPattern[i];
                subjectPattern[i] = ch;
                break;
            }
        }else{
            switch (mode)
            {
            case 1: //pattern
                pattern[i-1] = pattern[i];
                break;
            
            case 2: //messageID
                idPattern[i-1] = idPattern[i];
                break;
            case 3: //subject
                subjectPattern[i-1] = subjectPattern[i];
                break;
            }
        }
    }
    return;
}

/*
    @retrieveMEssageId
    -------------------------
    Reads header response and get it's message ID, 
    for further use (reading new mails.)

    !retrieved message must be freed in order to avoid memory leak!
    Return messageID if it was found within mail. Returns "none" if 
    not found.
*/
char *retrieveMessageId(char *prev){
    char *msgid = NULL;
    if ((msgid = strstr(buf, "Message-ID: <")) != NULL)
    {
        char *beg = strchr(msgid, '<');
        int b_indx = (int)(beg - msgid);// beginning index of msgid
        char *end = strchr(msgid, '>');
        int e_indx = (int)(end - msgid);// ending index of msgid
        int len = e_indx - b_indx;      //len of output string
        //char msg[len];
        char *msg = (char*)malloc(sizeof(char)*len +1);
        if (msg == NULL)
        {
            fprintf(stderr,"Error: Malloc has failed.\n");
            exit(1);
        }
        

        //write to string
        int j = 0;
        for (int i = b_indx+1; i < e_indx; i++)
        {   
            j = i-(b_indx+1);
            msg[j] = msgid[i];
            
        }
        msg[j+1] = '\n';
        msg[j+2] = '\0';
        return msg;
    }

    return prev;

}

/*
    @saveIntoFile
    ------------------------
    Save @msg to file with given @filename.
    Returns 0 on success. 1 if message is already
    within file. Handles duplicated within recorded ids and filenames.
    @type -> message/filename being written , 0-> message & 1-> filename
*/
int saveIntoFile(char *filename, char *msg, int type){
    FILE *file = openFile(filename, "a");
    //write string containing message-id to downloaded.txt
    //avoiding duplicates within file.
    if (type == 0){
        if (messageIdCheck(filename, msg) == 0)
        {
            //is newline is missing print it 
            if (strchr(msg, '\n') == NULL)
                fprintf(file,"%s\n",msg);
            else //already within msg
                fprintf(file, "%s",msg);
            closeFile(file);
            return 1;
        }
    }else{//filename check in files.txt
        if (recordWithinFile(filename, msg) == 0){
            //is newline is missing print it 
            if (strchr(msg, '\n') == NULL)
                fprintf(file,"%s\n",msg);
            else //already within msg
                fprintf(file, "%s",msg);
            closeFile(file);
            return 1;
        }
    }
    closeFile(file);
    return 0;
}

/*
    @messageIdCheck
    --------------------------
    Check whenever filename @file contains given message ID.

    Return 1 if it does, 0 otherwise.
*/
int messageIdCheck(char *file, char *msg){ 
    //skip "none" string
    if (msg == NULL)
    {
        return 1;
    }
      
    FILE *read = openFile(file, "r");

    //according to POP3 documentation, unique Message-ID is 1 - 70 characters long, so this buffer sholdnt overflow
    char fbuf[100]; 
    char cpy[100];
    while ((fgets(fbuf, 100, read)!=NULL))
    {
        strcpy(cpy, fbuf);
        cpy[strlen(cpy)-1] = '\0';
       // printf("msgid: %s %ld , written: %s %ld\n",msg,strlen(msg), fbuf, strlen(fbuf));
        if(strcmp(msg, fbuf) == 0){
            closeFile(read);
            return 1;
        }
        //without \n
        if(strcmp(msg,cpy) == 0){
            closeFile(read);
            return 1;
        }
    }
    closeFile(read);
    
    return 0;
}

/*
    @retrieveIntel
    --------------------
    Retrieves @string after specific pattern until given character from within whole
    retr message answer. Afterwards sets back state of BIO buffer as was before.
*/
char *retrieveIntel(char *ptrn, char endingChar, char *cmd, int mode){
    int found = 0;
    char *tmp = NULL;
    char *ptr = NULL;
    int j = 0;
    do{ 
        for (size_t i = 0; i < strlen(buf); i++)
        {
            //subject pattern now continues
            if ((strstr(subjectPattern, ptrn) != NULL || strstr(idPattern, ptrn) != NULL)&& found != 1)
            {
                j++;
                tmp =realloc(tmp,sizeof(char) * j );
                tmp[j-1] = buf[i];
                if (buf[i] == endingChar)
                {
                    tmp[j-1] = '\0';
                    found = 1;
                } 
                
            }else if (found == 0){
                if(mode == 3) appendPattern(3, 10, buf[i]);//appending subjectPattern
                else if (mode == 2) appendPattern(2, 14, buf[i]);//appending idPattern
                else if (mode == 4) appendPattern(4, 6, buf[i]);//appending idPattern
            }
            //search for \r\n.\r\n
            appendPattern(1, 5, buf[i]);
        }
        if (strcmp(pattern, "\r\n.\r\n") != 0)
        {
            readBIO();
            found = 0;
        }
    }while(strcmp(pattern, "\r\n.\r\n") != 0);
    memset(subjectPattern, 0, strlen(subjectPattern));
    memset(idPattern, 0, strlen(idPattern));
    return tmp;
}



/*
    @getSubjName
    ------------------------
    Retrieve subject name from answer, and returns it
    as a string for further use. 
    !Returned string must be freed afterwards.
*/
char *getSubjName(char *cmd){
    char *tmp = retrieveIntel("Subject: ", '\n', cmd, 3);
    
    //get bio back like it was before
    writeBIO(cmd);
    readBIO();
    return tmp;
}

char *getMsgId(char *cmd){
    char *tmp = retrieveIntel("Message-ID: <", '>', cmd, 2);

    //get bio back like it was before
    writeBIO(cmd);
    readBIO();
    return tmp;
}


/*
    @loginPop3
    ---------------------------------
    Attempts to login to pop3 server via bio socket with given creditials.
    If everything goes well, returns 0 on success.
    If error occured return 1;
*/
int loginPop3(char *username, char* password){
    int err;
    char *user = strcat3("USER ", username, "\r\n");
    err = writeBIO(user);
    if (err != 0)
        return 1;
    readBIO(bio);

    char *pass = strcat3("PASS ", password, "\r\n");
    err = writeBIO(pass);
    if (err != 0)
        return 1;
    readBIO(bio);

    free(user);
    free(pass);
    return 0; 
}

void closePop3(){
    writeBIO("quit \r\n");
    if (T == 1 || S == 1)
    {
        SSL_CTX_free(ctx);
    }else{
    BIO_free(bio);
    }
    return;
}

/*
    @establishConnection
    -----------------------------------------------
    This function opens connection for given @var host
    on given @var _port. If anything goes wrong, error
    message is printed to stderr, and 1 returned.

    Return 0 is connection is connected, 1 if something went wrong.
*/
int establishConnection(char* hostname, char* _port){
    //merge hostname and port, since it is required as one string
    char *conn = strcat3(hostname, ":", port);

    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
    // -T parameter pop3s SSL
    if (T == 1)
    {
        if(setupSecuredConnection(conn) > 0){
            free(conn);
            return 1;
        }
    //basic POP3
    }else{
        bio = BIO_new_connect(conn);
    }
    if (bio == NULL)
    {
        // handle failure 
        fprintf(stderr, "Error: Creating new connection has failed.\n");
        free(conn);
        return 1;
    }
    BIO_do_connect(bio);

    if (BIO_do_connect(bio) <= 0)
    {
        BIO_free_all(bio);
        free(conn);
        //handle failed connection
        fprintf(stderr, "Error: Failed to establish connection.\n");
        return 1;
    }
    readBIO();
    //STLS connection (upgrade basic non secured connection)
    if (S == 1)
    {
        
        //setup STLS
        char *cmd = "STLS\r\n";
        writeBIO(cmd);
        //recieve answer (OK begin TLS negotiation) if successfull
        readBIO();
        BIO *ret = NULL, *sslC = NULL;
        //set context and verify certificates
        ctx = SSL_CTX_new(SSLv23_client_method());
        SSL_CTX_set_default_verify_paths(ctx);
        if((sslC = BIO_new_ssl(ctx, 1)) == NULL){
            fprintf(stderr,"Error: Creating new bio SSL context.\n");
            return 1;
        }
        if((ret = BIO_push(sslC, bio)) == NULL){
            fprintf(stderr,"Error: Pushing ssl context to bio.\n");
            return 1;
        }
        

        //take newly created SSL
        BIO_get_ssl(ret, &ssl);

        //check connection after upgrading
        if (BIO_do_connect(ret) <= 0)
        {
            BIO_free_all(ret);
            free(conn);
            //handle failed connection
            fprintf(stderr, "Error: Failed to establish connection.tls\n");
            return 1;
        }

        bio = ret;
    }
    
    free(conn);
    return 0;
}

/*
    @setupSecuredConnection
    ------------------------------
    Setop context, verify arguments and files with certificates. Verify if certificates
    vere correctly verified. Returns 1 if error occured.

    If everything is Ok return 0;
*/
int setupSecuredConnection(char *conn){
    ctx = SSL_CTX_new(SSLv23_client_method());
    //certificates within -c arg file
    // -c "" input
    if (c == 0 && C == 0)
    {
        SSL_CTX_set_default_verify_paths(ctx);
    }else{
        if (! SSL_CTX_load_verify_locations(ctx, cert, NULL))
        {
            SSL_CTX_free(ctx);
            fprintf(stderr, "Error: Failed loading verify loactions.(-c)\n");
            return 1;
        }
        //certificates within -C arg dir
        if (! SSL_CTX_load_verify_locations(ctx, NULL, certaddr))
        {
            SSL_CTX_free(ctx);
            fprintf(stderr, "Error: Failed loading verify loactions.(-C)\n");
            return 1;
        }
    }
    bio = BIO_new_ssl_connect(ctx);
    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    BIO_set_conn_hostname(bio, conn);

    //verify certificates
    if (SSL_get_verify_result(ssl) != X509_V_OK)
    {
        fprintf(stderr, "Error: Cirtificates could not be verified\n");
        return 1;
    }

    return 0;
}

/*
    @writeBIO
    ---------------------------------------
    This function attempts to write to server via bio socket.
    If message is successfully written, 0 is returned.
    Otherwise function exits with error code 1;
*/
int writeBIO(char *str){
    if (BIO_write(bio,str,strlen(str)) <= 0)
    {
        if (!BIO_should_retry(bio))
        {
            fprintf(stderr, "Error: Problem occured while trying to write to bio.\n");
            freeAll();
            exit(1);
        }
    }
    return 0;
}

/*
    @readBIO
    ---------------------------------
    This function attempts to read from bio socket single line,
    and fill that response into the buffer. If error occured, program exits with
    error code 1.

    If reading was successful, function returns 0.
*/
int readBIO(){
    memset(buf, 0, strlen(buf));
    int x = BIO_read(bio,buf,BUFF_SIZE);

    if (x == 0 )
    {
        fprintf(stderr, "Error: Trying to read from closed connection.\n");
        freeAll();
        exit(1);
    }else if(x < 0){
        if (! BIO_should_retry(bio))
        {
            fprintf(stderr, "Error: Reading from bio socked has failed.\n");
            freeAll();
            exit(1);
        }
    }
    if (checkResponse() != 0)
    {
        fprintf(stderr, "Error: Incorrect command / typo in command detected.\n Or wrong username / password has been entered.\n Server responded with -ERR.\n");
        freeAll();
        exit(1);
    }
    
    return 0;
}

/*
    @checkResponse
    -------------------------------
    POP3 server must answer with "+OK" for correct command or
    "-ERR" for incorrect one.
    This function returns 1 if incorrect command was entered,
    and 0 if everrything goes well. This function is called after
    every read.
*/
int checkResponse(){
    //-ERR returned
    if (buf[0] == '-' && buf[1] == 'E' && buf[2] == 'R' && buf[3] == 'R'){
        fprintf(stderr, "SERVER RESPONSE: %s\n",buf);
        return 1;
    }else
        return 0;
}

/*
     @firstIntWithinPOP3
    --------------------------
    Finds first int occurence within given answer from pop3 and returns it.
*/
int firstIntWithinPOP3(char *str){
    int num = 0;
    char *tmp = (char*)malloc(strlen(str)*sizeof(char) +1);
    if (tmp == NULL)
    {
        fprintf(stderr, "Error: Malloc has failed.\n");
        return -1;
    }
    for (int i = 4; i < strlen(str); i++) //4 because pop will answer "+OK "
    {
        tmp[i-4] = str[i];
    }
    int ret = atoi(tmp);
    free(tmp);
    return ret;
}