
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h> 
#include <dirent.h>  
#include <math.h>

#define faleat (((unsigned)random())/((double)(unsigned)(1<<31)))
#define aleat(n) ((unsigned)((faleat)*(n)))

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

    while(dnt=readdir(dptr))
    { if (dnt->d_name[0] == '.') continue; // skip these files, incl . and ..
      size += strlen(dnt->d_name)+1;
      np++;
    }    

    rewinddir(dptr);

    DB.dnames = (char **)malloc(np*sizeof(char*));
    tbuf = (char*)malloc(size);

    while(dnt = readdir(dptr))
    { if (dnt->d_name[0] == '.') continue; // skip these files, incl . and ..
      strcpy(tbuf,dnt->d_name);
      DB.dnames[DB.n++] = tbuf;
      tbuf += strlen(tbuf)+1;
    }	
    closedir(dptr);
    return DB.n;
}

int main (int argc, char **argv)

  { int n,from,nq;
    char *kk;
    struct timeval t;

    if (argc < 5)
	{ fprintf (stderr,"Usage: genqueries <db dir> <from> <nqueries> <k>\n"
			    "  See the manual for details.\n");
	  exit(1);
	}
    n = openDB(argv[1]);
    from = atoi(argv[2]);
    nq = atoi(argv[3]);
    kk = argv[4];
   
    gettimeofday (&t,NULL);
    srandom (t.tv_sec*t.tv_usec);

    while (nq--)
      { printf ("%s,%i\n",kk,from+1+aleat(n-from));
      }
    printf("-0\n");
  }

