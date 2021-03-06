#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;/* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#if defined(MYSQL_SERVER)
#include <m_string.h>/* To get strmov() */
#else
/* when compiled as standalone */
#include <string.h>
#define strmov(a,b) stpcpy(a,b)
#define bzero(a,b) memset(a,0,b)
#define memcpy_fixed(a,b,c) memcpy(a,b,c)
#endif
#endif
#include <mysql.h>
#include <ctype.h>

//#ifdef HAVE_DLOPEN

#if !defined(HAVE_GETHOSTBYADDR_R) || !defined(HAVE_SOLARIS_STYLE_GETHOST)
static pthread_mutex_t LOCK_hostname;
#endif



#include <math.h>
#include <gmp.h>
#include "paillier.h"
#define BASE 32

struct data_type{
	// store the data type that will be used in SUM_HE
	// include key, the original data and the results of product
	paillier_pubkey_t *pubkey;
//	int base_num;
	char *res;
};

// Summation on encrypted value
// constructor

my_bool SUM_HE_init(UDF_INIT *initid, UDF_ARGS *args, char *error){
	//checkers
	if(args->arg_count != 1){
		strcpy(error, "Need one argument");
		return 1;
	}
	if(args->arg_type[0] != STRING_RESULT)
	{
		strcpy(error, "Needs to have string value!");
		return 1;
	}
	printf("Allocating public key...");
	struct data_type* data = (struct data_type*)malloc(sizeof(struct data_type));
	paillier_ciphertext_t* paillier_create_enc_zero();

	// Give it an initial value.
	data->res = "1";
	// This is the pre-generated public key, must not change
	data->pubkey = paillier_pubkey_from_hex("8b7401f20812b8a58678f4c9f73f3b7f");
//	// Allocate a value to base number
//	data->base_num = atoi("8b7401f20812b8a58678f4c9f73f3b7f");

	// Another allocations
	initid->maybe_null = 1;
	args->maybe_null[0] = 1;
	initid->ptr = (char*)data;
	return 0;
}

void SUM_HE_deinit(UDF_INIT* initid){
	// Deconstructor
	struct data_type* data = (struct data_type*)initid->ptr;
	if (data->res != NULL) {
	    data->res = NULL;
	}
	paillier_freepubkey(data->pubkey);
	free((struct data_type*)initid->ptr);
}

void SUM_HE_clear(UDF_INIT *initid, char *is_null, char *error)
{
   struct data_type* data = (struct data_type*)initid->ptr;
   data->res = "1";
}

char * encry_sum (paillier_pubkey_t* public_key, char * input1, char * input2, int base ){
	// This is my SUM_add function, calling function
	// Using libpaillier library
	// http://acsc.cs.utexas.edu/libpaillier/

	paillier_ciphertext_t input_a;
	mpz_init(input_a.c);
	mpz_set_str(input_a.c, input1, base);

	paillier_ciphertext_t input_b;
	mpz_init(input_b.c);
	mpz_set_str(input_b.c, input2, base);

	paillier_ciphertext_t result;
	mpz_init(result.c);
	paillier_mul(public_key, &result, &input_a, &input_b);

	// dump result to output
	return mpz_get_str(NULL, base,result.c);
}

void SUM_HE_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error){
	// The function body of add, it will alter the 'data' object
	 struct data_type* data = (struct data_type*)initid->ptr;
	 char * output = encry_sum(data->pubkey, data->res, args->args[0], BASE );
	 data->res = output;
}

char *SUM_HE(UDF_INIT *initid, UDF_ARGS *args, char *output, unsigned long *length
		,char *empty_i, char *error){
	// This function will return the result of add
	struct data_type* data = (struct data_type*)initid->ptr;
	if (data->res == "1") {
	      *empty_i = 1;
	      *length = 0;
	      return NULL;
	   }
	*length = strlen(data->res);
	return data->res;
}




