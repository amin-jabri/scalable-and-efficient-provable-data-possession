

#include "sepdp.h"

int main(int argc, char **argv){
  
	SEPDP_challenge *challenge = NULL;
	SEPDP_proof *proof = NULL;
	int i = 0;
	int ret = 0;

  	if(!sepdp_setup_file(argv[1], strlen(argv[1]),  NULL, 0, 30)) printf("Error\n");

	for(i = 0; i < 5; i++){
		challenge = sepdp_challenge_file(argv[1], strlen(argv[1]),  i);
		if(!challenge){ printf("No challenge!\n"); return -1;}
		proof = sepdp_prove_file(argv[1], strlen(argv[1]),  NULL, 0, challenge);
		if(!proof){ printf("No proof!\n"); return -1;}
		ret = sepdp_verify_file(proof);
		if(ret == 0) printf("Verified!\n");
		else printf("Cheating!\n");
	}

  return 0;
  
}
