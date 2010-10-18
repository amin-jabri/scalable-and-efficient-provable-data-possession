
#include "scpdp.h"

SCPDP_key *scpdp_get_keys(){
  
  return generate_scpdp_key();
}

void destroy_scpdp_key(SCPDP_key *key){
  
  if(!key) return;
  if(key->W) sfree(key->W, SCPDP_PRF_KEY_SIZE);
  if(key->Z) sfree(key->Z, SCPDP_PRF_KEY_SIZE);
  if(key->K) sfree(key->K, SCPDP_AE_KEY_SIZE);
  sfree(key, sizeof(SCPDP_key));
  
  return;
}

SCPDP_key *generate_scpdp_key(){
  
  SCPDP_key *key = NULL;
  
  if( (key = malloc(sizeof(SCPDP_key))) == NULL) return NULL;
  memset(key, 0, sizeof(SCPDP_key));
  
  if( ((key->W = malloc(SCPDP_PRF_KEY_SIZE)) == NULL)) goto cleanup;
  if( ((key->Z = malloc(SCPDP_PRF_KEY_SIZE)) == NULL)) goto cleanup;
  if( ((key->K = malloc(SCPDP_AE_KEY_SIZE)) == NULL)) goto cleanup;	
  
  /* Generate symmetric keys */
  if(!RAND_bytes(key->W, SCPDP_PRF_KEY_SIZE)) goto cleanup;
  if(!RAND_bytes(key->Z, SCPDP_PRF_KEY_SIZE)) goto cleanup;
  if(!RAND_bytes(key->K, SCPDP_AE_KEY_SIZE)) goto cleanup;
  
  return key;
  
 cleanup:
  if(key) destroy_scpdp_key(key);
  return NULL;
}
