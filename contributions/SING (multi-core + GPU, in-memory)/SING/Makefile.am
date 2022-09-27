AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS =  ${ACLOCAL_FLAGS}

lib_LIBRARIES	= lib/libads.a
lib_libads_a_SOURCES = src/ads/isax_file_loaders.c src/ads/isax_first_buffer_layer.c src/ads/isax_index.c src/ads/isax_node.c src/ads/isax_node_buffer.c src/ads/isax_node_record.c src/ads/isax_node_split.c src/ads/isax_query_engine.c src/ads/isax_visualize_index.c src/ads/pqueue.c src/ads/sax/sax.c src/ads/sax/ts.c src/ads/inmemory_query_engine.c src/ads/parallel_inmemory_query_engine.c src/ads/inmemory_index_engine.c src/ads/parallel_index_engine.c src/ads/parallel_query_engine.c src/ads/inmemory_topk_engine.c src/ads/DTWfunction.c
lib_libads_a_CFLAGS =-I/opt/local/include -Iinclude/ -march=native -mavx -mavx2 -msse3 -fopenmp




bin_PROGRAMS	= bin/sing

bin_sing_SOURCES = src/utils/sing.cpp
bin_sing_CXXFLAGS = -I/opt/local/include -Iinclude/ -I/usr/local/include/ -ISING/ -I/usr/local/cuda/include -march=native -fopenmp
bin_sing_LDADD = -lreadline -lads -lm -lpthread -lcudart -lcuda -lsing  /usr/local/cuda/lib64/libcudart.so
bin_sing_LDFLAGS = -L/opt/local/lib -Llib/ -L/usr/local/cuda/lib64 -mavx -mavx2 -msse3 

