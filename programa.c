#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(){
   int ret, fd;
   char operacaoDados[BUFFER_LENGTH];
   fd = open("/dev/cryptodevice", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Erro ao abrir o device!");
      return errno;
   }
   printf("Entre com uma operação de criptografia (operacao dados):\n");
   scanf("%[^\n]%*c", operacaoDados);                // Read in a string (with spaces)
   ret = write(fd, operacaoDados, strlen(operacaoDados)); // Send the string to the LKM
   if (ret < 0){
      perror("Erro ao escrever no device!");
      return errno;
   }

   printf("Press ENTER to read back from the device...\n");
   getchar();

   ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }
   printf("The received message is: [%s]\n", receive);
   return 0;
}
