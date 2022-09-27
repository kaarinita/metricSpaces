AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS =  ${ACLOCAL_FLAGS}

lib_LIBRARIES	= lib/libads.a
lib_libads_a_SOURCES = src/ads/isax_file_loaders.c src/ads/isax_first_buffer_layer.c src/ads/isax_index.c src/ads/isax_node.c src/ads/isax_node_buffer.c src/ads/isax_node_record.c src/ads/isax_node_split.c src/ads/isax_query_engine.c src/ads/isax_visualize_index.c src/ads/pqueue.c src/ads/sax/sax.c src/ads/sax/ts.c src/ads/inmemory_query_engine.c src/ads/parallel_inmemory_query_engine.c src/ads/inmemory_index_engine.c src/ads/parallel_index_engine.c src/ads/parallel_query_engine.c src/ads/inmemory_topk_engine.c src/ads/DTWfunction.c
lib_libads_a_CFLAGS =-I/opt/local/include -Iinclude/ -march=native -mavx -mavx2 -msse3 -fopenmp -lm

bin_PROGRAMS	= bin/ads bin/MESSI bin/paris


bin_ads_SOURCES = src/utils/ads.c
bin_ads_CFLAGS = -I/opt/local/include -Iinclude/ -I/usr/local/include/ -march=native
bin_ads_LDADD = -lreadline -lpthread -lads -lm
bin_ads_LDFLAGS = -L/opt/local/lib -Llib/ -mavx -mavx2 -msse3 -fopenmp -march=native

bin_MESSI_SOURCES = src/utils/MESSI.c
bin_MESSI_CFLAGS = -I/opt/local/include -Iinclude/ -I/usr/local/include/ -march=native
bin_MESSI_LDADD = -lreadline -lpthread -lads -lm
bin_MESSI_LDFLAGS = -L/opt/local/lib -Llib/ -mavx -mavx2 -msse3 -fopenmp -march=native


bin_paris_SOURCES = src/utils/paris.c
bin_paris_CFLAGS = -I/opt/local/include -Iinclude/ -I/usr/local/include/ -march=native
bin_paris_LDADD = -lreadline -lpthread -lads -lm
bin_paris_LDFLAGS = -L/opt/local/lib -Llib/ -mavx -mavx2 -msse3 -fopenmp -march=native
