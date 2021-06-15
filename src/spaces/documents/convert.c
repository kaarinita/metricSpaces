
#include <stdio.h>

int main(int argc, char**argv)

{
  FILE *f,*g;
  int id; double w;

  f=fopen(argv[1],"r");
  g=fopen(argv[2],"wb+");

  while(fscanf(f,"%d %lg\n",&id,&w) == 2)
    { fwrite(&id,sizeof(id),1,g);
      fwrite(&w,sizeof(w),1,g);
    }
  fclose(f);fclose(g);
}

