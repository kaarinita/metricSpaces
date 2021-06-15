
	/* manages buckets of elements, for usage at tree leaves */
	/* very rudimentary! */

typedef int *bucket;

	// creates empty
bucket createbucket (void);

	// adds obj to bucket B of current size bsize
bucket addbucket (bucket B, int bsize, Obj obj);

	// gives pos-th object in B
Obj elembuck (bucket B, int pos);

	// frees bucket of size bsize
void freebucket (bucket B, int bsize);

	// prints all objects in B (size bsize) at distance <= r from query obj
int searchbucket (bucket B, int bsize, Obj obj, Tdist r, bool show);

	// inserts into celem all objects of B (size bsize), together with 
	// their distances to obj
void searchbucketNN (bucket B, int bsize, Obj obj, Tcelem *res);

	// writes B (size bsize) to file f
void savebucket (bucket B, int bsize, FILE *f);

	// reads B (size bsize) from file f
bucket loadbucket (int bsize, FILE *f);

