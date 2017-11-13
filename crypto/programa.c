#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 17
static char receive[BUFFER_LENGTH];

//MENSAGEM TESTE: "OITUDOBEMCOMVOCE"

int main() {
   int ret, fd, i;
   char operacaoDados[18];

   fd = open("/dev/cryptodevice", O_RDWR);

   if (fd < 0){
      perror("Erro ao abrir o device!");
      return errno;
   }

   printf("Entre com uma operação de criptografia (operacao dados)!\n");
   scanf("%[^\n]%*c", operacaoDados);

   ret = write(fd, operacaoDados, 18);
   if (ret < 0){
      perror("Erro ao escrever no device!");
      return errno;
   }

   printf("Pressione ENTER para finalizar.\n");
   getchar();

   ret = read(fd, receive, BUFFER_LENGTH);
   //Enviar na pos receive[1] o tamanho da msg
   if(receive[0] == 'c') {
      for(i = 1; i < BUFFER_LENGTH; i++) {
         printf("%02X", (unsigned int)(receive[i] & 0xFF));
      }
   }
   else if(receive[0] == 'd') {
      for(i = 1; i < BUFFER_LENGTH; i++) {
         printf("%c", receive[i]);
      }
   }
   else if(receive[0] == 'h') {
      for(i = 1; i < BUFFER_LENGTH; i++) {
         printf("%02X", (unsigned int)(receive[i] & 0xFF));
      }
   }

   if (ret < 0){
      perror("Erro ao receber do device!");
      return errno;
   }

   printf("\n");

   return 0;
}