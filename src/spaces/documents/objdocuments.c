
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <dirent.h>  
#include <string.h>
#include <math.h>
#include "../../obj.h"

#define db(p) (DB.dnames[(uint)p])

typedef struct t_vocab
  { uint  id;
    double w;      
  } Tvocab;

typedef struct t_dDB
  { char *dirname;  /* directory name */
    char **dnames; /* pointers to file names all concatenated */
    uint n; /* number of files */
  } dDB;

static int never = 1;
static dDB DB;

double tfdf(char *fname1, char *fname2)
{
   double  sum,norm1,norm2;
   FILE *g;
   Tvocab *w1,*w2;      
   uint n1,n2,i,j;
   struct stat sdata;
   char Fname1[1024],Fname2[1024];

    sprintf(Fname1,"%s/%s",DB.dirname,fname1);
    sprintf(Fname2,"%s/%s",DB.dirname,fname2);
    stat (Fname1,&sdata);                                                       
    n1=sdata.st_size/(sizeof(Tvocab)); 
    stat (Fname2,&sdata);                                                       
    n2=sdata.st_size/(sizeof(Tvocab)); 

    w1=(Tvocab *)malloc(n1*sizeof(*w1));
    w2=(Tvocab *)malloc(n2*sizeof(*w2));    
    
   
   g= fopen (Fname1,"rb");
   if(!g)
      { fprintf(stderr,"Error: file %s not found\n",Fname1); exit(-1); }
   
   i=fread(w1,sizeof(*w1),n1,g); 
   fclose(g);
   
   g= fopen (Fname2,"rb");
   if(!g)
      { fprintf(stderr,"Error: file %s not found\n",Fname2); exit(-1); }

   j=fread(w2,sizeof(*w2),n2,g);  
   fclose(g);
   
   norm1=norm2=sum=0.0;
   for(i=0;i<n1;i++)norm1+=w1[i].w*w1[i].w;
   for(i=0;i<n2;i++)norm2+=w2[i].w*w2[i].w;
   for(i=0,j=0;(i<n1)&&(j<n2);)
      { if (w1[i].id==w2[j].id) // match
           { sum+=w1[i].w*w2[j].w;
             i++;j++;
           }
        else if (w1[i].id<w2[j].id) i++;
        else j++;
      }

   free(w1); free(w2);
// printf ("internal product: %f\n",sum/(sqrt(norm1)*sqrt(norm2)));
// printf ("distance: %f\n",acos(sum/(sqrt(norm1)*sqrt(norm2))));

   return acos(sum/(sqrt(norm1)*sqrt(norm2)));
}

float distanceInter (Obj o1, Obj o2)

   { return tfdf(db(o1),db(o2));
   }

int openDB (char *name)

{   DIR *dptr;
    struct dirent *dnt;
    uint size,np;
    char *tbuf;

    DB.dirname = malloc (strlen(name)+1);
    strcpy (DB.dirname,name);
    DB.n = 0;    
    
    if (!(dptr=opendir(name)))
       { fprintf (stderr,"Error: directory %s not found\n",name); exit(-1); }
    
    size = 0; np = 0;

    while((dnt=readdir(dptr)))
    { if (dnt->d_name[0] == '.') continue; // skip these files, incl . and ..
      size += strlen(dnt->d_name)+1;
      np++;
    }    

    rewinddir(dptr);

    DB.dnames = (char **)malloc(np*sizeof(char*));
    tbuf = (char*)malloc(size);

    while((dnt = readdir(dptr)))
    { if (dnt->d_name[0] == '.') continue; // skip these files, incl . and ..
      strcpy(tbuf,dnt->d_name);
      DB.dnames[DB.n++] = tbuf;
      tbuf += strlen(tbuf)+1;
    }	
    closedir(dptr);
    return DB.n;
}

void closeDB (void)

   { if (never) { never = 0; DB.dnames = NULL; }
     if (DB.dnames == NULL) return;
     free(DB.dirname);
     free(DB.dnames[0]);
     free(DB.dnames);
     DB.dnames = NULL;
   }

void printobj (Obj obj)

   { printf ("%s\n",DB.dnames[obj]);
   }

Obj parseobj (char *s)

   { int i;
     sscanf (s,"%i",&i);
     return i;
   }

