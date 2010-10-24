/* 
* sepdp-file.c
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

#include "sepdp.h"

/* Create t tokens */
int sepdp_setup_file(char *filepath, size_t filepath_len, char *tokenfilepath, size_t tokenfilepath_len, unsigned int t){
	
	SEPDP_key *key = NULL;
	FILE *file = NULL;
	FILE *tokenfile = NULL;
	char realtokenfilepath[MAXPATHLEN];
	unsigned char *ki = NULL;
	unsigned char *ci = NULL;
	unsigned char **D = NULL;
	unsigned char *token_vi = NULL;
	unsigned int *indices = NULL;
	size_t ki_size = 0;
	size_t ci_size = 0;
	size_t token_vi_size = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int numfileblocks = 0;
	unsigned int r = 0;
	struct stat st;
	
	if(!filepath || !filepath_len || !t) return 0;
	
	file = fopen(filepath, "r");
	if(!file){
		fprintf(stderr, "ERROR: Was unable to open %s\n", filepath);
		return 0;
	}
	
	memset(realtokenfilepath, 0, MAXPATHLEN);
	/* If no token file path is specified, add a .tok extension to the filepath */
	if(!tokenfilepath && (filepath_len < MAXPATHLEN - 5)){
		if( snprintf(realtokenfilepath, MAXPATHLEN, "%s.tok", filepath) >= MAXPATHLEN) goto cleanup;
	}else{
		memcpy(realtokenfilepath, tokenfilepath, tokenfilepath_len);
	}
	
	tokenfile = fopen(realtokenfilepath, "w");
	if(!tokenfile) goto cleanup;

	/* Calculate the number sepdp blocks in the file */
	if(stat(filepath, &st) < 0) goto cleanup;
	numfileblocks = (st.st_size/SEPDP_BLOCK_SIZE);
	if(st.st_size%SEPDP_BLOCK_SIZE)
		numfileblocks++;

	/* Calculate the numbe of sepdp blocks to challenge */
	if(numfileblocks < MAGIC_NUM_CHALLENGE_BLOCKS)
		r = numfileblocks;
	else
		r = MAGIC_NUM_CHALLENGE_BLOCKS;

	/* Allocate room for the selected data blocks */
	if( ((D = malloc(r * sizeof(unsigned char *))) == NULL)) goto cleanup;
	memset(D, 0, r * sizeof(unsigned char *));
	for(i = 0; i < r; i++){
		if( ((D[i] = malloc(SEPDP_BLOCK_SIZE)) == NULL)) goto cleanup;
		memset(D[i], 0, SEPDP_BLOCK_SIZE);
	}

	/* Create a new PRF, PRP and AE keys */
	key = generate_sepdp_key();
	if(!key) goto cleanup;


	/* Generate each of the t tokens */
	for(i = 0; i < t; i++){
		/* Generate the psuedo-random permutation key k_i */
		ki = generate_prf_f(key->W, i, &ki_size);
		if(!ki) goto cleanup;
		/* Generate the pseudo-random challenge nonce c_i */
		ci = generate_prf_f(key->Z, i, &ci_size);
		if(!ci) goto cleanup;
		/* Generate the block indices for this token */
		indices = generate_prp_g(ki, ki_size, numfileblocks, r);
		if(!indices) goto cleanup;
		
		/* Seek to start of data file */
		if(fseek(file, 0, SEEK_SET) < 0) goto cleanup;
		for(j = 0; j < r; j++){
			/* Seek to data block at indices[j] */
			if(fseek(file, (SEPDP_BLOCK_SIZE * (indices[j])), SEEK_SET) < 0) goto cleanup;
			
			/* Read data block */
			fread(D[j], SEPDP_BLOCK_SIZE, 1, file);
			if(ferror(file)) goto cleanup;				
		}
		
		token_vi = generate_H(ci, ci_size, D, r, &token_vi_size);
		
		//TODO: encrypt and MAC the token.
		
		/* Write the token */
		fwrite(&i, sizeof(unsigned int), 1, tokenfile);
		if(ferror(tokenfile)) goto cleanup;
		fwrite(&token_vi_size, sizeof(size_t), 1, tokenfile);
		if(ferror(tokenfile)) goto cleanup;
		fwrite(token_vi, token_vi_size, 1, tokenfile);
		
		/* Cleanup */
		if(ki) sfree(ki, ki_size);
		if(ci) sfree(ci, ci_size);
		if(token_vi) sfree(token_vi, token_vi_size);
		for(j = 0; j < r; j++) memset(D[j], 0, SEPDP_BLOCK_SIZE);	
	}

	
	if(key) destroy_sepdp_key(key);
	if(file) fclose(file);
	if(tokenfile) fclose(tokenfile);
	if(indices) sfree(indices, sizeof(unsigned int) * r);
	if(D){
		for(i = 0; i < r; i++) sfree(D[i], SEPDP_BLOCK_SIZE);
		sfree(D, r * sizeof(unsigned char *));
	}

	return 1;

 cleanup:
	if(key) destroy_sepdp_key(key);
	if(file) fclose(file);
	if(tokenfile) fclose(tokenfile);
	if(indices) sfree(indices, sizeof(unsigned int) * r);
	if(ki) sfree(ki, ki_size);
	if(ci) sfree(ci, ci_size);
	if(token_vi) sfree(token_vi, token_vi_size);
	if(D){
		for(i = 0; i < r; i++) sfree(D[i], SEPDP_BLOCK_SIZE);
		sfree(D, r * sizeof(unsigned char *));
	}
	
	return 0;
	
}

/* Challenge the ith token */
SEPDP_challenge *sepdp_challenge_file(char *filepath, size_t filepath_len, unsigned int i){
	
	SEPDP_key *key = NULL;
	SEPDP_challenge *challenge = NULL;
	
	challenge = generate_sepdp_challenge();
	if(!challenge) goto cleanup;
	
	/* Get the PRF, PRP and AE keys */
	key = sepdp_get_keys();
	if(!key) goto cleanup;

	/* Generate the psuedo-random permutation key k_i */
	challenge->ki = generate_prf_f(key->W, i, &(challenge->ki_size));
	if(!challenge->ki) goto cleanup;
	/* Generate the pseudo-random challenge nonce c_i */
	challenge->ci = generate_prf_f(key->Z, i, &(challenge->ci_size));
	if(!challenge->ci) goto cleanup;
	challenge->i = i;

	return challenge;
	
 cleanup:
	if(challenge) destroy_sepdp_challenge(challenge);
	if(key) destroy_sepdp_key(key);

	return NULL;
}

SEPDP_proof *cpor_prove_file(char *filepath, size_t filepath_len, SEPDP_challenge *challenge){
	
	SEPDP_proof *proof = NULL;
	FILE *file = NULL;
	unsigned char **D = NULL;
	unsigned int numfileblocks = 0;
	unsigned int r = 0;
	unsigned int *indices = NULL;
	int i = 0, j = 0;
	struct stat st;
	
	if(!filepath || !filepath_len || !challenge) return NULL;
	
	proof = generate_sepdp_proof();
	if(!proof) goto cleanup;
	
	file = fopen(filepath, "r");
	if(!file){
		fprintf(stderr, "ERROR: Was unable to open %s\n", filepath);
		return NULL;
	}
	
	/* Calculate the number sepdp blocks in the file */
	if(stat(filepath, &st) < 0) goto cleanup;
	numfileblocks = (st.st_size/SEPDP_BLOCK_SIZE);
	if(st.st_size%SEPDP_BLOCK_SIZE)
		numfileblocks++;

	/* Calculate the numbe of sepdp blocks to challenge */
	if(numfileblocks < MAGIC_NUM_CHALLENGE_BLOCKS)
		r = numfileblocks;
	else
		r = MAGIC_NUM_CHALLENGE_BLOCKS;
	
	/* Allocate room for the selected data blocks */
	if( ((D = malloc(r * sizeof(unsigned char *))) == NULL)) goto cleanup;
	memset(D, 0, r * sizeof(unsigned char *));
	for(i = 0; i < r; i++){
		if( ((D[i] = malloc(SEPDP_BLOCK_SIZE)) == NULL)) goto cleanup;
		memset(D[i], 0, SEPDP_BLOCK_SIZE);
	}
	
	/* Generate the block indices for this token */
	indices = generate_prp_g(challenge->ki, challenge->ki_size, numfileblocks, r);
	if(!indices) goto cleanup;
		
	/* Seek to start of tag file */
	if(fseek(file, 0, SEEK_SET) < 0) goto cleanup;
	for(j = 0; j < r; j++){
		/* Seek to data block at indices[j] */
		if(fseek(file, (SEPDP_BLOCK_SIZE * (indices[j])), SEEK_SET) < 0) goto cleanup;
			
		/* Read data block */
		fread(D[j], SEPDP_BLOCK_SIZE, 1, file);
		if(ferror(file)) goto cleanup;				
	}
		
	proof->z = generate_H(challenge->ci, challenge->ci_size, D, r, &(proof->z_size));
	if(!proof->z) goto cleanup;
	
	if(file) fclose(file);
	if(indices) sfree(indices, sizeof(unsigned int) * r);
	if(D){
		for(i = 0; i < r; i++) sfree(D[i], SEPDP_BLOCK_SIZE);
		sfree(D, r * sizeof(unsigned char *));
	}
	
	return proof;
	
 cleanup:
	if(file) fclose(file);
	if(proof) destroy_sepdp_proof(proof);
	if(indices) sfree(indices, sizeof(unsigned int) * r);
	if(D){
		for(i = 0; i < r; i++) sfree(D[i], SEPDP_BLOCK_SIZE);
		sfree(D, r * sizeof(unsigned char *));
	}
	
	return NULL;
}

int CPOR_verify_file(char *filepath, size_t filepath_len, char *tokenfilepath, size_t tokenfilepath_len, SEPDP_challenge *challenge, SEPDP_proof *proof){

	return 0;
}