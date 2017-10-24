#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <linux/random.h>
#include <crypto/skcipher.h>
#define  DEVICE_NAME "cryptodevice"
#define  CLASS_NAME  "cryptodevice"

MODULE_LICENSE("GPL");

static int    majorNumber;
static char   data[32];
static short  size_of_data;
static struct class*  cryptoClass  = NULL;
static struct device* cryptoDevice = NULL;
static char   operation;
static char   *key;

static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

module_param(key, charp, 0000);

static struct file_operations fops =
{
	.write = dev_write,
};

struct tcrypt_result {
	struct completion completion;
	int err;
};

struct skcipher_def {
	struct scatterlist sg;
	struct crypto_skcipher *tfm;
	struct skcipher_request *req;
	struct tcrypt_result result;
};

static void test_skcipher_cb(struct crypto_async_request *req, int error);
static void test_skcipher_encdec(struct skcipher_def *sk, int enc);

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

static void test_skcipher_cb(struct crypto_async_request *req, int error)
{
	struct tcrypt_result *result = req->data;

	if (error == -EINPROGRESS)
		return;
	result->err = error;
	complete(&result->completion);
	printk(KERN_INFO "Encryption finished successfully\n");
}

static void test_skcipher_encdec(struct skcipher_def *sk, int enc)
{
	int rc = 0;

	if (enc == 1)
		rc = crypto_skcipher_encrypt(sk->req);
	else
		rc = crypto_skcipher_decrypt(sk->req);

	init_completion(&sk->result.completion);
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
	struct skcipher_def sk;
	struct crypto_skcipher *skcipher = NULL;
	struct skcipher_request *req = NULL;
	char *scratchpad = NULL;
	char *ivdata = NULL;
	unsigned char key[32];// = "AAAABBBBAAAABBBBAAAABBBBAAAABBBB";
	unsigned char *buf = NULL;
	struct scatterlist resultado;

	int i;

	skcipher = crypto_alloc_skcipher("cbc-aes-aesni", 0, 0);
	if (IS_ERR(skcipher)) {
		printk(KERN_INFO "could not allocate skcipher handle\n");
		return 1;
	}

	req = skcipher_request_alloc(skcipher, GFP_KERNEL);
	if (!req) {
		printk(KERN_INFO "could not allocate skcipher request\n");
		return 1;
	}

	skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG, test_skcipher_cb, &sk.result);

	//TEM QUE VERIFICAR SE A PESSOA EH SACANA!!!!!!!!!! URGENTE
	
	/* AES 256 with random key */
	get_random_bytes(&key, 32);
	if (crypto_skcipher_setkey(skcipher, key, 32)) {
		pr_info("key could not be set\n");
		return 1;
	}
	
	/* IV will be random */
	ivdata = kmalloc(16, GFP_KERNEL);
	if (!ivdata) {
		pr_info("could not allocate ivdata\n");
		return 1;
	}
	get_random_bytes(ivdata, 16);

	/* Input data will be random */
	scratchpad = kmalloc(16, GFP_KERNEL);
	if (!scratchpad) {
		pr_info("could not allocate scratchpad\n");
		return 1;
	}
	get_random_bytes(scratchpad, 16);

	//scratchpad = "OITUDOBEMCOMVOCE";

	sk.tfm = skcipher;
	sk.req = req;

	sg_init_one(&sk.sg, scratchpad, 16);
	skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, ivdata);
	init_completion(&sk.result.completion);

	test_skcipher_encdec(&sk, 1);

	buf = kmalloc(16, GFP_KERNEL);
	//sg_copy_to_buffer(&resultado, 1, &buf, 16);

	printk(KERN_INFO "Encryption triggered successfully");
	// for(i = 0; i < strlen(buf); i++) {
 //    	printk(KERN_CONT "%02x", buf[i]);
	// }

	//crypto_free_skcipher(skcipher);

	return 1;
}

module_init(cryptodevice_init);
module_exit(cryptodevice_exit);
