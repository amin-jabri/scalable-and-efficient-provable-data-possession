
/* 
* scpdp.h
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

#ifndef __SCPDP_H__
#define __SCPDP_H__

#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define SCPDP_PRF_KEY_SIZE 20 /* Size (in bytes) of an HMAC-SHA1 */
#define SCPDP_PRP_KEY_SIZE 16 /* Size (in bytes) of an AES key */
#define SCPDP_AE_KEY_SIZE 16  /* Size (in bytes) of the authenticated encryption key */

#define SCPDP_BLOCK_SIZE 4096

/* 512 blocks gives you 99.4% chance of detecting an error */
#define MAGIC_NUM_CHALLENGE_BLOCKS 512

typedef struct SCPDP_key_struct SCPDP_key;

struct SCPDP_key_struct{

	unsigned char *W;
	unsigned char *Z;
	unsigned char *K;
	
};

typedef struct SCPDP_challenge_struct SCPDP_challnenge;

struct SCPDP_challenge_struct{

	unsigned int i;
	unsigned char *ki;
	unsigned char *ci;

};

/* From scpdp.file.c */
int scpdp_setup_file(char *filepath, size_t filepath_len,  char *tokenfilepath, size_t tokenfilepath_len, unsigned int t);

/* From scpdp-key.c */
SCPDP_key *scpdp_get_keys();
SCPDP_key *generate_scpdp_key();
void destroy_scpdp_key(SCPDP_key *key);

/* From scpdp-misc.c */
void printhex(unsigned char *ptr, size_t size);

void sfree(void *ptr, size_t size);

unsigned char *generate_prf_f(unsigned char *W, unsigned int i, size_t *prf_result_size);

unsigned int *generate_prp_g(unsigned char *ki, size_t ki_size, unsigned int d, unsigned int r);

unsigned char *generate_H(unsigned char *c_i, size_t c_i_len, unsigned char **D, unsigned int r, size_t *vi_len);

#endif