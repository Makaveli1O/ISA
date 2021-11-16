popcl:
	$ gcc -o popcl popcl.c -std=gnu99 -Werror -pedantic -g -lssl -lcrypto 
clean:
	-rm -f popcl