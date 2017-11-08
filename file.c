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

static ssize_t read_file(struct kiocb *iocb, struct iov_iter *from);
static ssize_t write_file(struct kiocb *iocb, struct iov_iter *iter);

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

	ret = generic_file_read_iter(iocb, iter);

	printk("minix: teste_read: %i\n", ret);
	return ret;
}

static ssize_t write_file(struct kiocb *iocb, struct iov_iter *from)
{
	ssize_t ret;
	char *ptr = (char *)from->kvec->iov_base;

	ret = generic_file_write_iter(iocb, from);

	printk("minix: teste_write %s: \n", ptr);

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