
#include "obj.h"

typedef void *Index;

/* creates and returns a structure for the first n elements of 
   database dbname (all if n=0). other construction parameters come 
   in argc and argv, which should be used and modified accordingly.
   this operation must open the database in this case the index is built
   inserting the objects one by one.
*/
Index build (char *dbname, int n, int *argc, char ***argv);

/* initializes and returns an empty structure (without objects). then is 
   possible to make insertions of objects */
Index init_Index  (char *dbname, int *argc, char ***argv);

/* frees the index; and closes the database if closedb */
void freeIndex (Index S, bool closedb);

/* stores it in fname (can be a file, or directory, or set of files).
   it must stode the database name, as it is not given for loading */
void saveIndex (Index S, char *fname);

/* reads an index from fname, and also opens the DB */
Index loadIndex (char *fname);

/* range search for query obj with radius r in index S. it returns
   how many objects it found. it prints them if show */
int search (Index S, Obj obj, Tdist r, bool show);

/* idem, but the k closest are sought and it returns the distance to
   the farthest */
Tdist searchNN (Index S, Obj obj, int k, bool show);

/* insert an object obj in the index S. it prints the object father where
   obj will be inserted as neighbor if show */
void insert (Index S, Obj obj, bool show );

/* delete an object obj, if it exist into the index S. it prints in which
   node was stored obj if show */ 
void delete (Index S, Obj obj, bool show);

