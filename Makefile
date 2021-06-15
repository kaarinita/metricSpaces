
all:  all-strings all-vectors all-documents all-faces

all-strings:  bin/build-bkt-strings bin/query-bkt-strings \
    bin/build-sat-strings bin/query-sat-strings \
    bin/build-ght-strings bin/query-ght-strings \
    bin/build-lcluster-strings bin/query-lcluster-strings \
    bin/build-fqt-strings bin/query-fqt-strings \
    bin/build-mvp-strings bin/query-mvp-strings \
    bin/build-aesa-strings bin/query-aesa-strings \
    bin/build-iaesa-strings bin/query-iaesa-strings \
    bin/build-pivots-strings bin/query-pivots-strings \
    bin/build-fqh-strings bin/query-fqh-strings \
    bin/build-dyn-sat-strings bin/query-dyn-sat-strings
	
all-vectors:    bin/build-bkt-vectors bin/query-bkt-vectors \
    bin/build-sat-vectors bin/query-sat-vectors \
    bin/build-ght-vectors bin/query-ght-vectors \
    bin/build-lcluster-vectors bin/query-lcluster-vectors \
    bin/build-fqt-vectors bin/query-fqt-vectors \
    bin/build-mvp-vectors bin/query-mvp-vectors \
    bin/build-aesa-vectors bin/query-aesa-vectors \
    bin/build-iaesa-vectors bin/query-iaesa-vectors \
    bin/build-pivots-vectors bin/query-pivots-vectors \
    bin/build-dyn-sat-vectors bin/query-dyn-sat-vectors \
    bin/build-fqh-vectors bin/query-fqh-vectors
	
all-documents:    bin/build-bkt-documents bin/query-bkt-documents \
    bin/build-sat-documents bin/query-sat-documents \
    bin/build-ght-documents bin/query-ght-documents \
    bin/build-lcluster-documents bin/query-lcluster-documents \
    bin/build-fqt-documents bin/query-fqt-documents \
    bin/build-mvp-documents bin/query-mvp-documents \
    bin/build-aesa-documents bin/query-aesa-documents \
    bin/build-iaesa-documents bin/query-iaesa-documents \
    bin/build-pivots-documents bin/query-pivots-documents \
    bin/build-dyn-sat-documents bin/query-dyn-sat-documents \
    bin/build-fqh-documents bin/query-fqh-documents 

all-faces:    bin/build-bkt-faces bin/query-bkt-faces \
    bin/build-sat-faces bin/query-sat-faces \
    bin/build-ght-faces bin/query-ght-faces \
    bin/build-lcluster-faces bin/query-lcluster-faces \
    bin/build-fqt-faces bin/query-fqt-faces \
    bin/build-mvp-faces bin/query-mvp-faces \
    bin/build-aesa-faces bin/query-aesa-faces \
    bin/build-iaesa-faces bin/query-iaesa-faces \
    bin/build-pivots-faces bin/query-pivots-faces \
    bin/build-dyn-sat-faces bin/query-dyn-sat-faces \
    bin/build-fqh-faces bin/query-fqh-faces

all-dyn:  bin/query-dyn-dyn-sat-strings \
     bin/query-dyn-dyn-sat-vectors \
     bin/query-dyn-dyn-sat-documents \
     bin/query-dyn-dyn-sat-faces


clean: 
	rm -f bin/* lib/*.o lib/*/*.o lib/*/*/*.o
	
FLAGS = -O9 -Wall
CFLAGS = $(FLAGS) -DCONT -ggdb
DFLAGS = $(FLAGS) -DDISCR -ggdb

# Common files

lib/basicsD.o: src/basics.c
	 gcc $(DFLAGS) -c -o lib/basicsD.o  src/basics.c
lib/basicsC.o: src/basics.c
	 gcc $(CFLAGS) -c -o lib/basicsC.o  src/basics.c

lib/bucketD.o: src/bucket.c
	 gcc $(DFLAGS) -c -o lib/bucketD.o  src/bucket.c
lib/bucketC.o: src/bucket.c
	 gcc $(CFLAGS) -c -o lib/bucketC.o  src/bucket.c

lib/buildD.o: src/build.c
	 gcc $(DFLAGS) -c -o lib/buildD.o src/build.c
lib/buildC.o: src/build.c
	 gcc $(CFLAGS) -c -o lib/buildC.o src/build.c

lib/queryD.o: src/query.c
	 gcc $(DFLAGS) -c -o lib/queryD.o src/query.c
lib/queryC.o: src/query.c
	 gcc $(CFLAGS) -c -o lib/queryC.o src/query.c

lib/query-dynD.o: src/query-dyn.c
	 gcc $(DFLAGS) -c -o lib/query-dynD.o src/query-dyn.c
lib/query-dynC.o: src/query-dyn.c
	 gcc $(CFLAGS) -c -o lib/query-dynC.o src/query-dyn.c

# INDEXES

lib/indexD-bkt.o: src/indexes/bkt/bkt.c
	 gcc $(DFLAGS) -c -o lib/indexD-bkt.o src/indexes/bkt/bkt.c

lib/indexC-bkt.o: src/indexes/bkt/bkt.c
	 gcc $(CFLAGS) -c -o lib/indexC-bkt.o src/indexes/bkt/bkt.c

lib/indexD-sat.o: src/indexes/sat/sat.c
	 gcc $(DFLAGS) -c -o lib/indexD-sat.o src/indexes/sat/sat.c
lib/indexC-sat.o: src/indexes/sat/sat.c
	 gcc $(CFLAGS) -c -o lib/indexC-sat.o src/indexes/sat/sat.c

lib/indexD-ght.o: src/indexes/ght/ght.c
	 gcc $(DFLAGS) -c -o lib/indexD-ght.o src/indexes/ght/ght.c
lib/indexC-ght.o: src/indexes/ght/ght.c
	 gcc $(CFLAGS) -c -o lib/indexC-ght.o src/indexes/ght/ght.c

lib/indexD-lcluster.o: src/indexes/lcluster/lcluster.c
	 gcc $(DFLAGS) -c -o lib/indexD-lcluster.o src/indexes/lcluster/lcluster.c
lib/indexC-lcluster.o: src/indexes/lcluster/lcluster.c
	 gcc $(CFLAGS) -c -o lib/indexC-lcluster.o src/indexes/lcluster/lcluster.c

lib/indexD-fqt.o: src/indexes/fqt/fqt.c
	 gcc $(DFLAGS) -c -o lib/indexD-fqt.o src/indexes/fqt/fqt.c
lib/indexC-fqt.o: src/indexes/fqt/fqt.c
	 gcc $(CFLAGS) -c -o lib/indexC-fqt.o src/indexes/fqt/fqt.c

lib/indexD-mvp.o: src/indexes/mvp/mvp.c
	 gcc $(DFLAGS) -c -o lib/indexD-mvp.o src/indexes/mvp/mvp.c
lib/indexC-mvp.o: src/indexes/mvp/mvp.c
	 gcc $(CFLAGS) -c -o lib/indexC-mvp.o src/indexes/mvp/mvp.c

lib/indexD-aesa.o: src/indexes/aesa/aesa.c
	 gcc $(DFLAGS) -c -o lib/indexD-aesa.o src/indexes/aesa/aesa.c
lib/indexC-aesa.o: src/indexes/aesa/aesa.c
	 gcc $(CFLAGS) -c -o lib/indexC-aesa.o src/indexes/aesa/aesa.c

lib/indexD-iaesa.o: src/indexes/iaesa/iaesa.c
	 gcc $(DFLAGS) -c -o lib/indexD-iaesa.o src/indexes/iaesa/iaesa.c
lib/indexC-iaesa.o: src/indexes/iaesa/iaesa.c
	 gcc $(CFLAGS) -c -o lib/indexC-iaesa.o src/indexes/iaesa/iaesa.c

lib/indexD-pivots.o: src/indexes/pivots/pivots.c
	 gcc $(DFLAGS) -c -o lib/indexD-pivots.o src/indexes/pivots/pivots.c
lib/indexC-pivots.o: src/indexes/pivots/pivots.c
	 gcc $(CFLAGS) -c -o lib/indexC-pivots.o src/indexes/pivots/pivots.c

lib/indexD-fqh.o: src/indexes/fqh/fqh.c
	 gcc $(DFLAGS) -c -o lib/indexD-fqh.o src/indexes/fqh/fqh.c
lib/indexC-fqh.o: src/indexes/fqh/fqh.c
	 gcc $(CFLAGS) -c -o lib/indexC-fqh.o src/indexes/fqh/fqh.c

lib/indexD-dyn-sat.o: src/indexes/dynamic/sat/dyn-sat.c
	 gcc $(DFLAGS) -c -o lib/indexD-dyn-sat.o src/indexes/dynamic/sat/dyn-sat.c
lib/indexC-dyn-sat.o: src/indexes/dynamic/sat/dyn-sat.c
	 gcc $(CFLAGS) -c -o lib/indexC-dyn-sat.o src/indexes/dynamic/sat/dyn-sat.c

######

# SPACES

lib/space-strings.o: src/spaces/strings/objstrings.c
	 gcc $(DFLAGS) -c -o lib/space-strings.o src/spaces/strings/objstrings.c

lib/space-vectors.o: src/spaces/vectors/objvector.c
	 gcc $(CFLAGS) -c -o lib/space-vectors.o src/spaces/vectors/objvector.c

lib/space-documents.o: src/spaces/documents/objdocuments.c
	 gcc $(CFLAGS) -c -o lib/space-documents.o src/spaces/documents/objdocuments.c

lib/space-faces.o: src/spaces/faces/objfaces.c
	gcc $(CFLAGS) -c -o lib/space-faces.o src/spaces/faces/objfaces.c

# PROGRAMS

bin/build-bkt-strings: lib/basicsD.o lib/bucketD.o lib/indexD-bkt.o lib/space-strings.o lib/buildD.o 
	gcc $(DFLAGS) -o bin/build-bkt-strings lib/buildD.o  lib/basicsD.o lib/bucketD.o lib/indexD-bkt.o lib/space-strings.o -lm

bin/query-bkt-strings: lib/basicsD.o lib/bucketD.o lib/indexD-bkt.o lib/space-strings.o lib/queryD.o 
	gcc $(DFLAGS) -o bin/query-bkt-strings lib/queryD.o  lib/basicsD.o lib/bucketD.o lib/indexD-bkt.o lib/space-strings.o -lm

bin/build-sat-strings: lib/basicsD.o lib/bucketD.o lib/indexD-sat.o lib/space-strings.o lib/buildD.o 
	gcc $(DFLAGS) -o bin/build-sat-strings lib/buildD.o  lib/basicsD.o lib/bucketD.o lib/indexD-sat.o lib/space-strings.o -lm

bin/query-sat-strings: lib/basicsD.o lib/bucketD.o lib/indexD-sat.o lib/space-strings.o lib/queryD.o 
	gcc $(DFLAGS) -o bin/query-sat-strings lib/queryD.o  lib/basicsD.o lib/bucketD.o lib/indexD-sat.o lib/space-strings.o -lm

bin/build-ght-strings: lib/basicsD.o lib/bucketD.o lib/indexD-ght.o lib/space-strings.o lib/buildD.o 
	gcc $(DFLAGS) -o bin/build-ght-strings lib/buildD.o  lib/basicsD.o lib/bucketD.o lib/indexD-ght.o lib/space-strings.o -lm

bin/query-ght-strings: lib/basicsD.o lib/bucketD.o lib/indexD-ght.o lib/space-strings.o lib/queryD.o 
	gcc $(DFLAGS) -o bin/query-ght-strings lib/queryD.o  lib/basicsD.o lib/bucketD.o lib/indexD-ght.o lib/space-strings.o -lm

bin/build-lcluster-strings: lib/basicsD.o lib/bucketD.o lib/indexD-lcluster.o lib/space-strings.o lib/buildD.o 
	gcc $(DFLAGS) -o bin/build-lcluster-strings lib/buildD.o  lib/basicsD.o lib/bucketD.o lib/indexD-lcluster.o lib/space-strings.o -lm

bin/query-lcluster-strings: lib/basicsD.o lib/bucketD.o lib/indexD-lcluster.o lib/space-strings.o lib/queryD.o 
	gcc $(DFLAGS) -o bin/query-lcluster-strings lib/queryD.o  lib/basicsD.o lib/bucketD.o lib/indexD-lcluster.o lib/space-strings.o -lm

bin/build-fqt-strings: lib/basicsD.o lib/bucketD.o lib/indexD-fqt.o lib/space-strings.o lib/buildD.o 
	gcc $(DFLAGS) -o bin/build-fqt-strings lib/buildD.o  lib/basicsD.o lib/bucketD.o lib/indexD-fqt.o lib/space-strings.o -lm

bin/query-fqt-strings: lib/basicsD.o lib/bucketD.o lib/indexD-fqt.o lib/space-strings.o lib/queryD.o 
	gcc $(DFLAGS) -o bin/query-fqt-strings lib/queryD.o  lib/basicsD.o lib/bucketD.o lib/indexD-fqt.o lib/space-strings.o -lm

bin/build-mvp-strings: lib/basicsD.o lib/bucketD.o lib/indexD-mvp.o lib/space-strings.o lib/buildD.o 
	gcc $(DFLAGS) -o bin/build-mvp-strings lib/buildD.o  lib/basicsD.o lib/bucketD.o lib/indexD-mvp.o lib/space-strings.o -lm

bin/query-mvp-strings: lib/basicsD.o lib/bucketD.o lib/indexD-mvp.o lib/space-strings.o lib/queryD.o 
	gcc $(DFLAGS) -o bin/query-mvp-strings lib/queryD.o  lib/basicsD.o lib/bucketD.o lib/indexD-mvp.o lib/space-strings.o -lm

bin/build-aesa-strings: lib/basicsD.o lib/indexD-aesa.o lib/space-strings.o lib/buildD.o 
	gcc $(DFLAGS) -o bin/build-aesa-strings lib/buildD.o  lib/basicsD.o lib/indexD-aesa.o lib/space-strings.o -lm

bin/query-aesa-strings: lib/basicsD.o lib/indexD-aesa.o lib/space-strings.o lib/queryD.o 
	gcc $(DFLAGS) -o bin/query-aesa-strings lib/queryD.o  lib/basicsD.o lib/indexD-aesa.o lib/space-strings.o -lm

bin/build-iaesa-strings: lib/basicsD.o lib/indexD-iaesa.o lib/space-strings.o lib/buildD.o 
	gcc $(DFLAGS) -o bin/build-iaesa-strings lib/buildD.o  lib/basicsD.o lib/indexD-iaesa.o lib/space-strings.o -lm

bin/query-iaesa-strings: lib/basicsD.o lib/indexD-iaesa.o lib/space-strings.o lib/queryD.o 
	gcc $(DFLAGS) -o bin/query-iaesa-strings lib/queryD.o  lib/basicsD.o lib/indexD-iaesa.o lib/space-strings.o -lm

bin/build-pivots-strings: lib/basicsD.o lib/indexD-pivots.o lib/space-strings.o lib/buildD.o 
	gcc $(DFLAGS) -o bin/build-pivots-strings lib/buildD.o  lib/basicsD.o lib/indexD-pivots.o lib/space-strings.o -lm

bin/query-pivots-strings: lib/basicsD.o lib/indexD-pivots.o lib/space-strings.o lib/queryD.o 
	gcc $(DFLAGS) -o bin/query-pivots-strings lib/queryD.o  lib/basicsD.o lib/indexD-pivots.o lib/space-strings.o -lm

bin/build-fqh-strings: lib/basicsD.o lib/indexD-fqh.o lib/space-strings.o lib/buildD.o lib/bucketD.o
	gcc $(DFLAGS) -o bin/build-fqh-strings lib/buildD.o  lib/basicsD.o lib/indexD-fqh.o lib/space-strings.o lib/bucketD.o -lm

bin/query-fqh-strings: lib/basicsD.o lib/indexD-fqh.o lib/space-strings.o lib/queryD.o lib/bucketD.o
	gcc $(DFLAGS) -o bin/query-fqh-strings lib/queryD.o  lib/basicsD.o lib/indexD-fqh.o lib/space-strings.o lib/bucketD.o -lm

bin/build-dyn-sat-strings: lib/basicsD.o lib/indexD-dyn-sat.o lib/space-strings.o lib/buildD.o lib/bucketD.o
	gcc $(DFLAGS) -o bin/build-dyn-sat-strings lib/buildD.o  lib/basicsD.o lib/indexD-dyn-sat.o lib/space-strings.o lib/bucketD.o -lm

bin/query-dyn-sat-strings: lib/basicsD.o lib/indexD-dyn-sat.o lib/space-strings.o lib/queryD.o lib/bucketD.o
	gcc $(DFLAGS) -o bin/query-dyn-sat-strings lib/queryD.o  lib/basicsD.o lib/indexD-dyn-sat.o lib/space-strings.o lib/bucketD.o -lm

bin/query-dyn-dyn-sat-strings: lib/basicsD.o lib/indexD-dyn-sat.o lib/space-strings.o lib/query-dynD.o lib/bucketD.o
	gcc $(DFLAGS) -o bin/query-dyn-dyn-sat-strings lib/query-dynD.o  lib/basicsD.o lib/indexD-dyn-sat.o lib/space-strings.o lib/bucketD.o -lm


bin/build-bkt-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-vectors.o lib/buildC.o
	gcc $(CFLAGS) -o bin/build-bkt-vectors lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-vectors.o -lm

bin/query-bkt-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-vectors.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-bkt-vectors lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-vectors.o -lm

bin/build-sat-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-vectors.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-sat-vectors lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-vectors.o -lm

bin/query-sat-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-vectors.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-sat-vectors lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-vectors.o -lm

bin/build-ght-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-vectors.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-ght-vectors lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-vectors.o -lm

bin/query-ght-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-vectors.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-ght-vectors lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-vectors.o -lm

bin/build-lcluster-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-vectors.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-lcluster-vectors lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-vectors.o -lm

bin/query-lcluster-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-vectors.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-lcluster-vectors lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-vectors.o -lm

bin/build-fqt-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-vectors.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-fqt-vectors lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-vectors.o -lm

bin/query-fqt-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-vectors.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-fqt-vectors lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-vectors.o -lm

bin/build-mvp-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-vectors.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-mvp-vectors lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-vectors.o -lm

bin/query-mvp-vectors: lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-vectors.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-mvp-vectors lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-vectors.o -lm

bin/build-aesa-vectors: lib/basicsC.o lib/indexC-aesa.o lib/space-vectors.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-aesa-vectors lib/buildC.o  lib/basicsC.o lib/indexC-aesa.o lib/space-vectors.o -lm

bin/query-aesa-vectors: lib/basicsC.o lib/indexC-aesa.o lib/space-vectors.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-aesa-vectors lib/queryC.o  lib/basicsC.o lib/indexC-aesa.o lib/space-vectors.o -lm

bin/build-iaesa-vectors: lib/basicsC.o lib/indexC-iaesa.o lib/space-vectors.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-iaesa-vectors lib/buildC.o  lib/basicsC.o lib/indexC-iaesa.o lib/space-vectors.o -lm

bin/query-iaesa-vectors: lib/basicsC.o lib/indexC-iaesa.o lib/space-vectors.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-iaesa-vectors lib/queryC.o  lib/basicsC.o lib/indexC-iaesa.o lib/space-vectors.o -lm

bin/build-pivots-vectors: lib/basicsC.o lib/indexC-pivots.o lib/space-vectors.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-pivots-vectors lib/buildC.o  lib/basicsC.o lib/indexC-pivots.o lib/space-vectors.o -lm

bin/query-pivots-vectors: lib/basicsC.o lib/indexC-pivots.o lib/space-vectors.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-pivots-vectors lib/queryC.o  lib/basicsC.o lib/indexC-pivots.o lib/space-vectors.o -lm

bin/build-fqh-vectors: lib/basicsC.o lib/indexC-fqh.o lib/space-vectors.o lib/buildC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/build-fqh-vectors lib/buildC.o  lib/basicsC.o lib/indexC-fqh.o lib/space-vectors.o lib/bucketC.o -lm

bin/query-fqh-vectors: lib/basicsC.o lib/indexC-fqh.o lib/space-vectors.o lib/queryC.o lib/bucketD.o
	gcc $(CFLAGS) -o bin/query-fqh-vectors lib/queryC.o  lib/basicsC.o lib/indexC-fqh.o lib/space-vectors.o lib/bucketD.o -lm

bin/build-dyn-sat-vectors: lib/basicsC.o lib/indexC-dyn-sat.o lib/space-vectors.o lib/buildC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/build-dyn-sat-vectors lib/buildC.o  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-vectors.o lib/bucketC.o -lm

bin/query-dyn-sat-vectors: lib/basicsC.o lib/indexC-dyn-sat.o lib/space-vectors.o lib/queryC.o lib/bucketD.o
	gcc $(CFLAGS) -o bin/query-dyn-sat-vectors lib/queryC.o  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-vectors.o lib/bucketD.o -lm

bin/query-dyn-dyn-sat-vectors: lib/basicsC.o lib/indexC-dyn-sat.o lib/space-vectors.o lib/query-dynC.o lib/bucketD.o
	gcc $(CFLAGS) -o bin/query-dyn-dyn-sat-vectors lib/query-dynC.o  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-vectors.o lib/bucketD.o -lm


bin/build-bkt-documents: lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-documents.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-bkt-documents lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-documents.o -lm

bin/query-bkt-documents: lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-documents.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-bkt-documents lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-documents.o -lm

bin/build-sat-documents: lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-documents.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-sat-documents lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-documents.o -lm

bin/query-sat-documents: lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-documents.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-sat-documents lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-documents.o -lm

bin/build-ght-documents: lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-documents.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-ght-documents lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-documents.o -lm

bin/query-ght-documents: lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-documents.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-ght-documents lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-documents.o -lm

bin/build-lcluster-documents: lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-documents.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-lcluster-documents lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-documents.o -lm

bin/query-lcluster-documents: lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-documents.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-lcluster-documents lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-documents.o -lm

bin/build-fqt-documents: lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-documents.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-fqt-documents lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-documents.o -lm

bin/query-fqt-documents: lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-documents.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-fqt-documents lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-documents.o -lm

bin/build-mvp-documents: lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-documents.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-mvp-documents lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-documents.o -lm

bin/query-mvp-documents: lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-documents.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-mvp-documents lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-documents.o -lm

bin/build-aesa-documents: lib/basicsC.o lib/indexC-aesa.o lib/space-documents.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-aesa-documents lib/buildC.o  lib/basicsC.o lib/indexC-aesa.o lib/space-documents.o -lm

bin/query-aesa-documents: lib/basicsC.o lib/indexC-aesa.o lib/space-documents.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-aesa-documents lib/queryC.o  lib/basicsC.o lib/indexC-aesa.o lib/space-documents.o -lm

bin/build-iaesa-documents: lib/basicsC.o lib/indexC-iaesa.o lib/space-documents.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-iaesa-documents lib/buildC.o  lib/basicsC.o lib/indexC-iaesa.o lib/space-documents.o -lm

bin/query-iaesa-documents: lib/basicsC.o lib/indexC-iaesa.o lib/space-documents.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-iaesa-documents lib/queryC.o  lib/basicsC.o lib/indexC-iaesa.o lib/space-documents.o -lm

bin/build-pivots-documents: lib/basicsC.o lib/indexC-pivots.o lib/space-documents.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-pivots-documents lib/buildC.o  lib/basicsC.o lib/indexC-pivots.o lib/space-documents.o -lm

bin/query-pivots-documents:  lib/basicsC.o lib/indexC-pivots.o lib/space-documents.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-pivots-documents lib/queryC.o  lib/basicsC.o lib/indexC-pivots.o lib/space-documents.o -lm

bin/build-fqh-documents: lib/basicsC.o lib/indexC-fqh.o lib/space-documents.o lib/buildC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/build-fqh-documents lib/buildC.o  lib/basicsC.o lib/indexC-fqh.o lib/space-documents.o lib/bucketC.o -lm

bin/query-fqh-documents:  lib/basicsC.o lib/indexC-fqh.o lib/space-documents.o lib/queryC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/query-fqh-documents lib/queryC.o  lib/basicsC.o lib/indexC-fqh.o lib/space-documents.o lib/bucketC.o -lm

bin/build-dyn-sat-documents: lib/basicsC.o lib/indexC-dyn-sat.o lib/space-documents.o lib/buildC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/build-dyn-sat-documents lib/buildC.o  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-documents.o lib/bucketC.o -lm

bin/query-dyn-sat-documents:  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-documents.o lib/queryC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/query-dyn-sat-documents lib/queryC.o  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-documents.o lib/bucketC.o -lm

bin/query-dyn-dyn-sat-documents: lib/basicsC.o lib/indexC-dyn-sat.o lib/space-documents.o lib/query-dynC.o lib/bucketD.o
	gcc $(CFLAGS) -o bin/query-dyn-dyn-sat-documents lib/query-dynC.o  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-documents.o lib/bucketD.o -lm


bin/build-bkt-faces: lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-faces.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-bkt-faces lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-faces.o -lm

bin/query-bkt-faces: lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-faces.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-bkt-faces lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-bkt.o lib/space-faces.o -lm

bin/build-sat-faces: lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-faces.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-sat-faces lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-faces.o -lm

bin/query-sat-faces: lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-faces.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-sat-faces lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-sat.o lib/space-faces.o -lm

bin/build-ght-faces: lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-faces.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-ght-faces lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-faces.o -lm

bin/query-ght-faces: lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-faces.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-ght-faces lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-ght.o lib/space-faces.o -lm

bin/build-lcluster-faces: lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-faces.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-lcluster-faces lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-faces.o -lm

bin/query-lcluster-faces: lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-faces.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-lcluster-faces lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-lcluster.o lib/space-faces.o -lm

bin/build-fqt-faces: lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-faces.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-fqt-faces lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-faces.o -lm

bin/query-fqt-faces: lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-faces.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-fqt-faces lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-fqt.o lib/space-faces.o -lm

bin/build-mvp-faces: lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-faces.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-mvp-faces lib/buildC.o  lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-faces.o -lm

bin/query-mvp-faces: lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-faces.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-mvp-faces lib/queryC.o  lib/basicsC.o lib/bucketC.o lib/indexC-mvp.o lib/space-faces.o -lm

bin/build-aesa-faces: lib/basicsC.o lib/indexC-aesa.o lib/space-faces.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-aesa-faces lib/buildC.o  lib/basicsC.o lib/indexC-aesa.o lib/space-faces.o -lm

bin/query-aesa-faces: lib/basicsC.o lib/indexC-aesa.o lib/space-faces.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-aesa-faces lib/queryC.o  lib/basicsC.o lib/indexC-aesa.o lib/space-faces.o -lm

bin/build-iaesa-faces: lib/basicsC.o lib/indexC-iaesa.o lib/space-faces.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-iaesa-faces lib/buildC.o  lib/basicsC.o lib/indexC-iaesa.o lib/space-faces.o -lm

bin/query-iaesa-faces: lib/basicsC.o lib/indexC-iaesa.o lib/space-faces.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-iaesa-faces lib/queryC.o  lib/basicsC.o lib/indexC-iaesa.o lib/space-faces.o -lm

bin/build-pivots-faces: lib/basicsC.o lib/indexC-pivots.o lib/space-faces.o lib/buildC.o 
	gcc $(CFLAGS) -o bin/build-pivots-faces lib/buildC.o  lib/basicsC.o lib/indexC-pivots.o lib/space-faces.o -lm

bin/query-pivots-faces:  lib/basicsC.o lib/indexC-pivots.o lib/space-faces.o lib/queryC.o 
	gcc $(CFLAGS) -o bin/query-pivots-faces lib/queryC.o  lib/basicsC.o lib/indexC-pivots.o lib/space-faces.o -lm

bin/build-fqh-faces: lib/basicsC.o lib/indexC-fqh.o lib/space-faces.o lib/buildC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/build-fqh-faces lib/buildC.o  lib/basicsC.o lib/indexC-fqh.o lib/space-faces.o lib/bucketC.o -lm

bin/query-fqh-faces:  lib/basicsC.o lib/indexC-fqh.o lib/space-faces.o lib/queryC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/query-fqh-faces lib/queryC.o  lib/basicsC.o lib/indexC-fqh.o lib/space-faces.o lib/bucketC.o -lm

bin/build-dyn-sat-faces: lib/basicsC.o lib/indexC-dyn-sat.o lib/space-faces.o lib/buildC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/build-dyn-sat-faces lib/buildC.o  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-faces.o lib/bucketC.o -lm

bin/query-dyn-sat-faces:  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-faces.o lib/queryC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/query-dyn-sat-faces lib/queryC.o  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-faces.o lib/bucketC.o -lm

bin/query-dyn-dyn-sat-faces:  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-faces.o lib/query-dynC.o lib/bucketC.o
	gcc $(CFLAGS) -o bin/query-dyn-dyn-sat-faces lib/query-dynC.o  lib/basicsC.o lib/indexC-dyn-sat.o lib/space-faces.o lib/bucketC.o -lm
