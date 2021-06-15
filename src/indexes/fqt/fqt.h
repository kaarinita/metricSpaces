#include "../../index.h"
#include "../../bucket.h"

typedef struct sfqnode
   { bool hoja;
     union
        { struct
             { void *bucket;
               int size;
             } hoja;
          struct
             { void *children;
             } interno;
        } u;
   } fqvpnode;

typedef struct
   { Tdist dist;  /* [dist,dist sgte) */
     fqvpnode child;
   } Tchild;

typedef struct
   { Obj query;
     Tdist dist;
   } query;

typedef struct
   { query *queries;
     int height;
     int bsize;
     int arity;
      int n;
     fqvpnode node;
     char *descr;
   } fqvpt;

#define child(node,i) (((Tchild*)((node)->u.interno.children))[i])

typedef struct
   { Obj obj;
     Tdist dist;
   } Tod;


