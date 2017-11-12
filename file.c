/*
 *  linux/fs/minix/file.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  minix regular file handling primitives
 */

#include "minix.h"
#include <linux/fs.h>
#include <linux/uio.h>
#include <asm/uaccess.h>
#include <linux/crypto.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>

static ssize_t read_file(struct kiocb *iocb, struct iov_iter *from);
static ssize_t write_file(struct kiocb *iocb, struct iov_iter *iter);

static char *key;
module_param(key, charp, 0000);

#define SIZE_OF_DATA 32

static char data[SIZE_OF_DATA];
static u8 criptografado[SIZE_OF_DATA / 2], descriptografado[SIZE_OF_DATA];
static struct crypto_cipher *cryptoCipher = NULL;
static struct crypto_shash *cryptoSHash = NULL;

/*
 * We have mostly NULLs here: the current defaults are OK for
 * the minix filesystem.
 */
const struct file_operations minix_file_operations = {
	.llseek		= generic_file_llseek,
	.read_iter	= read_file,
	.write_iter	= write_file,
	.mmap		= generic_file_mmap,
	.fsync		= generic_file_fsync,
	.splice_read	= generic_file_splice_read,
};

static ssize_t read_file(struct kiocb *iocb, struct iov_iter *iter)
{
	ssize_t ret;
	char *ptr = (char *)iter->kvec->iov_base;
	int i;

	ret = generic_file_read_iter(iocb, iter);

    printk(KERN_INFO "cryptomodule: key secreta é %s\n", key);

	for(i = 0; i < (strlen(ptr) - 1); i++) {
		ptr[i] = 'W'; //Descriptografado
	}

	printk("minix: teste_read: %s\n", ptr);

	return ret;
}

static ssize_t write_file(struct kiocb *iocb, struct iov_iter *from)
{
	ssize_t ret;
	char *ptr = (char *)from->kvec->iov_base;
	int i;

	//Começa o crypto
	cryptoCipher = crypto_alloc_cipher("aes", 0, 0);

	if (IS_ERR(crypto_cipher_tfm(cryptoCipher))) {
		pr_info("cryptodevice: não foi possível alocar o handle para o cipher\n");
		return PTR_ERR(crypto_cipher_tfm(cryptoCipher));
	}

	if (crypto_cipher_setkey(cryptoCipher, key, 32 * sizeof(u8)) != 0) {
		pr_info("cryptodevice: não foi possível definir a key\n");
		return 1;
	}

	crypto_cipher_encrypt_one(cryptoCipher, criptografado, ptr);
	printk(KERN_INFO "cryptodevice: a operacao recebida é crypto, resultado: ");

	for(i = 0; i < sizeof(criptografado); i++) {
		printk(KERN_CONT "%02x", criptografado[i]);
	}

	printk(KERN_CONT "\n");

	for(i = 0; i < (strlen(ptr) - 1); i++) {
		ptr[i] = criptografado[i]; //Criptografado
	}

	ret = generic_file_write_iter(iocb, from);

	printk("minix: teste_write %s: \n", ptr);

	int j = 0;

    printk(KERN_INFO "cryptodevice: criptografado[]: ");
    for(i = 0; i < SIZE_OF_DATA; i++) {
       if(i % 2 == 0) {
          printk(KERN_CONT "%x", criptografado[j]);
          j++;
       }
    }

	crypto_cipher_decrypt_one(cryptoCipher, descriptografado, criptografado);
	printk(KERN_INFO "cryptodevice: a operacao recebida é decrypto, resultado: %s\n", descriptografado);

	crypto_free_cipher(cryptoCipher);

	return ret;
}

static int minix_setattr(struct dentry *dentry, struct iattr *attr)
{
	struct inode *inode = d_inode(dentry);
	int error;

	error = inode_change_ok(inode, attr);
	if (error)
		return error;

	if ((attr->ia_valid & ATTR_SIZE) &&
	    attr->ia_size != i_size_read(inode)) {
		error = inode_newsize_ok(inode, attr->ia_size);
		if (error)
			return error;

		truncate_setsize(inode, attr->ia_size);
		minix_truncate(inode);
	}

	setattr_copy(inode, attr);
	mark_inode_dirty(inode);
	return 0;
}

const struct inode_operations minix_file_inode_operations = {
	.setattr	= minix_setattr,
	.getattr	= minix_getattr,
};