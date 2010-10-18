/* 
* scpdp-file.c
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

int scpdp_setup_file(char *filepath, size_t filepath_len,  char *tokenfilepath, size_t tokenfilepath_len, unsigned int t){
	
	SCPDP_key *key = NULL;
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

	/* Calculate the number scpdp blocks in the file */
	if(stat(filepath, &st) < 0) goto cleanup;
	numfileblocks = (st.st_size/SCPDP_BLOCK_SIZE);
	if(st.st_size%SCPDP_BLOCK_SIZE)
		numfileblocks++;

	/* Calculate the numbe of scpdp blocks to challenge */
	if(numfileblocks < MAGIC_NUM_CHALLENGE_BLOCKS)
		r = numfileblocks;
	else
		r = MAGIC_NUM_CHALLENGE_BLOCKS;

	/* Allocate room for the selected data blocks */
	if( ((D = malloc(r * sizeof(unsigned char *))) == NULL)) goto cleanup;
	memset(D, 0, r * sizeof(unsigned char *));
	for(i = 0; i < r; i++){
		if( ((D[i] = malloc(SCPDP_BLOCK_SIZE)) == NULL)) goto cleanup;
		memset(D[i], 0, SCPDP_BLOCK_SIZE);
	}

	/* Get the PRF, PRP and AE keys */
	key = scpdp_get_keys();
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
		
		/* Seek to start of tag file */
		if(fseek(file, 0, SEEK_SET) < 0) goto cleanup;
		for(j = 0; j < r; j++){
			/* Seek to data block at indices[j] */
			if(fseek(file, (SCPDP_BLOCK_SIZE * (indices[j])), SEEK_SET) < 0) goto cleanup;
			
			/* Read data block */
			fread(D[j], SCPDP_BLOCK_SIZE, 1, file);
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
		for(j = 0; j < r; j++) memset(D[j], 0, SCPDP_BLOCK_SIZE);	
	}

	
	if(file) fclose(file);
	if(tokenfile) fclose(tokenfile);
	if(D){
		for(i = 0; i < r; i++) sfree(D[i], SCPDP_BLOCK_SIZE);
		sfree(D, r * sizeof(unsigned char *));
	}

	return 1;

cleanup:
	if(file) fclose(file);
	if(tokenfile) fclose(tokenfile);
	if(ki) sfree(ki, ki_size);
	if(ci) sfree(ci, ci_size);
	if(token_vi) sfree(token_vi, token_vi_size);
	if(D){
		for(i = 0; i < r; i++) sfree(D[i], SCPDP_BLOCK_SIZE);
		sfree(D, r * sizeof(unsigned char *));
	}
	
	return 0;
	
}

/* Challenge the ith token */
SCPDP_challenge *scpdp_challenge_file(unsigned int i){
	
	SCPDP_key *key = NULL;
	SCPDP_challenge *challenge = NULL;
	
	challenge = generate_scpdp_challenge();
	
	/* Get the PRF, PRP and AE keys */
	key = scpdp_get_keys();
	if(!key) goto cleanup;

	/* Generate the psuedo-random permutation key k_i */
	ki = generate_prf_f(key->W, i, &ki_size);
	if(!ki) goto cleanup;
	/* Generate the pseudo-random challenge nonce c_i */
	ci = generate_prf_f(key->Z, i, &ci_size);


}