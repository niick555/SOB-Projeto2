#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <crypto/hash.h>
#define  DEVICE_NAME "cryptodevice"
#define  CLASS_NAME  "cryptodevice"

MODULE_LICENSE("GPL");

#define SIZE_OF_DATA 16

static int    majorNumber;
static char   data[SIZE_OF_DATA * 2];
static short  size_of_data = 0;
static struct class*  cryptoClass  = NULL;
static struct device* cryptoDevice = NULL;
static char   operation = 0;
static char   *key;
static u8     criptografado[SIZE_OF_DATA / 2], descriptografado[SIZE_OF_DATA];
static u8     hashOutput[SIZE_OF_DATA];

static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct sdesc *init_sdesc(struct crypto_shash *alg);
static int calc_hash(struct crypto_shash *alg, const unsigned char *data, unsigned int datalen, unsigned char *out);

module_param(key, charp, 0000);

static struct file_operations fops =
{
   .read = dev_read,
   .write = dev_write,
};

struct sdesc {
    struct shash_desc shash;
    char ctx[];
};

static int __init cryptodevice_init(void){
   printk(KERN_INFO "cryptodevice: inicializado com sucesso!\n");
   printk(KERN_INFO "cryptomodule: key secreta é %s\n", key);

   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "cryptodevice: falha ao criar o major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "cryptodevice: major number criado corretamente: %d\n", majorNumber);

   cryptoClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(cryptoClass)){
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "cryptodevice: falha ao criar a classe do device\n");
      return PTR_ERR(cryptoClass);
   }
   printk(KERN_INFO "cryptodevice: classe do device criada com sucesso!\n");

   cryptoDevice = device_create(cryptoClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(cryptoDevice)){
      class_destroy(cryptoClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "cryptodevice: falha ao criar o device\n");
      return PTR_ERR(cryptoDevice);
   }
   printk(KERN_INFO "cryptodevice: device criado com sucesso!\n");
   return 0;
}

static void __exit cryptodevice_exit(void){
   device_destroy(cryptoClass, MKDEV(majorNumber, 0));
   class_unregister(cryptoClass);
   class_destroy(cryptoClass);
   unregister_chrdev(majorNumber, DEVICE_NAME);
   printk(KERN_INFO "cryptodevice: finalizado com sucesso!\n");
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   char buf[256];

   buf[0] = operation;

   if(operation == 'c') {
      sprintf(&(buf[1]), "%s", criptografado);
      copy_to_user(buffer, buf, sizeof(buf));
   }
   else if(operation == 'd') {
      sprintf(&(buf[1]), "%s", descriptografado);
      copy_to_user(buffer, buf, sizeof(buf));
   }
   else if(operation == 'h') {
      sprintf(&(buf[1]), "%s", hashOutput);
      copy_to_user(buffer, buf, sizeof(buf));
   }

   return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
	struct crypto_cipher *cryptoCipher = NULL;
   struct crypto_shash *cryptoSHash = NULL;
   int i, j;
   char aux[SIZE_OF_DATA];

	cryptoCipher = crypto_alloc_cipher("aes", 0, 0);

	if (IS_ERR(crypto_cipher_tfm(cryptoCipher))) {
		pr_info("cryptodevice: não foi possível alocar o handle para o cipher\n");
		return PTR_ERR(crypto_cipher_tfm(cryptoCipher));
	}

   cryptoSHash = crypto_alloc_shash("sha1", 0, 0);

   if (crypto_cipher_setkey(cryptoCipher, key, 32 * sizeof(u8)) != 0) {
      pr_info("cryptodevice: não foi possível definir a key\n");
      return 1;
   }
   
   //sprintf(data, "%s", &(buffer[2]));
   strcpy(data, "AAAABBBBAAAABBBBCCCCDDDDCCCCDDDD");

   //operation = buffer[0];

   printk(KERN_INFO "cryptodevice: os dados recebidos são %s\n", data);

      for(j = 0; j < (strlen(data)/15); j++) {
         strncpy(aux, (j * 16) + &data[0], 15);
         aux[15] = '\0';
         printk("teste: %s - tam: %i\n", aux, strlen(aux));
         size_of_data = strlen(data);

         crypto_cipher_encrypt_one(cryptoCipher, criptografado, aux);

         for(i = 0; i < sizeof(criptografado); i++) {
            printk(KERN_CONT "%02x", criptografado[i]);
         }
      }
   	   

         // printk(KERN_CONT "\n");
   // }
   // else if(operation == 'd') {
   //    int j = 0;

   //    printk(KERN_INFO "cryptodevice: criptografado[]: ");
   //    for(i = 0; i < SIZE_OF_DATA; i++) {
   //       if(i % 2 == 0) {
   //          char aux[2];
   //          unsigned long res = 0;

   //          aux[0] = data[i];
   //          aux[1] = data[i + 1];

   //          kstrtol(aux, 16, &res);

   //          criptografado[j] = res;
   //          printk(KERN_CONT "%x", criptografado[j]);
   //          j++;
   //       }
   //    }

   	crypto_cipher_decrypt_one(cryptoCipher, descriptografado, criptografado);
   	printk(KERN_INFO "cryptodevice: o resultado: %s\n", descriptografado);
   // }
   // else if(operation == 'h') {
   //    char teste[32] = "AAAABBBBAAAABBBBAAAABBBBAAAABBBD";
   //    calc_hash(cryptoSHash, teste, 32, hashOutput);

   //    printk(KERN_INFO "cryptodevice: a operacao recebida é %c, resultado: ", operation);

   //    for(i = 0; i < sizeof(hashOutput); i++) {
   //       printk(KERN_CONT "%02x", hashOutput[i]);
   //    }
   // }
   
   crypto_free_cipher(cryptoCipher);
   crypto_free_shash(cryptoSHash);
   return len;
}

static struct sdesc *init_sdesc(struct crypto_shash *alg) {
    struct sdesc *sdesc;
    int size;

    size = sizeof(struct shash_desc) + crypto_shash_descsize(alg);
    sdesc = kmalloc(size, GFP_KERNEL);
    
    sdesc->shash.tfm = alg;
    sdesc->shash.flags = 0x0;

    return sdesc;
}

static int calc_hash(struct crypto_shash *alg, const unsigned char *data, unsigned int datalen, unsigned char *out) {
    struct sdesc *sdesc;
    int ret;

    sdesc = init_sdesc(alg);

    ret = crypto_shash_digest(&sdesc->shash, data, datalen, out);
    kfree(sdesc);
    return ret;
}

module_init(cryptodevice_init);
module_exit(cryptodevice_exit);