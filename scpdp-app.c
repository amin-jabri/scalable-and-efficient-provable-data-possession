

#include "scpdp.h"

int main(int argc, char **argv){
  
  
  if(!scpdp_setup(argv[1], strlen(argv[1]),  NULL, 0, 30))
    printf("Error\n");
  else
    printf("Success\n");
  
  return 0;
  
}
