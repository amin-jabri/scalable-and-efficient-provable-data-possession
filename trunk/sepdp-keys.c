
#include "sepdp.h"

SEPDP_key *sepdp_get_keys(){
  
	return generate_sepdp_key();
}

void destroy_sepdp_key(SEPDP_key *key){
  
	if(!key) return;
	if(key->W) sfree(key->W, SEPDP_PRF_KEY_SIZE);
	if(key->Z) sfree(key->Z, SEPDP_PRF_KEY_SIZE);
	if(key->K) sfree(key->K, SEPDP_AE_KEY_SIZE);
	key->W_size = 0;
	key->Z_size = 0;
	key->K_size = 0;
	sfree(key, sizeof(SEPDP_key));
  
	return;
}

SEPDP_key *generate_sepdp_key(){
  
	SEPDP_key *key = NULL;
  
	if( (key = malloc(sizeof(SEPDP_key))) == NULL) return NULL;
	memset(key, 0, sizeof(SEPDP_key));
  
	if( ((key->W = malloc(SEPDP_PRF_KEY_SIZE)) == NULL)) goto cleanup;
	if( ((key->Z = malloc(SEPDP_PRF_KEY_SIZE)) == NULL)) goto cleanup;
	if( ((key->K = malloc(SEPDP_AE_KEY_SIZE)) == NULL)) goto cleanup;	
  
	/* Generate symmetric keys */
/*
	if(!RAND_bytes(key->W, SEPDP_PRF_KEY_SIZE)) goto cleanup;
	if(!RAND_bytes(key->Z, SEPDP_PRF_KEY_SIZE)) goto cleanup;
	if(!RAND_bytes(key->K, SEPDP_AE_KEY_SIZE)) goto cleanup;
	*/
	memset(key->W, 'W', SEPDP_PRF_KEY_SIZE);
	memset(key->Z, 'Z', SEPDP_PRF_KEY_SIZE);
	memset(key->K, 'K', SEPDP_AE_KEY_SIZE);
	key->W_size = SEPDP_PRF_KEY_SIZE;
	key->Z_size = SEPDP_PRF_KEY_SIZE;
	key->K_size = SEPDP_AE_KEY_SIZE;

	return key;
  
 cleanup:
	if(key) destroy_sepdp_key(key);
	return NULL;
}
