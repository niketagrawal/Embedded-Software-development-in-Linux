/*
* This code implements a secure program loader that only executes programs which
have been signed and rejects programs with missing or wrong signature.
In order to do so, first, a RSA public/private key pair is created. In the first step, private key 
is created
openssl genpkey -algorithm RSA -out private_key.pem
-pkeyopt rsa_keygen_bits:2048

This key needs to remain a secret. Set the file system access permissions so that only you can read
it:
chmod 400 private_key.pem

Now the public key can be extracted from the private key:
openssl rsa -pubout -in private_key.pem -out public_key.pem

The idea behind secured programs is to create a secure digest (hash) using openssl and embedding
the hash into the binary as an ELF section named .sha. The loader can now, with the knowledge
of the public key which can be made available for verification purposes by the author of the binary,
verify the integrity of the program binary before executing it. Binaries that do not contain a valid
secure digest or no digest at all should be rejected by the loader.
Please note that adding a header to the ELF binary is a modification and therefore the header needs
to be removed again before verification. The intended usage pattern of the loader is the following:

loader <secured binary> <public key>
Useful tools for this exercise are openssl dgstl and objcopy. Since the RaspberryPi image
does not contain the openssl or libelf header files, the tools need to be called from the program
and cannot use the APIs directly. Please note that for some operations the return code of the call is
important
An example secured binary and matching public key file is in test_material directory for
testing purposes. The binaries contain a SHA-256 signature, this needs to be considered this when writing
the loader.
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


int main()
{


	char *command_one = "objcopy --dump-section .sha=/home/pi/TI2726-C/stack_exploit/5_2/test_material/signature.txt --input /home/pi/TI2726-C/stack_exploit/5_2/test_material/test_signed";
	char *command_two = "objcopy --remove-section .sha test_signed test_signed_1"; 
	char *command_three = "openssl dgst -sha256 -verify public_key.pem -signature signature.txt test_signed_1"; 
/*
	char *command_one = "ls";
    char *command_two = "pwd";      
    char *command_three = "ls -l";
*/
	pid_t pid;
	pid = fork();

	char *arg_one[2];
	arg_one[0] = command_one;
	arg_one[1] = NULL;

	char *arg_two[2];
    arg_two[0] = command_two;
    arg_two[1] = NULL;

	char *arg_three[2];
    arg_three[0] = command_three;
    arg_three[1] = NULL;

	if(pid == -1)
	{
		perror("fork");
	}

	if(pid == 0)
	{
		//child process
		if(execvp(arg_one[0], arg_one) == -1)
		{
		    perror("exec");
		}
    }


	if(pid > 0)
    {
        //parent process
		if(wait(0) == -1)
		{
			perror("wait");
		}
        if(execvp(arg_two[0], arg_two) == -1)
        {
            perror("exec");
        }
    }


	return 0;
}







