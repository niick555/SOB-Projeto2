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

static void test_skcipher_cb(struct crypto_async_request *req, int error)
{
	struct tcrypt_result *result = req->data;

	if (error == -EINPROGRESS)
		return;
	result->err = error;
	complete(&result->completion);
	pr_info("Encryption finished successfully\n");
}

static void test_skcipher_encdec(struct skcipher_def *sk, int enc)
{
	if (enc == 1)
		rc = crypto_skcipher_encrypt(sk->req);
	else
		rc = crypto_skcipher_decrypt(sk->req);

	init_completion(&sk->result.completion);
}

static int test_skcipher(void)
{
	struct skcipher_def sk;
	struct crypto_skcipher *skcipher = NULL;
	struct skcipher_request *req = NULL;
	char *scratchpad = NULL;
	unsigned char key[32] = "AAAABBBBAAAABBBBAAAABBBBAAAABBBB";

	skcipher = crypto_alloc_skcipher("ebc-aes", 0, 0);
	if (IS_ERR(skcipher)) {
		pr_info("could not allocate skcipher handle\n");
		return 1;
	}

	req = skcipher_request_alloc(skcipher, GFP_KERNEL);
	if (!req) {
		pr_info("could not allocate skcipher request\n");
		return 1;
	}

	skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG, test_skcipher_cb, &sk.result);

	if (crypto_skcipher_setkey(skcipher, key, 32)) {
		pr_info("key could not be set\n");
		return 1;
	}
	
	scratchpad = kmalloc(16, GFP_KERNEL);
	if (!scratchpad) {
		pr_info("could not allocate scratchpad\n");
		return 1;
	}

	scratchpad = "OITUDOBEMCOMVOCE";

	sk.tfm = skcipher;
	sk.req = req;

	sg_init_one(&sk.sg, scratchpad, 16);
	skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, NULL);
	init_completion(&sk.result.completion);

	ret = test_skcipher_encdec(&sk, 1);
	if (ret)
		return 1;

	pr_info("Encryption triggered successfully\n");

	return ret;
}