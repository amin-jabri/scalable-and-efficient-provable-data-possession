
/* 
* scpdp-misc.c
*
* Copyright (c) 2010, Zachary N J Peterson <znpeters@nps.edu>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Naval Postgraduate School nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY ZACHARY N J PETERSON ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL ZACHARY N J PETERSON BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include "scpdp.h"

void printhex(unsigned char *ptr, size_t size){

	int i = 0;
	for(i = 0; i < size; i++){
		printf("%02X", *ptr);
		ptr++;
	}
	printf("\n");
}

void sfree(void *ptr, size_t size){ memset(ptr, 0, size); free(ptr); ptr = NULL;}

/* generate_prf_f: an pseudo-random function (PRF) indexed on secret key W.
*  In this implementation, we use HMAC-SHA1
*/
unsigned char *generate_prf_f(unsigned char *key, unsigned int i, size_t *prf_result_size){

	unsigned char *prf_result = NULL;

	if(!key) return NULL;
	
	/* Allocate memory */
	if( ((prf_result = malloc(SHA_DIGEST_LENGTH)) == NULL)) goto cleanup;

	memset(prf_result, 0, SHA_DIGEST_LENGTH);

	/* Perform the HMAC on the index */
	if(!HMAC(EVP_sha1(), key, SCPDP_PRF_KEY_SIZE, (unsigned char *)&i, sizeof(unsigned int), 
		prf_result, (unsigned int *)prf_result_size)) goto cleanup;

	return prf_result;

cleanup:
	if(prf_result) sfree(prf_result, SHA_DIGEST_LENGTH);
	if(prf_result_size) *prf_result_size = 0;

	return NULL;	
}

/* key ki (generated by f_W() above), d blocks in the file, r indicies */
unsigned int *generate_prp_g(unsigned char *ki, size_t ki_size, unsigned int d, unsigned int r){

	unsigned char *prp_result = NULL;
	unsigned char *prp_input = NULL;
	AES_KEY aes_key;
	unsigned int index = 0;
	double result = 0.0;
	int x = 0, j = 0;
	unsigned int *indices = NULL;
	
	if(!ki || ki_size < SCPDP_PRP_KEY_SIZE) return NULL;

	/* Allocate memory */
	if( ((prp_result = malloc(SCPDP_PRP_KEY_SIZE)) == NULL)) goto cleanup;
	if( ((prp_input = malloc(SCPDP_PRP_KEY_SIZE)) == NULL)) goto cleanup;
	if( ((indices = malloc(r * sizeof(unsigned int))) == NULL)) goto cleanup;
	
	memset(prp_result, 0, SCPDP_PRP_KEY_SIZE);
	memset(prp_input, 0, SCPDP_PRP_KEY_SIZE);
	memset(&aes_key, 0, sizeof(AES_KEY));
	
	/* Setup the AES key */
	AES_set_encrypt_key(ki, SCPDP_PRP_KEY_SIZE * 8, &aes_key);
	
	/* Choose r blocks from 0 to d - 1 (file blocks) without replacement */
	for(x = 0; x < d && j < r; x++){
	
		/* Setup in the input buffer */
		memcpy(prp_input, &x, sizeof(int));
	
		/* Perform AES on the index */
		AES_encrypt(prp_input, prp_result, &aes_key);
	
		/* Turn the PRP result into a number */
		memcpy(&index, prp_result, sizeof(unsigned int));
	
		/* Make that number between 0 and 1 */
		result = ((double)index/(double)UINT_MAX);
	
		if( (d - x) * result < r - j){
			printf("X: %d\n", x);
			indices[j] = x;
			j++;
		}
	}
	
	/* Clear and free variables */
	if(prp_input) sfree(prp_input, SCPDP_PRP_KEY_SIZE);
	if(prp_result) sfree(prp_result, SCPDP_PRP_KEY_SIZE);
	memset(&aes_key, 0, sizeof(AES_KEY));
	
	return indices;

cleanup:
	if(prp_result) sfree(prp_result, SCPDP_PRP_KEY_SIZE);
	if(prp_input) sfree(prp_input, SCPDP_PRP_KEY_SIZE);
	if(indices) sfree(indices, (r * sizeof(unsigned int)));
	memset(&aes_key, 0, sizeof(AES_KEY));

	return NULL;
}

unsigned char *generate_H(unsigned char *c_i, size_t c_i_len, unsigned char **D, unsigned int r, size_t *vi_len){

	SHA_CTX ctx;
	unsigned char *vi = NULL; /* The token, aka hash result */
	int i = 0;

	if(!c_i || !D || !r) return NULL;
	
	if( ((vi = malloc(SHA_DIGEST_LENGTH)) == NULL)) goto cleanup;
	memset(vi, 0, SHA_DIGEST_LENGTH);
	
	if(!SHA1_Init(&ctx)) return NULL;
	
	/* Add c_i to the hash */
	if(!SHA1_Update(&ctx, c_i, c_i_len)) goto cleanup;
	
	for(i = 0; i < r; i++){
		if(!D[i]) goto cleanup;
		if(!SHA1_Update(&ctx, D[i], SCPDP_BLOCK_SIZE)) goto cleanup;
	}
	
	if(!SHA1_Final(vi, &ctx)) goto cleanup;
	
	*vi_len = SHA_DIGEST_LENGTH;
	return vi;
	
cleanup:
	if(vi) sfree(vi, SHA_DIGEST_LENGTH);
	if(vi_len) *vi_len = 0;
	
	return NULL;
}
