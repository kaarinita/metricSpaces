#include "sing.h"
int main (int argc, char **argv)
{
    static char * dataset = "/home/botao/document/";
    static char * queries = "/home/botao/document/";
    static char * index_path = "/home/botao/document/myexperiment/";
    static char * labelset="/home/botao/document/myexperiment/";
    static long int dataset_size = 6000000;//testbench
    static int queries_size = 10;
    static int time_series_size = 256;
    static int paa_segments = 16;
    static int sax_cardinality = 8;
    static int leaf_size = 2000;
    static int min_leaf_size = 10;
    static int initial_lbl_size = 2000;
    static int flush_limit = 200000;
    static int initial_fbl_size = 100;
    static char use_index = 0;
    static int complete_type = 0;
    static int total_loaded_leaves = 1;
    static int tight_bound = 0;
    static int aggressive_check = 0;
    static float minimum_distance = FLT_MAX;
    static int serial_scan = 0;
    static char knnlabel = 0;
    static int min_checked_leaves = -1;
    static int cpu_control_type = 81;
    static char inmemory_flag=0;
    int calculate_thread=8;
    int  function_type =0;
    maxreadthread=5;
    read_block_length=20000;
    int k_size=0;
    long int labelsize=1;
    int topk=0;
    N_PQUEUE=1;
    while (1)
    {
        static struct option long_options[] =  {
            {"use-index", no_argument, 0, 'a'},
            {"initial-lbl-size", required_argument, 0, 'b'},
            {"complete-type", required_argument, 0, 'c'},
            {"dataset", required_argument, 0, 'd'},
            {"total-loaded-leaves", required_argument, 0, 'e'},
            {"flush-limit", required_argument, 0, 'f'},
            {"aggressive-check", no_argument, 0, 'g'},
            {"help", no_argument, 0, 'h'},
            {"initial-fbl-size", required_argument, 0, 'i'},
            {"serial", no_argument, 0, 'j'},
            {"queries-size", required_argument, 0, 'k'},
            {"leaf-size", required_argument, 0, 'l'},
            {"min-leaf-size", required_argument, 0, 'm'},
            {"tight-bound", no_argument, 0, 'n'},
            {"read-thread", required_argument, 0, 'o'},
            {"index-path", required_argument, 0, 'p'},
            {"queries", required_argument, 0, 'q'},
            {"read-block", required_argument, 0, 'r'},
            {"minimum-distance", required_argument, 0, 's'},
            {"timeseries-size", required_argument, 0, 't'},
            {"min-checked-leaves", required_argument, 0, 'u'},
            {"in-memory", no_argument, 0, 'v'},
            {"cpu-type", required_argument, 0, 'w'},
            {"sax-cardinality", required_argument, 0, 'x'},
            {"function-type", required_argument, 0, 'y'},
            {"dataset-size", required_argument, 0, 'z'},
            {"k-size", required_argument, 0, '0'},
            {"knn-label-set", required_argument, 0, '1'},
            {"knn-label-size", required_argument, 0, '2'},
            {"knn", no_argument, 0, '3'},
            {"topk", no_argument, 0, '4'},
            {"queue-number", required_argument, 0, '6'},
            {NULL, 0, NULL, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;
        int c = getopt_long (argc, argv, "",
                             long_options, &option_index);
        if (c == -1)
            break;
        switch (c)
        {
            case 'j':
                serial_scan = 1;
                break;
            case 'g':
                aggressive_check = 1;
                break;

            case 's':
                minimum_distance = atof(optarg);
                break;

            case 'n':
                tight_bound = 1;
                break;

            case 'e':
                total_loaded_leaves = atoi(optarg);
                break;

            case 'c':
                complete_type = atoi(optarg);
                break;
            case 'q':
                queries = optarg;
                break;

            case 'k':
                queries_size = atoi(optarg);
                break;

            case 'd':
                dataset = optarg;
                break;

            case 'p':
                index_path = optarg;
                break;

            case 'z':
                dataset_size = atoi(optarg);
                break;

            case 't':
                time_series_size = atoi(optarg);
                break;

            case 'x':
                sax_cardinality = atoi(optarg);
                break;

            case 'l':
                leaf_size = atoi(optarg);
                break;

            case 'm':
                min_leaf_size = atoi(optarg);
                break;

            case 'b':
                initial_lbl_size = atoi(optarg);
                break;

            case 'f':
                flush_limit = atoi(optarg);
                break;

            case 'u':
                min_checked_leaves = atoi(optarg);
                break;
            case 'w':
                cpu_control_type = atoi(optarg);
                break;
            case 'y':
                function_type = atoi(optarg);
                break;
            case 'i':
                initial_fbl_size = atoi(optarg);
                break;
            case 'o':
                maxreadthread = atoi(optarg);
                break;
            case 'r':
                read_block_length = atoi(optarg);
                break;
            case '0':
                k_size = atoi(optarg); 
                
                break;
            case '1':
                labelset = optarg;
                break;
            case '2':
                labelsize =  atoi(optarg);
            case '3':
                knnlabel=1;
            case '4':
                topk=1;
                break;
            case '6':
                N_PQUEUE =  atoi(optarg);
                break;
            case 'h':

                printf("Usage:\n\
                \t--dataset XX \t\t\tThe path to the dataset file\n\
                \t--queries XX \t\t\tThe path to the queries file\n\
                \t--dataset-size XX \t\tThe number of time series to load\n\
                \t--queries-size XX \t\tThe number of queries to do\n\
                \t--minimum-distance XX\t\tThe minimum distance we search (MAX if not set)\n\
                \t--use-index  \t\t\tSpecifies that an input index will be used\n\
                \t--index-path XX \t\tThe path of the output folder\n\
                \t--timeseries-size XX\t\tThe size of each time series\n\
                \t--sax-cardinality XX\t\tThe maximum sax cardinality in number of bits (power of two).\n\
                \t--leaf-size XX\t\t\tThe maximum size of each leaf\n\
                \t--min-leaf-size XX\t\tThe minimum size of each leaf\n\
                \t--initial-lbl-size XX\t\tThe initial lbl buffer size for each buffer.\n\
                \t--flush-limit XX\t\tThe limit of time series in memory at the same time\n\
                \t--initial-fbl-size XX\t\tThe initial fbl buffer size for each buffer.\n\
                \t--complete-type XX\t\t0 for no complete, 1 for serial, 2 for leaf\n\
                \t--total-loaded-leaves XX\tNumber of leaves to load at each fetch\n\
                \t--min-checked-leaves XX\t\tNumber of leaves to check at minimum\n\
                \t--tight-bound XX\tSet for tight bounds.\n\
                \t--aggressive-check XX\t\tSet for aggressive check\n\
                \t--serial\t\t\tSet for serial scan\n\
                \t--queue-number\t\tset the number of priority queues\n\
                \t--in-memory\t\t\tSet for in-memory search\n\
                \t--function-type\t\t\tSet for query answering type on disk\n\
                \t--k-size\t\t\tk size of knn\n\
                \t--knn-label-size\t\t\tlabel size of KNN\n\
                \t\t\tP+G: 0\n\
                \t\t\tM+G: 1\n\
                \t\t\tSING: 2\n\
                \t\t\tSING-knn: 3\n\
                \t--cpu-type\t\t\tSet for how many cores you want to used and in 1 or 2 cpu\n\
                \t--help\n\n\
                \tCPU type code:\t\t\t21 : 2 core in 1 CPU\n\
                \t\t\t\t\t22 : 2 core in 2 CPUs\n\
                \t\t\t\t\t41 : 4 core in 1 CPU\n\
                \t\t\t\t\t42 : 4 core in 2 CPUs\n\
                \t\t\t\t\t61 : 6 core in 1 CPU\n\
                \t\t\t\t\t62 : 6 core in 2 CPUs\n\
                \t\t\t\t\t81 : 8 core in 1 CPU\n\
                \t\t\t\t\t82 : 8 core in 2 CPUs\n\
                \t\t\t\t\t101: 10 core in 1 CPU\n\
                \t\t\t\t\t102: 10 core in 2 CPUs\n\
                \t\t\t\t\t121: 12 core in 1 CPU\n\
                \t\t\t\t\t122: 12 core in 2 CPUs\n\
                \t\t\t\t\t181: 18 core in 1 CPU\n\
                \t\t\t\t\t182: 18 core in 2 CPUs\n\
                \t\t\t\t\t242: 24 core in 2 CPUs\n\
                \t\t\t\t\tOther: 1 core in 1 CPU\n\
                ");
                return 0;
                break;
            case 'a':
                use_index = 1;
                break;
            case 'v':
                inmemory_flag = 1;
                break;
            default:
                exit(-1);
                break;
        }
    }
    INIT_STATS();
	cpu_set_t mask,get;
    CPU_ZERO(&mask);
    CPU_ZERO(&get);
    if(cpu_control_type==21)
    {
        CPU_SET(0, &mask);
        CPU_SET(2, &mask);
        calculate_thread=2;
        maxquerythread=2;
    }
    else if(cpu_control_type==22)
    {
        CPU_SET(0, &mask);
        CPU_SET(1, &mask);
        calculate_thread=2;
        maxquerythread=2;
    }
    else if(cpu_control_type==41)
    {
        CPU_SET(0, &mask);
        CPU_SET(2, &mask);
        CPU_SET(4, &mask);
        CPU_SET(6, &mask);
	    calculate_thread=4;
        maxquerythread=4;
    }
    else if(cpu_control_type==42)
    {
        CPU_SET(0, &mask);
        CPU_SET(1, &mask);
        CPU_SET(2, &mask);
        CPU_SET(3, &mask);
        calculate_thread=4;
        maxquerythread=4;
    }
    else if(cpu_control_type==61)
    {
        CPU_SET(0, &mask);
        CPU_SET(2, &mask);
        CPU_SET(4, &mask);
        CPU_SET(6, &mask);
        CPU_SET(8, &mask);
        CPU_SET(10, &mask);
        calculate_thread=6;
        maxquerythread=6;
    }
    else if(cpu_control_type==62)
    {
        CPU_SET(0, &mask);
        CPU_SET(1, &mask);
        CPU_SET(2, &mask);
        CPU_SET(3, &mask);
        CPU_SET(4, &mask);
        CPU_SET(5, &mask);
        calculate_thread=6;
        maxquerythread=6;
    }
    else if (cpu_control_type==81)
    {
        CPU_SET(0, &mask);
        CPU_SET(2, &mask);
        CPU_SET(4, &mask);
        CPU_SET(6, &mask);
        CPU_SET(8, &mask);
        CPU_SET(10, &mask);
        CPU_SET(12, &mask);
        CPU_SET(14, &mask);
        calculate_thread=8;
        maxquerythread=8;
    }
else if (cpu_control_type==161)
    {
        CPU_SET(0, &mask);
        CPU_SET(2, &mask);
        CPU_SET(4, &mask);
        CPU_SET(6, &mask);
        CPU_SET(8, &mask);
        CPU_SET(10, &mask);
        CPU_SET(12, &mask);
        CPU_SET(14, &mask);
        CPU_SET(16, &mask);
        CPU_SET(18, &mask);
        CPU_SET(20, &mask);
        CPU_SET(22, &mask);
        CPU_SET(24, &mask);
        CPU_SET(26, &mask);
        CPU_SET(28, &mask);
        CPU_SET(30, &mask);

        calculate_thread=16;
        maxquerythread=16;
    }
    else if (cpu_control_type==82)
    {
        CPU_SET(0, &mask);
        CPU_SET(1, &mask);
        CPU_SET(2, &mask);
        CPU_SET(3, &mask);
        CPU_SET(4, &mask);
        CPU_SET(5, &mask);
        CPU_SET(6, &mask);
        CPU_SET(7, &mask);
        calculate_thread=8;
        maxquerythread=8;
    }
    else if (cpu_control_type==162)
    {
        CPU_SET(0, &mask);
        CPU_SET(1, &mask);
        CPU_SET(2, &mask);
        CPU_SET(3, &mask);
        CPU_SET(4, &mask);
        CPU_SET(5, &mask);
        CPU_SET(6, &mask);
        CPU_SET(7, &mask);
        CPU_SET(8, &mask);
        CPU_SET(9, &mask);
        CPU_SET(10, &mask);
        CPU_SET(11, &mask);
        CPU_SET(12, &mask);
        CPU_SET(13, &mask);
        CPU_SET(14, &mask);
        CPU_SET(15, &mask);
        calculate_thread=16;
        maxquerythread=16;
    }
    else if (cpu_control_type==101)
    {
        CPU_SET(0, &mask);
        CPU_SET(2, &mask);
        CPU_SET(4, &mask);
        CPU_SET(6, &mask);
        CPU_SET(8, &mask);
        CPU_SET(10, &mask);
        CPU_SET(12, &mask);
        CPU_SET(14, &mask);
        CPU_SET(16, &mask);
        CPU_SET(18, &mask);
        calculate_thread=10;
        maxquerythread=10;
    }
    else if (cpu_control_type==102)
    {
        CPU_SET(0, &mask);
        CPU_SET(1, &mask);
        CPU_SET(2, &mask);
        CPU_SET(3, &mask);
        CPU_SET(4, &mask);
        CPU_SET(5, &mask);
        CPU_SET(6, &mask);
        CPU_SET(7, &mask);
        CPU_SET(8, &mask);
        CPU_SET(9, &mask);
        calculate_thread=10;
        maxquerythread=10;
    }
    else if (cpu_control_type==121)
    {
        CPU_SET(0, &mask);
        CPU_SET(2, &mask);
        CPU_SET(4, &mask);
        CPU_SET(6, &mask);
        CPU_SET(8, &mask);
        CPU_SET(10, &mask);
        CPU_SET(12, &mask);
        CPU_SET(14, &mask);
        CPU_SET(16, &mask);
        CPU_SET(18, &mask);
        CPU_SET(20, &mask);
        CPU_SET(22, &mask);
        calculate_thread=12;
        maxquerythread=12;
    }
    else if (cpu_control_type==122)
    {
        CPU_SET(0, &mask);
        CPU_SET(1, &mask);
        CPU_SET(2, &mask);
        CPU_SET(3, &mask);
        CPU_SET(4, &mask);
        CPU_SET(5, &mask);
        CPU_SET(6, &mask);
        CPU_SET(7, &mask);
        CPU_SET(8, &mask);
        CPU_SET(9, &mask);
        CPU_SET(10, &mask);
        CPU_SET(11, &mask);
        calculate_thread=12;
        maxquerythread=12;
    }
    else if (cpu_control_type==182)
    {
            CPU_SET(0, &mask);
            CPU_SET(1, &mask);
            CPU_SET(2, &mask);
            CPU_SET(3, &mask);
            CPU_SET(4, &mask);
            CPU_SET(5, &mask);
            CPU_SET(6, &mask);
            CPU_SET(7, &mask);
            CPU_SET(8, &mask);
            CPU_SET(9, &mask);
            CPU_SET(10, &mask);
            CPU_SET(11, &mask);
            CPU_SET(12, &mask);
            CPU_SET(13, &mask);
            CPU_SET(14, &mask);
            CPU_SET(15, &mask);
            CPU_SET(16, &mask);
            CPU_SET(17, &mask);
            calculate_thread=18;
            maxquerythread=18;
    }
    else if (cpu_control_type==242)
    {
            CPU_SET(0, &mask);
            CPU_SET(1, &mask);
            CPU_SET(2, &mask);
            CPU_SET(3, &mask);
            CPU_SET(4, &mask);
            CPU_SET(5, &mask);
            CPU_SET(6, &mask);
            CPU_SET(7, &mask);
            CPU_SET(8, &mask);
            CPU_SET(9, &mask);
            CPU_SET(10, &mask);
            CPU_SET(11, &mask);
            CPU_SET(12, &mask);
            CPU_SET(13, &mask);
            CPU_SET(14, &mask);
            CPU_SET(15, &mask);
            CPU_SET(16, &mask);
            CPU_SET(17, &mask);
            CPU_SET(18, &mask);
            CPU_SET(19, &mask);
            CPU_SET(20, &mask);
            CPU_SET(21, &mask);
            CPU_SET(22, &mask);
            CPU_SET(23, &mask);
            calculate_thread=24;
            maxquerythread=24;
    }
    else if(cpu_control_type==1)
    {

        calculate_thread=1;
        maxquerythread=1;
    }
    else
    {
        calculate_thread=cpu_control_type;
        maxquerythread=cpu_control_type;

        for (int i = 0; i < cpu_control_type; i++)
        {
            //CPU_SET(i, &mask);
        }
    }


    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) 
    {
        fprintf(stderr, "set thread affinity failed\n");
    }

    if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) 
    {
        fprintf(stderr, "get thread affinity failed\n");
    }

    isax_index_settings * index_settings = isax_index_settings_init(index_path,         // INDEX DIRECTORY
    	                                                                    time_series_size,   // TIME SERIES SIZE
    	                                                                    paa_segments,       // PAA SEGMENTS
    	                                                                    sax_cardinality,    // SAX CARDINALITY IN BITS
    	                                                                    leaf_size,          // LEAF SIZE
    	                                                                    min_leaf_size,      // MIN LEAF SIZE
    	                                                                    initial_lbl_size,   // INITIAL LEAF BUFFER SIZE
    	                                                                    flush_limit,        // FLUSH LIMIT
    	                                                                    initial_fbl_size,   // INITIAL FBL BUFFER SIZE
                                                                            total_loaded_leaves,// Leaves to load at each fetch
    																		tight_bound,		// Tightness of leaf bounds
																	aggressive_check,	// aggressive check
    													1,inmemory_flag);					// new index
    idx = isax_index_init_inmemory(index_settings);
    print_settings(idx->settings);
    idx->sax_cache_size=dataset_size;
    index_creation_gpu(dataset, dataset_size, idx);
    if(function_type==0)
    {
        //P+G
        isax_query_binary_file_PplusG(queries, queries_size, idx, minimum_distance, min_checked_leaves, &exact_search_serial_PplusG);
    }
    else if(function_type==1)
    {
        // M+G
        isax_query_binary_file_MplusG(queries, queries_size, idx, minimum_distance, min_checked_leaves, &exact_search_MplusG);
    }
    else if(function_type==2)
    {
        //sing with dynamic gap
        isax_query_binary_file_SING(queries, queries_size, idx, minimum_distance, min_checked_leaves, &exact_search_SING);
    }
    else if(function_type==3)
    {
        //sing knn with dynamic gap
        isax_knn_binary_file_SING(queries,labelset, queries_size, k_size, idx, minimum_distance, min_checked_leaves,labelsize, &exact_knn_SING);
    }

	PRINT_STATS(0.00f)
    free(rawfile);
    MESSI2_index_destroy(idx, NULL);
    return 0;
}


 void isax_query_binary_file_PplusG(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,bool*,bool*,float*)) 
{
	fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);
	FILE * ifile;
	ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    unsigned long long sz = (unsigned long long) ftell(ifile);
    unsigned long long total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

	int q_loaded = 0;
    ts_type * ts = (ts_type *)malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = (ts_type *)malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    bool *gposbitmap = NULL;
    bool *posbitmap = NULL;
    float *qts = NULL,*gqts = NULL;
    sax_type *gsaxarray=NULL;
    float *saxpoint=&sax_breakpoints[0];
	
	posbitmap=initialposbitmap(posbitmap,index->sax_cache_size);
	qts=(float*)malloc( sizeof(float)*index->settings->paa_segments); 
	initialdevice();
	gqts=initialgqts(gqts);
	gposbitmap=initialgposbitmap(gposbitmap,index->sax_cache_size);
	gsaxarray=initialgsaxarray(gsaxarray,index->sax_cache_size);
    gpumemcpy(gsaxarray,index->sax_cache,index->sax_cache_size);
	//printf("the isax cashe size is %ld\n",index->sax_cache_size);
    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
		
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
        query_result result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,qts,gqts,gsaxarray, posbitmap,gposbitmap,NULL);
        //COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        PRINT_STATS(result.distance)
        
        fflush(stdout);
    #if VERBOSE_LEVEL >= 1
        printf("[%p]: Distance: %lf\n", result.node, result.distance);
    #endif


        q_loaded++;
	}
    GPUfree((void*)gsaxarray);
    GPUfree((void*)gposbitmap);
    GPUfree((void*)gqts);

    cudaFreeHost(posbitmap);
    free(paa);
    free(ts);
	fclose(ifile);
	fprintf(stderr, ">>> Finished querying.\n");

}


 void isax_query_binary_file_MplusG(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,float*,float*,float*,node_list*)) 
{
	fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);
	FILE * ifile;
	ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    unsigned long long sz = (unsigned long long) ftell(ifile);
    unsigned long long total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

	int q_loaded = 0;
    ts_type * ts = (ts_type *)malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = (ts_type *)malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    float *gposbitmap = NULL;
    float *posbitmap = NULL;
    float *gqts = NULL;
    float *qts=NULL;
    sax_type *saxarray=NULL,*gsaxarray=NULL;
    float *saxpoint=&sax_breakpoints[0];
	node_list nodelist;
    nodelist.nlist=(isax_node**)malloc(sizeof(isax_node*)*index->settings->root_nodes_size);
    nodelist.node_amount=0;
    isax_node *current_root_node = index->first_node;
    //isax_node *current_root_node = index->first_node;
    for (int i = 0; i < index->settings->root_nodes_size; i++)
    {
        fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];
        if(current_buffer->initialized==1)
        {
            //offsetarray[nodelist.node_amount]=currentpositon;
            nodelist.nlist[nodelist.node_amount]=current_buffer->node;
            nodelist.node_amount++;
            //currentpositon=currentpositon+current_buffer->max_buffer_size;
        }
    }


	saxarray=initialsaxarray(gsaxarray,index->sax_cache_size);
	posbitmap=initialposbitmapfloat(posbitmap,index->sax_cache_size);
	initialdevice();
	gqts=initialgqts(gqts);
    qts=(float*)malloc( sizeof(float)*index->settings->paa_segments); 

	//gdictionary=initialgdictionary(gdictionary);
	gposbitmap=initialgposbitmapfloat(gposbitmap,index->sax_cache_size);
	gsaxarray=initialgsaxarray(gsaxarray,index->sax_cache_size);
    gpumemcpy(gsaxarray,index->sax_cache,index->sax_cache_size);
		//printf("the isax cashe size is %ld\n",index->sax_cache_size);
    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
		
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
                    memcpy(qts,paa,sizeof(float)*index->settings->paa_segments);
        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
        query_result result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,qts,gqts,gsaxarray, posbitmap,gposbitmap,NULL,&nodelist);
        //COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        PRINT_STATS(result.distance)
        
        fflush(stdout);
    #if VERBOSE_LEVEL >= 1
        printf("[%p]: Distance: %lf\n", result.node, result.distance);
    #endif
        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}

        q_loaded++;
	}
    GPUfree((void*)gsaxarray);
    GPUfree((void*)gposbitmap);
    GPUfree((void*)gqts);
    cudaFreeHost(posbitmap);
    free(qts);
    free(paa);
    free(ts);
	fclose(ifile);
	fprintf(stderr, ">>> Finished querying.\n");

}

 void isax_query_binary_file_SING(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,float*,float*,float*,node_list*,unsigned long int*)) 
{
	fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);
	FILE * ifile;
	ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    unsigned long long sz = (unsigned long long) ftell(ifile);
    unsigned long long total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
	int q_loaded = 0;
    ts_type * ts = (ts_type *)malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = (ts_type *)malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    float *gposbitmap = NULL;
    float *posbitmap = NULL;
    float *qts = NULL,*gqts = NULL;
    sax_type *saxarray=NULL,*gsaxarray=NULL;
    unsigned long int currentpositon=0;
    unsigned long int *offsetarray= (unsigned long int*)malloc(sizeof(unsigned long int)*(index->settings->root_nodes_size+1));
	sax_type *saxarrarysort=(sax_type*)malloc(sizeof(sax_type)*index->sax_cache_size*index->settings->paa_segments);

	node_list nodelist;
    nodelist.nlist=(isax_node**)malloc(sizeof(isax_node*)*pow(2, index->settings->paa_segments));
    nodelist.node_amount=0;
    //isax_node *current_root_node = index->first_node;
    //isax_node *current_root_node = index->first_node;
    for (int i = 0; i < index->settings->root_nodes_size; i++)
    {

        fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];

        if(current_buffer->initialized==1)
        {
            //offsetarray[nodelist.node_amount]=currentpositon;
            nodelist.nlist[nodelist.node_amount]=current_buffer->node;
            nodelist.node_amount++;
            //currentpositon=currentpositon+current_buffer->max_buffer_size;

        }

    }

	posbitmap=initialposbitmapfloat(posbitmap,index->sax_cache_size);//(float *)malloc(sizeof(float) * index->sax_cache_size);
	qts=(float*)malloc( sizeof(float)*index->settings->paa_segments); 
	initialdevice();
	gqts=initialgqts(gqts);
	gposbitmap=initialgposbitmapfloat(gposbitmap,index->sax_cache_size);
	gsaxarray=initialgsaxarray(gsaxarray,index->sax_cache_size);
		//printf("the isax cashe size is %ld\n",index->sax_cache_size);

unsigned long int currentposition=0;
    for(int i =0;i<nodelist.node_amount;i++)
	{
        offsetarray[i]=currentposition;
        pass_tree_node_m(nodelist.nlist[i],index,NULL,&currentposition,index->sax_cache,saxarrarysort,posbitmap);
        
    }
    offsetarray[nodelist.node_amount]=index->sax_cache_size;

    gpumemcpy(gsaxarray,saxarrarysort,index->sax_cache_size);

    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
		
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
                    memcpy(qts,paa,sizeof(float)*index->settings->paa_segments);
        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
        query_result result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,qts,gqts,gsaxarray, posbitmap,gposbitmap,NULL,&nodelist,offsetarray);
        //COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        PRINT_STATS(result.distance)
        
        fflush(stdout);
    #if VERBOSE_LEVEL >= 1
        printf("[%p]: Distance: %lf\n", result.node, result.distance);
    #endif
        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}

        q_loaded++;
	}
    GPUfree((void*)gsaxarray);
    GPUfree((void*)gposbitmap);
    GPUfree((void*)gqts);
    free(qts);
    cudaFreeHost(posbitmap);
    //free(posbitmap);
    free(saxarrarysort);
    free(paa);
    free(ts);
	fclose(ifile);

	fprintf(stderr, ">>> Finished querying.\n");

}


 void isax_knn_binary_file_SING(const char *ifilename ,const char *labelfilename, int q_num, int k,isax_index *index,
                            float minimum_distance, int min_checked_leaves, long int labelsize,
                            pqueue_bsf (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,float*,float*,float*,node_list*,unsigned long int*,int)) 
{
    fprintf(stderr, ">>> Performing queries in file: %s and label in file %s\n", ifilename,labelfilename);

    FILE *ifile ,*lfile ;
    ifile = fopen (ifilename,"rb");
    lfile = fopen (labelfilename,"rb");
    long int *datalabel;
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    if (lfile == NULL) {
        fprintf(stderr, "File %s not found!\n", labelfilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    datalabel=(long int*)malloc(sizeof(long int) * index->sax_cache_size);
    fread(datalabel, sizeof(long int),index->sax_cache_size,lfile);

	int q_loaded = 0;
    ts_type * ts = (ts_type *)malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = (ts_type *)malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    float *gposbitmap = NULL;
    float *posbitmap = NULL;
    float *qts = NULL,*gqts = NULL;
    sax_type *saxarray=NULL,*gsaxarray=NULL;
    unsigned long int currentpositon=0;
    unsigned long int *offsetarray= (unsigned long int*)malloc(sizeof(unsigned long int)*(index->settings->root_nodes_size+1));
	sax_type *saxarrarysort=(sax_type*)malloc(sizeof(sax_type)*index->sax_cache_size*index->settings->paa_segments);

	node_list nodelist;
    nodelist.nlist=(isax_node**)malloc(sizeof(isax_node*)*pow(2, index->settings->paa_segments));
    nodelist.node_amount=0;
    //isax_node *current_root_node = index->first_node;
    //isax_node *current_root_node = index->first_node;
    for (int i = 0; i < index->settings->root_nodes_size; i++)
    {

        fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];

        if(current_buffer->initialized==1)
        {
            //offsetarray[nodelist.node_amount]=currentpositon;
            nodelist.nlist[nodelist.node_amount]=current_buffer->node;
            nodelist.node_amount++;
            //currentpositon=currentpositon+current_buffer->max_buffer_size;
        }
    }



	saxarray=initialsaxarray(gsaxarray,index->sax_cache_size);
	posbitmap=initialposbitmapfloat(posbitmap,index->sax_cache_size);
	qts=(float*)malloc( sizeof(float)*index->settings->paa_segments); 
	initialdevice();
	gqts=initialgqts(gqts);
	gposbitmap=initialgposbitmapfloat(gposbitmap,index->sax_cache_size);
	gsaxarray=initialgsaxarray(gsaxarray,index->sax_cache_size);
		//printf("the isax cashe size is %ld\n",index->sax_cache_size);

    unsigned long int currentposition=0;
    for(int i =0;i<nodelist.node_amount;i++)
	{
        offsetarray[i]=currentposition;
        pass_tree_node_m(nodelist.nlist[i],index,NULL,&currentposition,index->sax_cache,saxarrarysort,posbitmap);
        
    }
    offsetarray[nodelist.node_amount]=index->sax_cache_size;

    gpumemcpy(gsaxarray,saxarrarysort,index->sax_cache_size);

    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
		
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
                    memcpy(qts,paa,sizeof(float)*index->settings->paa_segments);
        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
        pqueue_bsf result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,qts,gqts,gsaxarray, posbitmap,gposbitmap,NULL,&nodelist,offsetarray,k);
         long int *classcounter=(long int *)malloc(sizeof(long int) * labelsize);
        long int classeposition=0,classtmp=0;
        for (int i = 0; i < labelsize; i++)
        {
            classcounter[i]=0;
        }
        for (int i = 0; i < result.k; i++)
        {
            //printf("datalabel[result.position[i]] is %ld\n",datalabel[result.position[i]] );
            classcounter[datalabel[result.position[i]]]++;
            //printf("the class conter is %ld\n",classcounter[datalabel[result.position[i]]] );
            if(classtmp<classcounter[datalabel[result.position[i]]])
            {
                classtmp=classcounter[datalabel[result.position[i]]];
                classeposition=datalabel[result.position[i]];
 
            }
            //printf(" the [%d] query [%d] NN is %f at %ld label is %ld and now classeposition is %ld\n",q_loaded,i,result.knn[i],result.position[i],datalabel[result.position[i]],classeposition);
        }
        printf(" the [%d] query's label is %ld \n",q_loaded,classeposition);


        q_loaded++;
	}    
    GPUfree((void*)gsaxarray);
    GPUfree((void*)gposbitmap);
    GPUfree((void*)gqts);
    cudaFreeHost(posbitmap);

    free(qts);
    free(paa);
    free(ts);
	fclose(ifile);
	fprintf(stderr, ">>> Finished querying.\n");

}



query_result exact_search_serial_ParGIS(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,bool *positionmap,bool *gpositionmap,float *gdictionary) 
{

    RESET_BYTES_ACCESSED
    LBDcalculationnumber=0;
    RDcalculationnumber=0;




    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
   // bool *rdcbitmap=(bool*)malloc(sizeof(bool) * index->sax_cache_size);
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    ts_type *ts_buffer = (ts_type*)malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);


    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=approximate_result.distance;
    COUNT_CAL_TIME_START
    //LBDcalculationnumber=index->sax_cache_size;
    LBDstreamGPU(gsaxarray, positionmap, paa, gqts, bsf_distance,index->sax_cache_size,gpositionmap,index->settings->paa_segments,index->settings->mindist_sqrt);
        COUNT_CAL_TIME_END
	COUNT_QUEUE_TIME_START
    #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf_distance)
    for(unsigned long j=0; j<index->sax_cache_size; j++) {
        if(positionmap[j]==true)
        {
		//__sync_fetch_and_add(&RDcalculationnumber,1);
            ts_buffer=&rawfile[j*index->settings->timeseries_size];
            float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
            if(dist < bsf_distance) {
                bsf_distance = dist;
            }
        }
    }

    approximate_result.distance=bsf_distance;
        COUNT_QUEUE_TIME_END

            //printf(" the Real distance calculation is \t%ld\t\n ",RDcalculationnumber);

    return approximate_result;
}
query_result exact_search_serial_PplusG(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,bool *positionmap,bool *gpositionmap,float *gdictionary) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    RDcalculationnumber=0;

    pthread_barrier_t lock_barrier1;
    pthread_barrier_init(&lock_barrier1, NULL, 2);


    pthread_t threadid;
    COUNT_INPUT_TIME_START
   // bool *rdcbitmap=(bool*)malloc(sizeof(bool) * index->sax_cache_size);
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    ts_type *ts_buffer = (ts_type*)malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);


    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=approximate_result.distance;

    //LBDcalculationnumber=index->sax_cache_size;
    int loopnumber=10;
    GPUtransferdata gputransferdata1;
    gputransferdata1.bsf_distance=&bsf_distance;
    gputransferdata1.gsaxarray=gsaxarray;
    gputransferdata1.positionmap=positionmap;
    gputransferdata1.paa=paa;
    gputransferdata1.gqts=gqts;
    gputransferdata1.datasize=index->sax_cache_size;
    gputransferdata1.gpositionmap=gpositionmap;
    gputransferdata1.lock_barrier1=&lock_barrier1;
    gputransferdata1.loopnumber=loopnumber;
    gputransferdata1.startloop=0;
	gputransferdata1.stoploop=loopnumber;
    gputransferdata1.offsetnumber=0;
    gputransferdata1.segmentnumber=index->settings->paa_segments;
    gputransferdata1.segmentsize=index->settings->mindist_sqrt;
    pthread_create(&threadid,NULL,PplusGworker,(void*)(&gputransferdata1));
    for (int  i = 0; i < loopnumber; i++)
    {
        pthread_barrier_wait(&lock_barrier1);
        COUNT_QUEUE_TIME_START
        #pragma omp parallel for num_threads(maxquerythread-1) reduction(min : bsf_distance)
        for(unsigned long j=i*index->sax_cache_size/loopnumber; j<(i+1)*index->sax_cache_size/loopnumber; j++) {
            if(positionmap[j]==true)
            {	
                ts_buffer=&rawfile[j*index->settings->timeseries_size];
                //__sync_fetch_and_add(&RDcalculationnumber,1);
                float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
                if(dist < bsf_distance) {
                    bsf_distance = dist;
                }
            }
        }
        COUNT_QUEUE_TIME_END
    }
    pthread_join(threadid,NULL);

    approximate_result.distance=bsf_distance;



            //printf(" the Real distance calculation is \t%ld\t\n ",RDcalculationnumber);
    return approximate_result;
}

query_result exact_search_SING_sort (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray ) 
{   
    //COUNT_CAL_TIME_START
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    //COUNT_CAL_TIME_END
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    bool labelvalue=false;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    query_result bsf_result = approximate_result;
    pqueue_t **allpq=(pqueue_t**)malloc(sizeof(pqueue_t*)*N_PQUEUE);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];

    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);



    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread];
    SING_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread+1);
 
    
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;

        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=i%N_PQUEUE;
    workerdata[i].lbdmap=positionmap;
    workerdata[i].labelvalue=&labelvalue;
    }
        

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_SING_sort_worker,(void*)&(workerdata[i]));
    }

    COUNT_CAL_TIME_START
	    LBDfloatstreamGPU(gsaxarray, positionmap, paa, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap,index->settings->paa_segments,index->settings->mindist_sqrt);
    COUNT_CAL_TIME_END
    
	//labelvalue=true;
    pthread_barrier_wait(&lock_barrier);
    COUNT_QUEUE_TIME_START
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    COUNT_QUEUE_TIME_END
    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    bsf_result=bsf_result;

    //free(rfdata);
           //printf("the number of node added is \t%ld\t\t and the number of node remove is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);
    return bsf_result;

    // Free the nodes that where not popped.

}

query_result exact_search_MplusG (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist ) 
{   //SING new
        query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    //COUNT_CAL_TIME_END
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    bool labelvalue=false;
        LBDcalculationnumber=0;
    RDcalculationnumber=0;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    query_result bsf_result = approximate_result;
    pqueue_t **allpq=(pqueue_t**)malloc(sizeof(pqueue_t*)*N_PQUEUE);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];

    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);



    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread];
    SING_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread+1);
 
    
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;

        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=i%N_PQUEUE;
        workerdata[i].lbdmap=positionmap;
        workerdata[i].labelvalue=&labelvalue;
    }
        

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_MplusG_worker,(void*)&(workerdata[i]));
    }

    COUNT_CAL_TIME_START
	    LBDfloatstreamGPU(gsaxarray, positionmap, paa, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap,index->settings->paa_segments,index->settings->mindist_sqrt);
    COUNT_CAL_TIME_END
    
	//labelvalue=true;
    pthread_barrier_wait(&lock_barrier);
    COUNT_QUEUE_TIME_START
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    COUNT_QUEUE_TIME_END
    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    bsf_result=bsf_result;
    //LBDcalculationnumber=index->sax_cache_size;
    //free(rfdata);
        //printf("the number of insert node is \t%ld\t\t and the delete node is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);
    return bsf_result;

    // Free the nodes that where not popped.

}

query_result exact_search_SING (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray ) 
{   
    
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
   // COUNT_CAL_TIME_END
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    
    bool labelvalue=false;
    LBDcalculationnumber=0;
    RDcalculationnumber=0;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    query_result bsf_result = approximate_result;
    pqueue_t **allpq=(pqueue_t**)malloc(sizeof(pqueue_t*)*N_PQUEUE);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];

    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);



    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread],threadid2[maxquerythread];;
    SING_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
    int queueoffset[N_PQUEUE];
    unsigned long int gpuoffset=0;
    int loopnumber=20;
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
    }

    bool *activechunk=(bool*)malloc(sizeof(bool)*(loopnumber+1));
    for (int i = 0; i < loopnumber+1; i++)
    {
        activechunk[i]=false;
    }
    gap_workerdata gapworkerdata[maxquerythread];
    int startnode=nodelist->node_amount, stopnode=0,nodecounter=0,nodecounter2=nodelist->node_amount-1;//, *gapstartnode,*gapstopnode
    bool *activenode=(bool*)malloc(sizeof(bool)*nodelist->node_amount);


    for (int i = 0; i < maxquerythread; i++)
{
    gapworkerdata[i].nodelist=nodelist->nlist;;
	gapworkerdata[i].amountnode=nodelist->node_amount;
    gapworkerdata[i].startnode=&startnode;
    gapworkerdata[i].stopnode=&stopnode;// *gapstartnode,*gapstopnode;

    gapworkerdata[i].nodecounter=&nodecounter;
    gapworkerdata[i].nodecounter2=&nodecounter2;
    gapworkerdata[i].index=index;
    gapworkerdata[i].bsf=approximate_result.distance;
	gapworkerdata[i].paa=paa;//,*paaU,*paaL,*ts,*uo,*lo;
    gapworkerdata[i].lockposition=&lock_queue;
    gapworkerdata[i].offsetarray=offsetarray;
    gapworkerdata[i].activechunk=activechunk;
    gapworkerdata[i].chunknumber=loopnumber;
    gapworkerdata[i].activenode=activenode;
    gapworkerdata[i].workerstartnode=(i)*nodelist->node_amount/maxquerythread;
    gapworkerdata[i].workerstopnode=(i+1)*nodelist->node_amount/maxquerythread;
}
    gapworkerdata[maxquerythread-1].workerstopnode=nodelist->node_amount;

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid2[i]),NULL,multigapworker,(void*)&(gapworkerdata[i]));
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid2[i],NULL);
    }
    if(activechunk[loopnumber])
    activechunk[loopnumber-1]=true;


    int node_counter=0;



    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=(i%N_PQUEUE);//*(N_PQUEUE/maxquerythread);
        workerdata[i].lbdmap=positionmap;
        workerdata[i].labelvalue=&labelvalue;
        workerdata[i].gpuoffset=&gpuoffset;
        workerdata[i].activenode=activenode;

    }


    COUNT_QUEUE_TIME_START

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_SING_worker,(void*)&(workerdata[i]));
    }
//COUNT_CAL_TIME_START
COUNT_CAL_TIME_START
   // cudaStream_t *streams=LBDfloatstreamGPUdevidebegin( qts, gqts,datastartnumber,datasizerun,loopnumber);
        for (int i = 0; i < loopnumber; i++)
        {
    	    if(activechunk[i])
            {
                LBDfloatstreamGPU(&gsaxarray[i*index->sax_cache_size/loopnumber*index->settings->paa_segments], &positionmap[i*index->sax_cache_size/loopnumber], paa, gqts, approximate_result.distance,index->sax_cache_size/loopnumber,&gpositionmap[i*index->sax_cache_size/loopnumber],index->settings->paa_segments,index->settings->mindist_sqrt);
                LBDcalculationnumber+=index->sax_cache_size/loopnumber;
            }
            //__sync_fetch_and_add(&gpuoffset,index->sax_cache_size/10);
            gpuoffset=(i+1)*index->sax_cache_size/loopnumber;
        }
        
	    //LBDfloatstreamGPU(gsaxarray, positionmap, paa, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap, gdictionary);
        
    
    COUNT_CAL_TIME_END
    
	//labelvalue=true;
    //pthread_barrier_wait(&lock_barrier);
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    COUNT_QUEUE_TIME_END
    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    bsf_result=bsf_result;
    //free(rfdata);
    //LBDcalculationnumber=(datasizerun-datastartnumber)*index->sax_cache_size/loopnumber;
    //printf("the number of node added is \t%ld\t\t and the number of node remove is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);
    //printf("the number of lower bound distance calculation is \t%ld\t\t and the delete node is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);

    return bsf_result;

    // Free the nodes that where not popped.

}

pqueue_bsf exact_knn_SING (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray,int k ) 
{   
    
    pqueue_bsf *pq_bsf= pqueue_bsf_init(k);
    approximate_topk_SING(ts, paa, index, pq_bsf);
   // COUNT_CAL_TIME_END
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    
    bool labelvalue=false;
    LBDcalculationnumber=0;
    RDcalculationnumber=0;
    // Early termination...
    if(pq_bsf->knn[k-1] == FLT_MAX || min_checked_leaves > 1) {
        refine_topk_answer_inmemory(ts, paa, index, pq_bsf, minimum_distance, min_checked_leaves);
    }
    pqueue_t **allpq=(pqueue_t**)malloc(sizeof(pqueue_t*)*N_PQUEUE);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];






    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread],threadid2[maxquerythread];;
    SING_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
    int queueoffset[N_PQUEUE];
    unsigned long int gpuoffset=0;
    int loopnumber=20;
    for (int i = 0; i < N_PQUEUE; i++)
    {
        allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
        pthread_mutex_init(&ququelock[i], NULL);
        queuelabel[i]=1;
    }

    bool *activechunk=(bool*)malloc(sizeof(bool)*(loopnumber+1));
    for (int i = 0; i <= loopnumber+1; i++)
    {
        activechunk[i]=false;
    }
    gap_workerdata gapworkerdata[maxquerythread];
    int startnode=nodelist->node_amount, stopnode=0,nodecounter=0,nodecounter2=nodelist->node_amount-1;//, *gapstartnode,*gapstopnode
    bool *activenode=(bool*)malloc(sizeof(bool)*nodelist->node_amount);
    for (int i = 0; i < maxquerythread; i++)
    {
        gapworkerdata[i].nodelist=nodelist->nlist;
        gapworkerdata[i].amountnode=nodelist->node_amount;
        gapworkerdata[i].startnode=&startnode;
        gapworkerdata[i].stopnode=&stopnode;// *gapstartnode,*gapstopnode;

        gapworkerdata[i].nodecounter=&nodecounter;
        gapworkerdata[i].nodecounter2=&nodecounter2;
        gapworkerdata[i].index=index;
        gapworkerdata[i].bsf=pq_bsf->knn[pq_bsf->k-1];
        gapworkerdata[i].paa=paa;//,*paaU,*paaL,*ts,*uo,*lo;
        gapworkerdata[i].lockposition=&lock_queue;
        gapworkerdata[i].offsetarray=offsetarray;
        gapworkerdata[i].activechunk=activechunk;
        gapworkerdata[i].chunknumber=loopnumber;
        gapworkerdata[i].activenode=activenode;
        gapworkerdata[i].workerstartnode=(i)*nodelist->node_amount/loopnumber;
        gapworkerdata[i].workerstopnode=(i+1)*nodelist->node_amount/loopnumber;
    }
    gapworkerdata[maxquerythread-1].workerstopnode=nodelist->node_amount;

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid2[i]),NULL,multigapworker,(void*)&(gapworkerdata[i]));
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid2[i],NULL);
    }
    if(activechunk[loopnumber])
        activechunk[loopnumber-1]=true;

    int node_counter=0;







    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        //workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=(i%N_PQUEUE);//*(N_PQUEUE/maxquerythread);
        workerdata[i].lbdmap=positionmap;
        workerdata[i].labelvalue=&labelvalue;
        workerdata[i].gpuoffset=&gpuoffset;
        workerdata[i].activenode=activenode;
        workerdata[i].pq_bsf=pq_bsf;


    }


    //COUNT_QUEUE_TIME_START

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_knn_SING_worker,(void*)&(workerdata[i]));
    }
//COUNT_CAL_TIME_START
COUNT_CAL_TIME_START
        for (int i = 0; i < loopnumber; i++)
        {
            //LBDfloatstreamGPUdevideoneblock(gsaxarray,positionmap, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap,gdictionary,index->settings->mindist_sqrt, loopnumber, i,streams);
    	    if(activechunk[i])
            {
                LBDfloatstreamGPU(&gsaxarray[i*index->sax_cache_size/loopnumber*index->settings->paa_segments], &positionmap[i*index->sax_cache_size/loopnumber], paa, gqts, pq_bsf->knn[pq_bsf->k-1],index->sax_cache_size/loopnumber,&gpositionmap[i*index->sax_cache_size/loopnumber],index->settings->paa_segments,index->settings->mindist_sqrt);
                LBDcalculationnumber+=index->sax_cache_size/loopnumber;
            }
            //__sync_fetch_and_add(&gpuoffset,index->sax_cache_size/10);
            gpuoffset=(i+1)*index->sax_cache_size/loopnumber;
        }
        
	    //LBDfloatstreamGPU(gsaxarray, positionmap, paa, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap, gdictionary);
        
    
    COUNT_CAL_TIME_END
    
	//labelvalue=true;
    //pthread_barrier_wait(&lock_barrier);
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
   // COUNT_QUEUE_TIME_END
    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    return *pq_bsf;

    // Free the nodes that where not popped.

}

void* exact_search_SING_sort_worker(void *rfdata)
{   

    struct timeval workertimestart;
    struct timeval writetiemstart;
    struct timeval workercurenttime;
    struct timeval writecurenttime;
    double worker_total_time,write_total_time;
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance,distance;
    int calculate_node=0,calculate_node_quque=0;
    int tnumber=rand()% N_PQUEUE;
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    //COUNT_QUEUE_TIME_START

    while (1)
    {
        current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
        if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
            break;
        current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
        insert_tree_node_m_hybridpqueue(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
    }

    //COUNT_QUEUE_TIME_END
    //calculate_node_quque=pq->size;

    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
    while (1)
    {
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]);
             //   __sync_fetch_and_add(&RDcalculationnumber,1);
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((SING_workerdata*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

                checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                           // gettimeofday(&workertimestart, NULL);
                
                distance = calculate_node_distance_SING(index, n->node, ts,paa,bsfdisntance);
                 //gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
               // distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
        free(n);
    }

    if( (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])==1)
    {
        (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        while(n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]))
        {
            free(n);
        }
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
    }

    while(1)
    {   
        int offset=rand()% N_PQUEUE;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((SING_workerdata*)rfdata)->allqueuelabel[i])==1)
            {
                finished=false;
                while(1)
                {
                    pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[i]);
                //__sync_fetch_and_add(&RDcalculationnumber,1);
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    if(n==NULL)
                    break;
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                           // gettimeofday(&workertimestart, NULL);
                
                distance = calculate_node_distance_SING(index, n->node, ts,paa,bsfdisntance);
                // gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
                //distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                //add
                free(n);
                }

            }
        }
        if (finished)
        {
            break;
        }
    }
}

void* exact_search_MplusG_worker(void *rfdata)
{   

    struct timeval workertimestart;
    struct timeval writetiemstart;
    struct timeval workercurenttime;
    struct timeval writecurenttime;
    double worker_total_time,write_total_time;
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance,distance;
    int calculate_node=0,calculate_node_quque=0;
    int tnumber=rand()% N_PQUEUE;
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    //COUNT_QUEUE_TIME_START
    while (1)
    {
        current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
        if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
            break;

        current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
        insert_tree_node_m_hybridpqueue(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
    }

    //COUNT_QUEUE_TIME_END
    //calculate_node_quque=pq->size;

    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
    while (1)
    {
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]);
                                     //   __sync_fetch_and_add(&RDcalculationnumber,1);

        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((SING_workerdata*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            
            break;
        }
        else 
        { 
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {
                checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                    //        gettimeofday(&workertimestart, NULL);
                
                distance = calculate_node_distance_MplusG(index, n->node, ts,paa,lbdmap,bsfdisntance);
              //   gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
               // distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);

                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
        free(n);
    }

    if( (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])==1)
    {
        (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        while(n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]))
        {
            free(n);
        }
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
    }
    while(1)
    {   
        int offset=rand()% N_PQUEUE;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((SING_workerdata*)rfdata)->allqueuelabel[i])==1)
            {
                finished=false;
                while(1)
                {
                    pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[i]);
                                            //        __sync_fetch_and_add(&RDcalculationnumber,1);

                //__sync_fetch_and_add(&RDcalculationnumber,1);
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    if(n==NULL)
                    break;
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                    //        gettimeofday(&workertimestart, NULL);
                
                distance = calculate_node_distance_MplusG(index, n->node, ts,paa,lbdmap,bsfdisntance);
                // gettimeofday(&workercurenttime, NULL); \
                  //                    tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                 ////                     tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
               // distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                //add
                free(n);
                }

            }
        }
        if (finished)
        {
            break;
        }
    }


}

void* exact_search_SING_worker(void *rfdata)
{   

    struct timeval workertimestart;
    struct timeval writetiemstart;
    struct timeval workercurenttime;
    struct timeval writecurenttime;
    double worker_total_time,write_total_time;
    
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance,distance;
    int calculate_node=0,calculate_node_quque=0;
    bool *activenode=((SING_workerdata*)rfdata)->activenode;
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    int tnumber=startqueuenumber;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    int localqueuecounter=0;
    //COUNT_QUEUE_TIME_START
    pthread_mutex_t *alllock=((SING_workerdata*)rfdata)->alllock;
    while (1)
    {
        current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
        if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
            break;
        current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
        if(activenode[current_root_node_number]==true)
        insert_tree_node_m_hybridpqueueBreakpolyroot(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
    }
    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);

int queuenumber;
int offset=startqueuenumber;
    while(1)
    { 
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            queuenumber=(i+offset)%N_PQUEUE;
            if((((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])==1)
            {
                finished=false;
                bsfdisntance=bsf_result->distance;
                pthread_mutex_lock(&alllock[queuenumber]);
                n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[queuenumber]);
                if(n==NULL)
                {
                    (((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])=0;
                    pthread_mutex_unlock(&alllock[queuenumber]);
                    continue;
                }
                else 
                {
                    //n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[queuenumber]);
                           //     __sync_fetch_and_add(&RDcalculationnumber,1);

                    pthread_mutex_unlock(&alllock[queuenumber]);                    
                }
                // __sync_fetch_and_add(&RDcalculationnumber,1);
                
                if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                    (((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])=0;
                    continue;
                }
                else 
                {
                    // If it is a leaf, check its real distance.
                    if (n->node->is_leaf) 
                    {
                        checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
           // gettimeofday(&workertimestart, NULL);
                if(n->node->buffer->arrayposition < *((SING_workerdata*)rfdata)->gpuoffset)
                    distance = calculate_node_distance_SING(index, n->node, ts,paa,bsfdisntance);
                else
                    distance = calculate_node_distance_cal_inmemory(index, n->node, ts,paa, bsfdisntance);
//gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
                //distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                        if (distance < bsfdisntance)
                        {
                            pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                            if(distance < bsf_result->distance)
                            {
                                bsf_result->distance = distance;
                                bsf_result->node = n->node;
                            }
                            pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                        }
                    }
                }
                free(n);
            }
            else if((((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])==0)
            {
                pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[queuenumber]));
                n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[queuenumber]);
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[queuenumber]));                    
                if(n!=NULL)
                {
                    free(n);
                }
                else
                {
                    (((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])=2;
                }
                
            }
        }
        if (finished)
        {
            break;
        }
    }
}

void* exact_knn_SING_worker(void *rfdata)
{   

    struct timeval workertimestart;
    struct timeval writetiemstart;
    struct timeval workercurenttime;
    struct timeval writecurenttime;
    double worker_total_time,write_total_time;
    
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    int calculate_node=0,calculate_node_quque=0;
    bool *activenode=((SING_workerdata*)rfdata)->activenode;
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    int tnumber=startqueuenumber;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    int localqueuecounter=0;
    pqueue_bsf *pq_bsf=((SING_workerdata*)rfdata)->pq_bsf;
    float bsfdisntance=pq_bsf->knn[pq_bsf->k-1];

    //COUNT_QUEUE_TIME_START
    pthread_mutex_t *alllock=((SING_workerdata*)rfdata)->alllock;
    while (1)
    {
        current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
        if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
            break;
        current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
        if(activenode[current_root_node_number]==true)
        insert_tree_node_m_hybridpqueueBreakpolyroot(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
    }
    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);

int queuenumber;
int offset=startqueuenumber;
    while(1)
    { 
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            queuenumber=(i+offset)%N_PQUEUE;
            if((((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])==1)
            {
                finished=false;
                bsfdisntance=pq_bsf->knn[pq_bsf->k-1];
                pthread_mutex_lock(&alllock[queuenumber]);
                n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[queuenumber]);
                if(n==NULL)
                {
                    (((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])=0;
                    pthread_mutex_unlock(&alllock[queuenumber]);
                    continue;
                }
                else 
                {
                    //n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[queuenumber]);
                           //     __sync_fetch_and_add(&RDcalculationnumber,1);

                    pthread_mutex_unlock(&alllock[queuenumber]);                    
                }
                // __sync_fetch_and_add(&RDcalculationnumber,1);
                
                if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                    (((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])=0;
                    continue;
                }
                else 
                {
                    // If it is a leaf, check its real distance.
                    if (n->node->is_leaf) 
                    {
                        checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
            //gettimeofday(&workertimestart, NULL);
                if(n->node->buffer->arrayposition < *((SING_workerdata*)rfdata)->gpuoffset)
                calculate_node_topk_SING(index, n->node, ts,paa, pq_bsf,((SING_workerdata*)rfdata)->lock_bsf);
                else
                calculate_node_cal_topk_inmemory(index, n->node, ts,paa, pq_bsf,((SING_workerdata*)rfdata)->lock_bsf);
               // gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
                //distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);

                    }
                }
                free(n);
            }
            else if((((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])==0)
            {
                pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[queuenumber]));
                n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[queuenumber]);
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[queuenumber]));                    
                if(n!=NULL)
                {
                    free(n);
                }
                else
                {
                    (((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])=2;
                }
                
            }
        }
        if (finished)
        {
            break;
        }
    }

}


float calculate_node_distance_MplusG (isax_index *index, isax_node *node, ts_type *query,ts_type *paa,float* lbdmap, float bsf) 
{
    COUNT_CHECKED_NODE()
    float distmin;
    // If node has buffered data
    if (node->buffer != NULL) 
    {
        int i;
        //#pragma omp parallel for num_threads(2) reduction(min : bsf)
        
       // __sync_fetch_and_add(&LBDcalculationnumber,node->buffer->partial_buffer_size);
        for (i=0; i<node->buffer->partial_buffer_size; i++) {

            distmin=lbdmap[*node->buffer->partial_position_buffer[i]/index->settings->timeseries_size];
            if (distmin<bsf)
            {
                float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                               index->settings->timeseries_size, bsf);
                //__sync_fetch_and_add(&RDcalculationnumber,1);
                if (dist < bsf) {
                    bsf = dist;
                }
            }
        }
    }
    return bsf;
}

float calculate_node_distance_SING (isax_index *index, isax_node *node, ts_type *query,ts_type *paa, float bsf) 
{
    COUNT_CHECKED_NODE()
    float distmin;
    // If node has buffered data
    if (node->buffer != NULL) 
    {
        int i;
        //#pragma omp parallel for num_threads(2) reduction(min : bsf)
        
       // __sync_fetch_and_add(&LBDcalculationnumber,node->buffer->partial_buffer_size);
        for (i=0; i<node->buffer->partial_buffer_size; i++) {

            if (node->buffer->lbdarray[i]<bsf)
            {
                float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                               index->settings->timeseries_size, bsf);
                //__sync_fetch_and_add(&RDcalculationnumber,1);
                if (dist < bsf) {
                    bsf = dist;
                }
            }
        }
    }
    return bsf;
}

void calculate_node_topk_SING (isax_index *index, isax_node *node, ts_type *query,ts_type *paa, pqueue_bsf *pq_bsf, pthread_rwlock_t *lock_queue ) 
{
    COUNT_CHECKED_NODE()
    // If node has buffered data
    if (node->buffer != NULL) 
    {
        int i;

        for (i=0; i<node->buffer->partial_buffer_size; i++) {
            if (node->buffer->lbdarray[i]<= pq_bsf->knn[pq_bsf->k-1])
            {                                   
            float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pthread_rwlock_wrlock(lock_queue);
                COUNT_QUEUE_TIME_START
                pqueue_bsf_insert(pq_bsf,dist,*node->buffer->partial_position_buffer[i]/index->settings->timeseries_size,node);
                COUNT_QUEUE_TIME_END
                pthread_rwlock_unlock(lock_queue);
            }
            }
        }
    }
}




void* PplusGworker(void *GPUdtransferdata)
{
    int   loopnumber=((GPUtransferdata*)GPUdtransferdata)->loopnumber;
    long int blocksize=((GPUtransferdata*)GPUdtransferdata)->datasize/loopnumber;
    sax_type *gsaxarray=((GPUtransferdata*)GPUdtransferdata)->gsaxarray;
    bool *positionmap=((GPUtransferdata*)GPUdtransferdata)->positionmap;
    bool *gpositionmap=((GPUtransferdata*)GPUdtransferdata)->gpositionmap;
    float *paa=((GPUtransferdata*)GPUdtransferdata)->paa;
    float *gqts=((GPUtransferdata*)GPUdtransferdata)->gqts;
	int startloop=((GPUtransferdata*)GPUdtransferdata)->startloop;
    int segmentnumber=((GPUtransferdata*)GPUdtransferdata)->segmentnumber;

    float segmentsize=((GPUtransferdata*)GPUdtransferdata)->segmentsize;
int stoploop=((GPUtransferdata*)GPUdtransferdata)->stoploop;
    for (int i = startloop; i < stoploop; i++)
    {    COUNT_CAL_TIME_START
        unsigned long int offsetnumber=i*blocksize+((GPUtransferdata*)GPUdtransferdata)->offsetnumber;
        LBDstreamGPU(&gsaxarray[offsetnumber*segmentnumber], &positionmap[offsetnumber], paa, gqts, *((GPUtransferdata*)GPUdtransferdata)->bsf_distance,blocksize,&gpositionmap[offsetnumber],segmentnumber,segmentsize);
    COUNT_CAL_TIME_END
        pthread_barrier_wait(((GPUtransferdata*)GPUdtransferdata)->lock_barrier1);
    }
}

void pass_tree_node_m(isax_node *node,isax_index *index,pthread_mutex_t *lock_queue,unsigned long int *currentposition,sax_type *saxarray,sax_type *sortsaxarray, float *lbdarray)
{   
    if (node->is_leaf) 
    {   
        unsigned long int arrayposition=__sync_fetch_and_add (currentposition, node->buffer->partial_buffer_size);
        node->buffer->lbdarray=&lbdarray[arrayposition];
        node->buffer->arrayposition=arrayposition+node->buffer->partial_buffer_size;
        for (int i=0; i<node->buffer->partial_buffer_size; i++)
        {
            mempcpy((void*)&(sortsaxarray[(arrayposition+i)*index->settings->paa_segments]),node->buffer->partial_sax_buffer[i],index->settings->sax_byte_size);
        } 

    }
    else
    {   
        if (node->left_child->isax_cardinalities != NULL)
        {
            pass_tree_node_m(node->left_child,index,lock_queue,currentposition,saxarray,sortsaxarray,lbdarray);
        }
        if (node->right_child->isax_cardinalities != NULL)
        {
            pass_tree_node_m(node->right_child,index,lock_queue,currentposition,saxarray,sortsaxarray,lbdarray);        
        }
    }
}


void* gapworker( void *gapworkerdata)
{
    
    int   amountnode=((gap_workerdata*)gapworkerdata)->amountnode;
    isax_node **nodelist=((gap_workerdata*)gapworkerdata)->nodelist;
    isax_index *index=((gap_workerdata*)gapworkerdata)->index;
    ts_type *paa=((gap_workerdata*)gapworkerdata)->paa;
    float bsf=((gap_workerdata*)gapworkerdata)->bsf;
    //bool *activenode=((gap_workerdata*)gapworkerdata)->
    int current_root_node_number;
    while(1)
    {
        current_root_node_number=__sync_fetch_and_add(((gap_workerdata*)gapworkerdata)->nodecounter,1);
        isax_node* node=nodelist[current_root_node_number];
       /* float distance =  minidist_paa_to_isax(paa, node->isax_values,
                                           node->isax_cardinalities,
                                           index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);*/
        float distance =  nodedistance(paa, node->isax_values,
                                           node->isax_cardinalities,
                                           index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt); 
        //if(distance2!=distance)
        //("the node distance 1 is \t%f\t the node distance 2 is \t%f\n",distance,distance2);                                
            if(distance <bsf ||*((gap_workerdata*)gapworkerdata)->startnode!=amountnode ||current_root_node_number>= amountnode)
            break;
    }

    pthread_mutex_lock(((gap_workerdata*)gapworkerdata)->lockposition);
    if(*((gap_workerdata*)gapworkerdata)->startnode>current_root_node_number)
    *((gap_workerdata*)gapworkerdata)->startnode=current_root_node_number;
    pthread_mutex_unlock(((gap_workerdata*)gapworkerdata)->lockposition);

    while(1)
    {
        current_root_node_number=__sync_fetch_and_sub(((gap_workerdata*)gapworkerdata)->nodecounter2,1);
        isax_node* node=nodelist[current_root_node_number];
       /* float distance =  minidist_paa_to_isax_Breakpoly(paa, node->isax_values,
                                           node->isax_cardinalities,
                                           index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);*/
        float distance =  nodedistance(paa, node->isax_values,
                                           node->isax_cardinalities,
                                           index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt); 
     //if(distance2!=distance)

        if(distance <bsf ||*((gap_workerdata*)gapworkerdata)->stopnode!=0 ||current_root_node_number< 0)
            break;
    }

    pthread_mutex_lock(((gap_workerdata*)gapworkerdata)->lockposition);
    if(*((gap_workerdata*)gapworkerdata)->stopnode<current_root_node_number)
    *((gap_workerdata*)gapworkerdata)->stopnode=current_root_node_number;
    pthread_mutex_unlock(((gap_workerdata*)gapworkerdata)->lockposition);

}


void* multigapworker( void *gapworkerdata)
{
    
    int   amountnode=((gap_workerdata*)gapworkerdata)->amountnode;
    isax_node **nodelist=((gap_workerdata*)gapworkerdata)->nodelist;
    isax_index *index=((gap_workerdata*)gapworkerdata)->index;
    ts_type *paa=((gap_workerdata*)gapworkerdata)->paa;
    float bsf=((gap_workerdata*)gapworkerdata)->bsf;
    unsigned long* offsetarray=((gap_workerdata*)gapworkerdata)->offsetarray;
    bool *activechunk=((gap_workerdata*)gapworkerdata)->activechunk;
    bool *activenode=((gap_workerdata*)gapworkerdata)->activenode;
    int current_root_node_number;
    int chunknumber=((gap_workerdata*)gapworkerdata)->chunknumber;
    for (current_root_node_number = ((gap_workerdata*)gapworkerdata)->workerstartnode; current_root_node_number < ((gap_workerdata*)gapworkerdata)->workerstopnode; current_root_node_number++)
    {
        isax_node* node=nodelist[current_root_node_number];
        float distance =  nodedistance(paa, node->isax_values,
                                           node->isax_cardinalities,
                                           index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt); 
            if(distance<bsf)
            {
                activechunk[(int)(offsetarray[current_root_node_number]/(index->sax_cache_size/chunknumber))]=true;
                activechunk[(int)(offsetarray[current_root_node_number+1]/(index->sax_cache_size/chunknumber))]=true;
                activenode[current_root_node_number]=true;
            }
            else
            {
                activenode[current_root_node_number]=false;
            }
    }    
}

float nodedistance(float *paa, sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt) 
{

    float distance = 0;
    // For each sax record find the break point
    int i;
    for (i=0; i<number_of_segments; i++) 
    {
        sax_type c_c = sax[i];
        if((c_c==0 && paa[i]>0.0f))
        {
            distance+=paa[i]*paa[i];
        }
        else if (c_c !=0 && paa[i]<0.0f)
        {
            distance+=paa[i]*paa[i];
        }
    }
    //distance = ratio_sqrt * sqrtf(distance);
    distance = ratio_sqrt * distance;
    return distance;
}

void  approximate_topk_SING (ts_type *ts, ts_type *paa, isax_index *index,pqueue_bsf *pq_bsf) 
{

    sax_type *sax = (sax_type*)malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(paa, sax, index->settings->paa_segments,
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);

    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);

    if ((&((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) root_mask])->initialized) {
        isax_node *node = (&((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) root_mask])->node;
        // Traverse tree

        // Adaptive splitting

        while (!node->is_leaf) {
            int location = index->settings->sax_bit_cardinality - 1 -
            node->split_data->split_mask[node->split_data->splitpoint];
            root_mask_type mask = index->settings->bit_masks[location];

            if(sax[node->split_data->splitpoint] & mask)
            {
                node = node->right_child;
            }
            else
            {
                node = node->left_child;
            }

            // Adaptive splitting
        }
        calculate_node_topk_inmemory(index, node, ts, pq_bsf);
    }
    else {

    }
    for (int i = 0; i < pq_bsf->k-1; ++i)
    {
        pq_bsf->knn[i]=pq_bsf->knn[pq_bsf->k-1];
    }
    free(sax);
}


float minidist_paa_to_isax_Breakpoly(float *paa, sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt) 
{

    float distance = 0;
    // TODO: Store offset in index settings. and pass index settings as parameter.
    
    int offset = ((max_cardinality - 1) * (max_cardinality - 2)) / 2;
    
    // For each sax record find the break point
    int i;
    for (i=0; i<number_of_segments; i++) {

        sax_type c_c = sax_cardinalities[i];

        sax_type c_m = max_bit_cardinality;
        sax_type v = sax[i];
        //sax_print(&v, 1, c_m);
        
        
        sax_type region_lower = (v <<  (c_m - c_c));
        sax_type region_upper = (~((int)MAXFLOAT << (c_m - c_c)) | region_lower);
		//printf("[%d, %d] %d -- %d\n", sax[i], c_c, region_lower, region_upper);
        float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        float breakpoint_upper = 0; // <-- - || -
        
        
        if (region_lower == 0) {
            breakpoint_lower = min_val;
        }
        else
        {
            breakpoint_lower = sax_breakpointsnew3[ region_lower];
        }
        if (region_upper == max_cardinality - 1) {
            breakpoint_upper = max_val;
        }
        else
        {
            breakpoint_upper = sax_breakpointsnew3[ region_upper+1];
        }
        //printf("\n%d.%d is from %d to %d, %lf - %lf\n", v, c_c, region_lower, region_upper,
        //       breakpoint_lower, breakpoint_upper);

        //printf("FROM: \n");
        //sax_print(&region_lower, 1, c_m);
        //printf("TO: \n");
        //sax_print(&region_upper, 1, c_m);
		
		//printf ("\n---------\n");
        
        if (breakpoint_lower > paa[i]) {
            distance += pow(breakpoint_lower - paa[i], 2);
        }
        else if(breakpoint_upper < paa[i]) {
            distance += pow(breakpoint_upper - paa[i], 2);
        }
        else {
    //      printf("%lf is between: %lf and %lf\n", paa[i], breakpoint_lower, breakpoint_upper);
        }
    }
    //distance = ratio_sqrt * sqrtf(distance);
    distance = ratio_sqrt * distance;
    return distance;
}

void insert_tree_node_m_hybridpqueueBreakpoly(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,int *tnumber)
{   
    //COUNT_CAL_TIME_START
    float distance =  minidist_paa_to_isax_Breakpoly(paa, node->isax_values,
                                            node->isax_cardinalities,
                                            index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);
    //COUNT_CAL_TIME_END
    if(distance < bsf)
    {
        if (node->is_leaf) 
        {   
            query_result * mindist_result = (query_result *)malloc(sizeof(query_result));
            mindist_result->node = node;
            mindist_result->distance=distance;
            pthread_mutex_lock(&lock_queue[*tnumber]);
            pqueue_insert(pq[*tnumber], mindist_result);
           // __sync_fetch_and_add(&LBDcalculationnumber,1);

            pthread_mutex_unlock(&lock_queue[*tnumber]);
            *tnumber=(*tnumber+1)%N_PQUEUE;
            added_tree_node++;
        }
        else
        {   
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_hybridpqueueBreakpoly(paa,node->left_child,index, bsf,pq,lock_queue,tnumber);
            }
            if (node->right_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_hybridpqueueBreakpoly(paa,node->right_child,index,bsf,pq,lock_queue,tnumber);
            }
        }
    }
}

void insert_tree_node_m_hybridpqueueBreakpolyroot(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,int *tnumber)
{   
    //COUNT_CAL_TIME_START
    //
    //COUNT_CAL_TIME_END
    //if(distance < bsf)
    //{
        if (node->is_leaf) 
        {   
            float distance =  minidist_paa_to_isax_Breakpoly(paa, node->isax_values,
                                            node->isax_cardinalities,
                                            index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);
            query_result * mindist_result = (query_result *)malloc(sizeof(query_result));
            mindist_result->node = node;
            mindist_result->distance=distance;
            pthread_mutex_lock(&lock_queue[*tnumber]);
            pqueue_insert(pq[*tnumber], mindist_result);
            // __sync_fetch_and_add(&LBDcalculationnumber,1);
            pthread_mutex_unlock(&lock_queue[*tnumber]);
            *tnumber=(*tnumber+1)%N_PQUEUE;
            added_tree_node++;
        }
        else
        {   
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_hybridpqueueBreakpoly(paa,node->left_child,index, bsf,pq,lock_queue,tnumber);
            }
            if (node->right_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_hybridpqueueBreakpoly(paa,node->right_child,index,bsf,pq,lock_queue,tnumber);
            }
        }
    //}
}





























/*



 void isax_query_binary_file_gpugrid(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type**,bool**,bool**,float*,unsigned long*)) 
{
	fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);
	FILE * ifile;
	ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }





    fseek(ifile, 0L, SEEK_END);
    unsigned long long sz = (unsigned long long) ftell(ifile);
    unsigned long long total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

	int q_loaded = 0;
    ts_type * ts = (ts_type *)malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = (ts_type *)malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    float *dictionary =NULL,*gdictionary = NULL;
    bool **gposbitmap = NULL;
    bool **posbitmap = NULL;
    float *qts = NULL,*gqts = NULL;
    sax_type **saxarray=NULL,**gsaxarray=NULL;
    float *saxpoint=&sax_breakpoints[0];
    unsigned long *gridsize=NULL;
    initialdevice();
    int buffernumber=pow(2, index->settings->paa_segments);
    saxarray=(sax_type**)malloc(sizeof(sax_type) * buffernumber);
    gsaxarray=(sax_type**)malloc(sizeof(sax_type) * buffernumber);
    posbitmap=(bool**)malloc(sizeof(bool*) * buffernumber);
    gposbitmap=(bool**)malloc(sizeof(bool*) * buffernumber);
    gridsize=(unsigned long*)malloc(sizeof(unsigned long) * buffernumber);
	gqts=initialgqts(gqts);
	dictionary=(float*)malloc(257*sizeof(float)); 
	gdictionary=initialgdictionary(gdictionary);


	for(int i =0;i<buffernumber;i++)
	{
        fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];
        if(current_buffer->initialized==1)
        {
            saxarray[i]=current_buffer->sax_records;
            gridsize[i]=(unsigned long int)current_buffer->max_buffer_size;
        }
		else
		{
            gridsize[i]=0;
        }
	}
	for(int i =0;i<buffernumber;i++)
	{
		if(gridsize[i]!=0)
		{            	
			gposbitmap[i]=initialgposbitmap(gposbitmap[i],gridsize[i]);//cudaMalloc(&gposbitmap[i], sizeof(float)*gridsize[i]); 

		}
	}
	for(int i =0;i<buffernumber;i++)
	{
		if(gridsize[i]!=0)
		{            	
			gsaxarray[i]=initialgsaxarray(gsaxarray[i],gridsize[i]);//cudaMalloc(&gsaxarray[i], sizeof(float)*gridsize[i]*16); 
		}
	}
	for(int i =0;i<buffernumber;i++)
	{
		if(gridsize[i]!=0)
		{            	
            posbitmap[i]=initialposbitmap(posbitmap[i],gridsize[i]);//cudaMallocHost(&posbitmap[i], sizeof(float)*gridsize[i]); 
            gpusaxgridmemcpy(gsaxarray[i],saxarray[i],gridsize[i]);//cudaMemcpy(gsaxarray[i], saxarray[i],sizeof(float)*gridsize[i]*16,cudaMemcpyHostToDevice);
		}
	}
	gpudictionarymemcpy(gdictionary,sax_breakpoints);//cudaMemcpy(gdictionary, &sax_breakpoints[offset-1], sizeof(float)*257,cudaMemcpyHostToDevice);




    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
		//memcpy(qts,ts,sizeof(float)*256);

        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
	//printf("the the start of the query\n");
        query_result result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,qts,gqts,gsaxarray, posbitmap,gposbitmap,gdictionary,gridsize);
        //COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        //PRINT_STATS(result.distance)
        
        fflush(stdout);
    #if VERBOSE_LEVEL >= 1
        printf("[%p]: Distance: %lf\n", result.node, result.distance);
    #endif
        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}

        q_loaded++;
	}
    free(paa);
    free(ts);
	fclose(ifile);
	fprintf(stderr, ">>> Finished querying.\n");

}

void isax_query_binary_file_gpu2(const char *ifilename,const char *ifilename2, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,bool*,bool*,float*,unsigned long long*,unsigned long int *)) 
{
	fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);
	FILE * ifile;
	ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    FILE * raw_file;
    COUNT_INPUT_TIME_START
    raw_file = fopen (ifilename2,"rb");
    fseek(ifile, 0L, SEEK_END);
    unsigned long long sz = (unsigned long long) ftell(ifile);
    unsigned long long total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

	int q_loaded = 0;
    ts_type * ts = (ts_type *)malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = (ts_type *)malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
	int buffernumber=pow(2, index->settings->paa_segments);
    float *dictionary =NULL,*gdictionary = NULL;
    bool *gposbitmap = NULL;
    bool *posbitmap = NULL;
    float *qts = NULL,*gqts = NULL;
    sax_type *saxarray=NULL,*gsaxarray=NULL;
    float *saxpoint=&sax_breakpoints[0];
	sax_type *saxarrarysort=(sax_type*)malloc(sizeof(sax_type)*index->sax_cache_size*index->settings->paa_segments);
	unsigned long long *positionarray=(unsigned long long*)malloc(sizeof(unsigned long long)*index->sax_cache_size);
	unsigned long long *oppositionarray=(unsigned long long*)malloc(sizeof(unsigned long long)*index->sax_cache_size);
    unsigned long int *offsetarray=(unsigned long int*)malloc(sizeof(unsigned long int)*buffernumber);
    unsigned long int currentpositon=0;

	for(int i =0;i<buffernumber;i++)
	{
		fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];
		offsetarray[i]=currentpositon;
    //printf("the number is %ld\n",offsetarray[i]);
        if(current_buffer->initialized==1)
        {


		memcpy(&saxarrarysort[currentpositon*index->settings->paa_segments],current_buffer->sax_records,current_buffer->max_buffer_size*sizeof(sax_type)*index->settings->paa_segments);
		memcpy(&positionarray[currentpositon],current_buffer->pos_records,current_buffer->max_buffer_size*sizeof(unsigned long long));
        currentpositon=currentpositon+current_buffer->max_buffer_size;
        }
		
	}
	saxarray=initialsaxarray(gsaxarray,index->sax_cache_size);
	dictionary=(float*)malloc(257*sizeof(float)); 
	posbitmap=initialposbitmap(posbitmap,index->sax_cache_size);
	qts=(float*)malloc( sizeof(float)*index->settings->paa_segments); 
	initialdevice();
	gqts=initialgqts(gqts);
    	
	gdictionary=initialgdictionary(gdictionary);
	gposbitmap=initialgposbitmap(gposbitmap,index->sax_cache_size);
	gsaxarray=initialgsaxarray(gsaxarray,index->sax_cache_size);
 	gpumemcpy(gsaxarray,saxarrarysort,saxpoint,gdictionary,index->sax_cache_size);
	//for(unsigned long j  =0;j<index->sax_cache_size;j++)
	{
   // if(j%1000000==0)
	//printf("the process is %ld\n",j/1000000);
	//fseek(raw_file,positionarray[j]*sizeof(float),SEEK_SET);
	//fread(ts,index->settings->ts_byte_size,1,raw_file);
	//memcpy(&rawfile[j*index->settings->timeseries_size],ts, sizeof(ts_type)* index->settings->timeseries_size);
	//oppositionarray[positionarray[j]/256]=j;
    }	




	//printf("the isax cashe size is %ld\n",index->sax_cache_size);
    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
		
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
                    memcpy(qts,paa,sizeof(float)*16);
        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
        query_result result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,qts,gqts,gsaxarray, posbitmap,gposbitmap,gdictionary,oppositionarray,offsetarray);
        //COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        PRINT_STATS(result.distance)
        
        fflush(stdout);
    #if VERBOSE_LEVEL >= 1
        printf("[%p]: Distance: %lf\n", result.node, result.distance);
    #endif
        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}

        q_loaded++;
	}
    free(paa);
    free(ts);
	fclose(ifile);
	fprintf(stderr, ">>> Finished querying.\n");

}
 void isax_query_binary_file_gpufloatgray(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,float*,float*,float*,node_list*,unsigned long int*)) 
{
	fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);
	FILE * ifile;
	ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    unsigned long long sz = (unsigned long long) ftell(ifile);
    unsigned long long total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
	int q_loaded = 0;
    ts_type * ts = (ts_type *)malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = (ts_type *)malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    float *dictionary =NULL,*gdictionary = NULL;
    float *gposbitmap = NULL;
    float *posbitmap = NULL;
    float *qts = NULL,*gqts = NULL;
    sax_type *saxarray=NULL,*gsaxarray=NULL;
    unsigned long int currentpositon=0;
    unsigned long int *offsetarray= (unsigned long int*)malloc(sizeof(unsigned long int)*65537);
    float *saxpoint=&sax_breakpoints[0];
	sax_type *saxarrarysort=(sax_type*)malloc(sizeof(sax_type)*index->sax_cache_size*index->settings->paa_segments);

	node_list nodelist;
    nodelist.nlist=(isax_node**)malloc(sizeof(isax_node*)*pow(2, index->settings->paa_segments));
    nodelist.node_amount=0;
    //isax_node *current_root_node = index->first_node;
    //isax_node *current_root_node = index->first_node;
    for (int i = 0; i < 65536; i++)
    {
// assign Gray = (Bin >> 1) ^ Bin;
    int binaryi=i;
    int grayi=(binaryi>>1)^binaryi;

        fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[grayi];

        if(current_buffer->initialized==1)
        {
            //offsetarray[nodelist.node_amount]=currentpositon;
            nodelist.nlist[nodelist.node_amount]=current_buffer->node;
            nodelist.node_amount++;
            //currentpositon=currentpositon+current_buffer->max_buffer_size;

        }

    }



	saxarray=initialsaxarray(gsaxarray,index->sax_cache_size);
	dictionary=(float*)malloc(257*sizeof(float)); 
	posbitmap=initialposbitmapfloat(posbitmap,index->sax_cache_size);
	qts=(float*)malloc( sizeof(float)*index->settings->paa_segments); 
	initialdevice();
	gqts=initialgqts(gqts);
    	
	gdictionary=initialgdictionary(gdictionary);
	gposbitmap=initialgposbitmapfloat(gposbitmap,index->sax_cache_size);
	gsaxarray=initialgsaxarray(gsaxarray,index->sax_cache_size);
		//printf("the isax cashe size is %ld\n",index->sax_cache_size);

unsigned long int currentposition=0;
    for(int i =0;i<nodelist.node_amount;i++)
	{
offsetarray[i]=currentposition;
            pass_tree_node_m(nodelist.nlist[i],index,NULL,&currentposition,index->sax_cache,saxarrarysort,posbitmap);
        
    }
offsetarray[nodelist.node_amount]=index->sax_cache_size;

    gpumemcpy(gsaxarray,saxarrarysort,saxpoint,gdictionary,index->sax_cache_size);

    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
		
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
                    memcpy(qts,paa,sizeof(float)*16);
        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
        query_result result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,qts,gqts,gsaxarray, posbitmap,gposbitmap,gdictionary,&nodelist,offsetarray);
        //COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        //PRINT_STATS(result.distance)
        
        fflush(stdout);
    #if VERBOSE_LEVEL >= 1
        printf("[%p]: Distance: %lf\n", result.node, result.distance);
    #endif
        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}

        q_loaded++;
	}
    free(paa);
    free(ts);
	fclose(ifile);
	fprintf(stderr, ">>> Finished querying.\n");

}

query_result exact_search_serial_ParGIS_openmp_inmemoryhybrid2(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,bool *positionmap,bool *gpositionmap,float *gdictionary) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    RDcalculationnumber=0;

    pthread_barrier_t lock_barrier1;
    pthread_barrier_init(&lock_barrier1, NULL, 2);

    pqueue_bsf *bsfqueue=pqueue_bsf_init(maxquerythread+1);
    pthread_t threadid;
    COUNT_INPUT_TIME_START
   // bool *rdcbitmap=(bool*)malloc(sizeof(bool) * index->sax_cache_size);
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    ts_type *ts_buffer = (ts_type*)malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER;
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    //printf("the old distance is: %f \n",approximate_result.distance);
    #pragma omp parallel for num_threads(maxquerythread)
    for(int i =0;i<65536;i++)
	{
		fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];
        if(current_buffer->initialized==1)
        {
            insert_tree_node_mgpu(paa,current_buffer->node,index, approximate_result.distance,bsfqueue,&lock_queue);
        }
	}

	    //#pragma omp parallel for num_threads(maxquerythread)
    //for(int i =1;i<maxquerythread+1;i++)
    //{
       // float distancenode=calculate_node_distance2_inmemory (index, bsfqueue->node[i], ts,paa, approximate_result.distance);

//if(distancenode<approximate_result.distance)
//approximate_result.distance=distancenode;

   // }
//


    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=approximate_result.distance;

    //LBDcalculationnumber=index->sax_cache_size;
    int loopnumber=10;
    // LBDstreamGPU(gsaxarray, positionmap, paa, gqts, bsf_distance,index->sax_cache_size,gpositionmap);
    GPUtransferdata gputransferdata1;
    gputransferdata1.bsf_distance=&bsf_distance;
    gputransferdata1.gsaxarray=gsaxarray;
    gputransferdata1.positionmap=positionmap;
    gputransferdata1.paa=paa;
    gputransferdata1.gqts=gqts;
    gputransferdata1.datasize=index->sax_cache_size;
    gputransferdata1.gpositionmap=gpositionmap;
    gputransferdata1.lock_barrier1=&lock_barrier1;
    gputransferdata1.loopnumber=loopnumber;
    gputransferdata1.startloop=0;
	gputransferdata1.stoploop=loopnumber;
    gputransferdata1.offsetnumber=0;

	
    pthread_create(&threadid,NULL,PplusGworker,(void*)(&gputransferdata1));
    for (int  i = 0; i < loopnumber; i++)
    {
            pthread_barrier_wait(&lock_barrier1);
    COUNT_QUEUE_TIME_START
    #pragma omp parallel for num_threads(maxquerythread-1) reduction(min : bsf_distance)
    for(unsigned long j=i*index->sax_cache_size/loopnumber; j<(i+1)*index->sax_cache_size/loopnumber; j++) {
        if(positionmap[j]==true)
        {	
            ts_buffer=&rawfile[j*index->settings->timeseries_size];
            float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
            if(dist < bsf_distance) {
                bsf_distance = dist;
            }
        }

    }
        COUNT_QUEUE_TIME_END
    }
    pthread_join(threadid,NULL);

    approximate_result.distance=bsf_distance;



           // printf(" the Real distance calculation is \t%ld\t\n ",RDcalculationnumber);
    return approximate_result;
}

query_result exact_search_serial_ParGIS_openmp_inmemoryhybrid3(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,bool *positionmap,bool *gpositionmap,float *gdictionary,unsigned long long *positionarray,unsigned long int *offsetarray) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    //RDcalculationnumber=0;

    pthread_barrier_t lock_barrier1;
    pthread_barrier_init(&lock_barrier1, NULL, 2);


    pthread_t threadid;
    COUNT_INPUT_TIME_START
   // bool *rdcbitmap=(bool*)malloc(sizeof(bool) * index->sax_cache_size);
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa,index);
    ts_type *ts_buffer = (ts_type*)malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    //printf("the old distance is: %f \n",approximate_result.distance);


    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=approximate_result.distance;

int numberofbuffer=65536;
COUNT_QUEUE_TIME_START
int aaa=0,bbb=0,ccc=0,ddd=0;
for(int i=0;i<numberofbuffer;i++)
{   
    fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];
    if(current_buffer->initialized==1)
    {
        isax_node* node=(((first_buffer_layer2*)(index->fbl))->soft_buffers[i]).node;
        float distance =  minidist_paa_to_isax(paa, node->isax_values,
                                            node->isax_cardinalities,
                                            index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);
        if(distance <= bsf_distance)
        {
aaa=i;
break;


            //pqueue_bsfre_insert(pq,distance,(long int)i,NULL);
        }
    }
}

for(int i=numberofbuffer-1;i>=0;i--)
{   
    fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];
    if(current_buffer->initialized==1)
    {
        isax_node* node=(((first_buffer_layer2*)(index->fbl))->soft_buffers[i]).node;
        float distance =  minidist_paa_to_isax(paa, node->isax_values,
                                            node->isax_cardinalities,
                                            index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);
        if(distance <= bsf_distance)
        {
bbb=i;
break;


            //pqueue_bsfre_insert(pq,distance,(long int)i,NULL);
        }
    }
}
//printf("the distance is %d\n",min(bbb-aaa,numberofbuffer-ddd));

COUNT_QUEUE_TIME_END

//if(bbb>numberofbuffer-20)
//bbb=numberofbuffer-1;


    //LBDcalculationnumber=index->sax_cache_size;
int loopnumber=10;
//printf("the begin offset is %ld\n",offsetarray[10]);
//printf("the end offset is %ld\n",offsetarray[numberofbuffer-1]);
     //LBDstreamGPU2(gsaxarray, positionmap, paa, gqts, bsf_distance,offsetarray[aaa],offsetarray[bbb+1],gpositionmap,NULL);
	//LBDstreamGPU(gsaxarray, positionmap, paa, gqts, bsf_distance,offsetarray[bbb+1],gpositionmap,NULL);
   
unsigned long int datasirereal=((offsetarray[bbb+1]-offsetarray[aaa])/(index->sax_cache_size/loopnumber)+1)*(index->sax_cache_size/loopnumber);
unsigned long int offsetnumber;
int startloop,stoploop;
if( offsetarray[aaa]+datasirereal>index->sax_cache_size)
{
//printf("the begin offset is %ld\n",offsetarray[10]);
 startloop=offsetarray[aaa]/(index->sax_cache_size/loopnumber);
 stoploop=offsetarray[bbb+1]/(index->sax_cache_size/loopnumber)+1;
offsetnumber=0;

}
else
{
	offsetnumber=offsetarray[aaa];
	startloop=0;
	stoploop=datasirereal/(index->sax_cache_size/loopnumber);

}


if(stoploop>loopnumber)
stoploop=loopnumber;
 GPUtransferdata gputransferdata1;
    gputransferdata1.bsf_distance=&bsf_distance;
    gputransferdata1.gsaxarray=gsaxarray;//[offsetarray[aaa]*16];
    gputransferdata1.positionmap=positionmap;//;[offsetarray[aaa]];
    gputransferdata1.paa=paa;
    gputransferdata1.gqts=gqts;
    gputransferdata1.datasize=index->sax_cache_size;//offsetarray[bbb]-offsetarray[aaa];
    gputransferdata1.gpositionmap=gpositionmap;
    gputransferdata1.lock_barrier1=&lock_barrier1;
    gputransferdata1.loopnumber=loopnumber;
    gputransferdata1.startloop=startloop;
	gputransferdata1.stoploop=stoploop;
gputransferdata1.offsetnumber=offsetnumber;
	
    for (int  i = startloop; i < stoploop; i++)
    {
            //pthread_barrier_wait(&lock_barrier1);

   // #pragma omp parallel for num_threads(maxquerythread-1) reduction(min : bsf_distance)
   // for(unsigned long j=offsetnumber+i*index->sax_cache_size/loopnumber; j<offsetnumber+(i+1)*index->sax_cache_size/loopnumber; j++) {
      //  if(positionmap[j]==true)
        {	
            //ts_buffer=&rawfile[j*index->settings->timeseries_size];
            //float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
            //if(dist < bsf_distance) {
              //  bsf_distance = dist;
           // }
        }

   // }
       // COUNT_QUEUE_TIME_END
    }
   // pthread_join(threadid,NULL);
//




    approximate_result.distance=bsf_distance;




    return approximate_result;
}



query_result exact_search_serial_ParGIS_openmp_inmemoryfloat(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    //RDcalculationnumber=0;




    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
   // bool *rdcbitmap=(bool*)malloc(sizeof(bool) * index->sax_cache_size);
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    ts_type *ts_buffer = (ts_type*)malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    //printf("the old distance is: %f \n",approximate_result.distance);


    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=approximate_result.distance;
    COUNT_CAL_TIME_START
    //LBDcalculationnumber=index->sax_cache_size;
    LBDfloatstreamGPU(gsaxarray, positionmap, paa, gqts, bsf_distance,index->sax_cache_size,gpositionmap, gdictionary,index->settings->mindist_sqrt);

        COUNT_CAL_TIME_END
COUNT_QUEUE_TIME_START

int roundnum=8;
for(int k=0; k<roundnum; k++) {

    #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf_distance)
    for(unsigned long j=k*index->sax_cache_size/roundnum; j<(k+1)*index->sax_cache_size/roundnum; j++) 
    {
        if(positionmap[j]<bsf_distance)
        {	
            ts_buffer=&rawfile[j*index->settings->timeseries_size];
            float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
            if(dist < bsf_distance) {
                //omp_set_lock(&bsflock);
		//printf("the bsf is %f\n",bsf_distance);
                bsf_distance = dist;
            //omp_unset_lock(&bsflock);
            }
        }
    }
}
    approximate_result.distance=bsf_distance;
        COUNT_QUEUE_TIME_END

    //        printf("the number of LB distance calculation is %ld\t\t and the Real distance calculation is %ld\n ",LBDcalculationnumber,RDcalculationnumber);

    return approximate_result;
}



query_result exact_search_serial_ParGIGS_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type **gsaxarray,bool **positionmap,bool **gpositionmap,float *gdictionary,unsigned long *gridnumber) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    //RDcalculationnumber=0;




    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
   // bool *rdcbitmap=(bool*)malloc(sizeof(bool) * index->sax_cache_size);
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    ts_type *ts_buffer = (ts_type*)malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
        omp_lock_t bsflock;
    omp_init_lock(&bsflock);
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }

    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    //printf("the old distance is: %f \n",approximate_result.distance);


    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=approximate_result.distance;
    int numberofbuffer=pow(2, index->settings->paa_segments);
    pqueue_bsf* pq=pqueue_bsf_init(numberofbuffer);

int *hereticial=(int*)malloc(sizeof(int) * numberofbuffer);

	for(int i=0;i<numberofbuffer;i++)
{
	hereticial[i]=i^(i>>1);
}

//#pragma omp parallel for num_threads(maxquerythread)
    COUNT_CAL_TIME_START
int aaa=0,bbb=0,ccc=0,ddd=0;
for(int i=0;i<numberofbuffer;i++)
{   
    fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[hereticial[i]];
    if(current_buffer->initialized==1)
    {
        isax_node* node=(((first_buffer_layer2*)(index->fbl))->soft_buffers[hereticial[i]]).node;
        float distance =  minidist_paa_to_isax(paa, node->isax_values,
                                            node->isax_cardinalities,
                                            index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);
	ccc++;
        if(distance <= bsf_distance)
        {
	if(ccc>ddd)
	{ddd=ccc;}
	ccc=0;
		if(aaa==0)
			{aaa=i;}

bbb=i;
            //pqueue_bsfre_insert(pq,distance,(long int)i,NULL);
        }
    }
}
//printf("the distance is %d\n",min(bbb-aaa,numberofbuffer-ddd));
copyqts(paa,gqts);
#pragma omp parallel for num_threads(maxquerythread)
for (int i = 0; i < pq->nowk; i++)
{
	//SIMSlowerGPUsmall(gsaxarray[pq->position[i]], positionmap[pq->position[i]],paa,gqts,bsf_distance,gridnumber[pq->position[i]],gpositionmap[pq->position[i]],gdictionary);

}
 GPUsyn();

	//SIMSlowerGPUgridstream(gsaxarray, positionmap, paa, gqts,bsf_distance,pq->nowk,gpositionmap,gdictionary,pq->position,  gridnumber);
        COUNT_CAL_TIME_END



	
	int kkkkk=0;		
		

			//if(pointerboo[0]==true)
		//printf("this is true!!!!\n");

COUNT_QUEUE_TIME_START
for (int i = 0; i < 0; i++)
    {   
        if(pq->knn[i]<bsf_distance)
        {
            fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[pq->position[i]];
		bool *pointerboo=positionmap[pq->position[i]];
            #pragma omp parallel for num_threads(maxquerythread-1) reduction(min : bsf_distance)
            for(unsigned long  j=0; j<current_buffer->max_buffer_size; j++)
            {	
                if(pointerboo[j]==true) {
                    ts_buffer=&rawfile[current_buffer->pos_records[j]];
                    float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
                   if(dist < bsf_distance) 
                   {
                        bsf_distance = dist;
                    }                                            
                }
            }

        }
    }
COUNT_QUEUE_TIME_END
    approximate_result.distance=bsf_distance;
    return approximate_result;
}
query_result exact_search_serial_ParGIGS2_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type **gsaxarray,bool **positionmap,bool **gpositionmap,float *gdictionary,unsigned long *gridnumber) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    //RDcalculationnumber=0;




    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
   // bool *rdcbitmap=(bool*)malloc(sizeof(bool) * index->sax_cache_size);
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    ts_type *ts_buffer = (ts_type*)malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
        omp_lock_t bsflock;
    omp_init_lock(&bsflock);
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }

    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    //printf("the old distance is: %f \n",approximate_result.distance);


    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=approximate_result.distance;
    int numberofbuffer=pow(2, index->settings->paa_segments);
    pqueue_bsf* pq=pqueue_bsf_init(numberofbuffer);

int *hereticial=(int*)malloc(sizeof(int) * numberofbuffer);

	for(int i=0;i<numberofbuffer;i++)
{
	hereticial[i]=i^(i>>1);
}

//#pragma omp parallel for num_threads(maxquerythread)
    COUNT_CAL_TIME_START
int aaa=0,bbb=0,ccc=0,ddd=0;
for(int i=0;i<numberofbuffer;i++)
{   
    fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[hereticial[i]];
    if(current_buffer->initialized==1)
    {
        isax_node* node=(((first_buffer_layer2*)(index->fbl))->soft_buffers[hereticial[i]]).node;
        float distance =  minidist_paa_to_isax(paa, node->isax_values,
                                            node->isax_cardinalities,
                                            index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);
	ccc++;
        if(distance <= bsf_distance)
        {
	if(ccc>ddd)
	{ddd=ccc;}
	ccc=0;
		if(aaa==0)
			{aaa=i;}

bbb=i;
            //pqueue_bsfre_insert(pq,distance,(long int)i,NULL);
        }
    }
}
//printf("the distance is %d\n",min(bbb-aaa,numberofbuffer-ddd));
copyqts(paa,gqts);
#pragma omp parallel for num_threads(maxquerythread)
for (int i = 0; i < pq->nowk; i++)
{
	//SIMSlowerGPUsmall(gsaxarray[pq->position[i]], positionmap[pq->position[i]],paa,gqts,bsf_distance,gridnumber[pq->position[i]],gpositionmap[pq->position[i]],gdictionary);

}
 GPUsyn();

	//SIMSlowerGPUgridstream(gsaxarray, positionmap, paa, gqts,bsf_distance,pq->nowk,gpositionmap,gdictionary,pq->position,  gridnumber);
        COUNT_CAL_TIME_END



	
	int kkkkk=0;		
		

			//if(pointerboo[0]==true)
		//printf("this is true!!!!\n");

COUNT_QUEUE_TIME_START
for (int i = 0; i < 0; i++)
    {   
        if(pq->knn[i]<bsf_distance)
        {
            fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[pq->position[i]];
		bool *pointerboo=positionmap[pq->position[i]];
            #pragma omp parallel for num_threads(maxquerythread-1) reduction(min : bsf_distance)
            for(unsigned long  j=0; j<current_buffer->max_buffer_size; j++)
            {	
                if(pointerboo[j]==true) {
                    ts_buffer=&rawfile[current_buffer->pos_records[j]];
                    float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
                   if(dist < bsf_distance) 
                   {
                        bsf_distance = dist;
                    }                                            
                }
            }

        }
    }
COUNT_QUEUE_TIME_END
    approximate_result.distance=bsf_distance;
    return approximate_result;
}

query_result exact_search_SING_sort_new_3 (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray ) 
{   
    COUNT_CAL_TIME_START
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    COUNT_CAL_TIME_END
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    bool labelvalue=false;
    LBDcalculationnumber=0;
    RDcalculationnumber=0;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    query_result bsf_result = approximate_result;
    pqueue_t **allpq=(pqueue_t**)malloc(sizeof(pqueue_t*)*N_PQUEUE);


    int queuelabel[N_PQUEUE];
    pthread_mutex_t *ququelock= (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*N_PQUEUE);



    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);
    int numberofbuffer=65536;



    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread];
    SING_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
    int queueoffset[N_PQUEUE];
    unsigned long int gpuoffset=0;
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
                queueoffset[i]=0;
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=i%N_PQUEUE;
        workerdata[i].lbdmap=positionmap;
        workerdata[i].labelvalue=&labelvalue;
        workerdata[i].offsetvalue=queueoffset;
        workerdata[i].gpuoffset=&gpuoffset;

    }
        
COUNT_QUEUE_TIME_START
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_worker_inmemory_SING_sort_new_6,(void*)&(workerdata[i]));
    }
//COUNT_CAL_TIME_START
        for (int i = 0; i < 10; i++)
        {
            LBDfloatstreamGPU(&gsaxarray[i*index->sax_cache_size/10*16], &positionmap[i*index->sax_cache_size/10], paa, gqts, approximate_result.distance,index->sax_cache_size/10,&gpositionmap[i*index->sax_cache_size/10], gdictionary,index->settings->mindist_sqrt);
            __sync_fetch_and_add(&gpuoffset,index->sax_cache_size/10);
        }
        
	    //LBDfloatstreamGPU(gsaxarray, positionmap, paa, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap, gdictionary);
        
    
   // COUNT_CAL_TIME_END
    
	//labelvalue=true;
    //pthread_barrier_wait(&lock_barrier);
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    COUNT_QUEUE_TIME_END
    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    bsf_result=bsf_result;

    //free(rfdata);
    //printf("the number of node added is \t%ld\t\t and the number of node remove is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);
    return bsf_result;

    // Free the nodes that where not popped.

}

query_result exact_search_SING_sort_new_4 (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray ) 
{   
    
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
   // COUNT_CAL_TIME_END
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    
    bool labelvalue=false;
    LBDcalculationnumber=0;
    RDcalculationnumber=0;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    query_result bsf_result = approximate_result;
    pqueue_t **allpq=(pqueue_t**)malloc(sizeof(pqueue_t*)*N_PQUEUE);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];

    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);
    int numberofbuffer=65536;



    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread],threadid2[maxquerythread];;
    SING_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
    int queueoffset[N_PQUEUE];
    unsigned long int gpuoffset=0;
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
    }


    gap_workerdata gapworkerdata[maxquerythread];
int startnode=nodelist->node_amount, stopnode=0,nodecounter=0,nodecounter2=nodelist->node_amount-1;//, *gapstartnode,*gapstopnode
for (int i = 0; i < maxquerythread; i++)
{
    gapworkerdata[i].nodelist=nodelist->nlist;;
	gapworkerdata[i].amountnode=nodelist->node_amount;
    gapworkerdata[i].startnode=&startnode;
    gapworkerdata[i].stopnode=&stopnode;// *gapstartnode,*gapstopnode;

    gapworkerdata[i].nodecounter=&nodecounter;
    gapworkerdata[i].nodecounter2=&nodecounter2;
    gapworkerdata[i].index=index;
    gapworkerdata[i].bsf=approximate_result.distance;
	gapworkerdata[i].paa=paa;//,*paaU,*paaL,*ts,*uo,*lo;
    gapworkerdata[i].lockposition=&lock_queue;
    gapworkerdata[i].offsetarray=offsetarray;
}
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid2[i]),NULL,gapworker,(void*)&(gapworkerdata[i]));
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid2[i],NULL);
    }

//printf("the start is %d     the end is %d the start new is %d     the new is %d \n",aaa,bbb,startnode,stopnode);
int loopnumber=20;
//printf("the begin offset is %ld\n",offsetarray[10]);
//printf("the end offset is %ld\n",offsetarray[numberofbuffer-1]);
     //LBDstreamGPU2(gsaxarray, positionmap, paa, gqts, bsf_distance,offsetarray[aaa],offsetarray[bbb+1],gpositionmap,NULL);
	//LBDstreamGPU(gsaxarray, positionmap, paa, gqts, bsf_distance,offsetarray[bbb+1],gpositionmap,NULL);
   
 int datasirereal=((offsetarray[stopnode+1])/(index->sax_cache_size/loopnumber)+1);
 int datastartnumber=((offsetarray[startnode])/(index->sax_cache_size/loopnumber));
unsigned long int datasizerun;
int startloop,stoploop;
if( datasirereal>loopnumber)
{
//printf("the begin offset is %ld\n",offsetarray[10]);
datasizerun=loopnumber;

}
else
{
datasizerun=datasirereal;
}
int node_counter=startnode;







    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=stopnode+1;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=(i%N_PQUEUE);//*(N_PQUEUE/maxquerythread);
        workerdata[i].lbdmap=positionmap;
        workerdata[i].labelvalue=&labelvalue;
        workerdata[i].gpuoffset=&gpuoffset;

    }

    //LBDfloatstreamGPUdevideoneblock(gsaxarray,positionmap, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap,gdictionary,index->settings->mindist_sqrt, loopnumber, datastartnumber,streams);

COUNT_QUEUE_TIME_START
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_worker_inmemory_SING_sort_new_4,(void*)&(workerdata[i]));
    }
//COUNT_CAL_TIME_START
COUNT_CAL_TIME_START
   // cudaStream_t *streams=LBDfloatstreamGPUdevidebegin( qts, gqts,datastartnumber,datasizerun,loopnumber);
        for (int i = datastartnumber; i < datasizerun; i++)
        {
            //LBDfloatstreamGPUdevideoneblock(gsaxarray,positionmap, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap,gdictionary,index->settings->mindist_sqrt, loopnumber, i,streams);
    	    LBDfloatstreamGPU(&gsaxarray[i*index->sax_cache_size/loopnumber*16], &positionmap[i*index->sax_cache_size/loopnumber], paa, gqts, approximate_result.distance,index->sax_cache_size/loopnumber,&gpositionmap[i*index->sax_cache_size/loopnumber], gdictionary,index->settings->mindist_sqrt);
            //__sync_fetch_and_add(&gpuoffset,index->sax_cache_size/10);
            gpuoffset=(i+1)*index->sax_cache_size/loopnumber;
        }
        
	    //LBDfloatstreamGPU(gsaxarray, positionmap, paa, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap, gdictionary);
        
    
    COUNT_CAL_TIME_END
    
	//labelvalue=true;
    //pthread_barrier_wait(&lock_barrier);
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    COUNT_QUEUE_TIME_END
    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    bsf_result=bsf_result;
    //free(rfdata);
    //LBDcalculationnumber=(datasizerun-datastartnumber)*index->sax_cache_size/loopnumber;
    //printf("the number of node added is \t%ld\t\t and the number of node remove is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);
    //printf("the number of lower bound distance calculation is \t%ld\t\t and the delete node is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);

    return bsf_result;

    // Free the nodes that where not popped.

}


pqueue_bsf exact_knn_SING_sort_new_4 (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray,int k ) 
{   
        pqueue_bsf *pq_bsf= pqueue_bsf_init(k);
    approximate_topk_inmemory(ts, paa, index, pq_bsf);
   // COUNT_CAL_TIME_END
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    
    bool labelvalue=false;
    LBDcalculationnumber=0;
    RDcalculationnumber=0;
    // Early termination...

    if(pq_bsf->knn[k-1] == FLT_MAX || min_checked_leaves > 1) {
        refine_topk_answer_inmemory(ts, paa, index, pq_bsf, minimum_distance, min_checked_leaves);
    }
    pqueue_t **allpq=(pqueue_t**)malloc(sizeof(pqueue_t*)*N_PQUEUE);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];






    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread],threadid2[maxquerythread];;
    SING_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
    int queueoffset[N_PQUEUE];
    unsigned long int gpuoffset=0;
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
    }


    gap_workerdata gapworkerdata[maxquerythread];
int startnode=nodelist->node_amount, stopnode=0,nodecounter=0,nodecounter2=nodelist->node_amount-1;//, *gapstartnode,*gapstopnode
for (int i = 0; i < maxquerythread; i++)
{
    gapworkerdata[i].nodelist=nodelist->nlist;;
	gapworkerdata[i].amountnode=nodelist->node_amount;
    gapworkerdata[i].startnode=&startnode;
    gapworkerdata[i].stopnode=&stopnode;// *gapstartnode,*gapstopnode;

    gapworkerdata[i].nodecounter=&nodecounter;
    gapworkerdata[i].nodecounter2=&nodecounter2;
    gapworkerdata[i].index=index;
    gapworkerdata[i].bsf=pq_bsf->knn[pq_bsf->k-1];
	gapworkerdata[i].paa=paa;//,*paaU,*paaL,*ts,*uo,*lo;
    gapworkerdata[i].lockposition=&lock_queue;
    gapworkerdata[i].offsetarray=offsetarray;
}
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid2[i]),NULL,gapworker,(void*)&(gapworkerdata[i]));
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid2[i],NULL);
    }

//printf("the start is %d     the end is %d the start new is %d     the new is %d \n",aaa,bbb,startnode,stopnode);
int loopnumber=20;
//printf("the begin offset is %ld\n",offsetarray[10]);
//printf("the end offset is %ld\n",offsetarray[numberofbuffer-1]);
     //LBDstreamGPU2(gsaxarray, positionmap, paa, gqts, bsf_distance,offsetarray[aaa],offsetarray[bbb+1],gpositionmap,NULL);
	//LBDstreamGPU(gsaxarray, positionmap, paa, gqts, bsf_distance,offsetarray[bbb+1],gpositionmap,NULL);

int datasirereal=((offsetarray[stopnode+1])/(index->sax_cache_size/loopnumber)+1);
int datastartnumber=((offsetarray[startnode])/(index->sax_cache_size/loopnumber));
unsigned long int datasizerun;
int startloop,stoploop;
if( datasirereal>loopnumber)
{
//printf("the begin offset is %ld\n",offsetarray[10]);
datasizerun=loopnumber;

}
else
{
datasizerun=datasirereal;
}
int node_counter=startnode;







    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=stopnode+1;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=(i%N_PQUEUE);//*(N_PQUEUE/maxquerythread);
        workerdata[i].lbdmap=positionmap;
        workerdata[i].labelvalue=&labelvalue;
        workerdata[i].gpuoffset=&gpuoffset;
        workerdata[i].pq_bsf=pq_bsf;


    }

    //LBDfloatstreamGPUdevideoneblock(gsaxarray,positionmap, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap,gdictionary,index->settings->mindist_sqrt, loopnumber, datastartnumber,streams);

COUNT_QUEUE_TIME_START
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_worker_inmemory_SING_sort_new_4,(void*)&(workerdata[i]));
    }
//COUNT_CAL_TIME_START
COUNT_CAL_TIME_START
   // cudaStream_t *streams=LBDfloatstreamGPUdevidebegin( qts, gqts,datastartnumber,datasizerun,loopnumber);
        for (int i = datastartnumber; i < datasizerun; i++)
        {
            //LBDfloatstreamGPUdevideoneblock(gsaxarray,positionmap, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap,gdictionary,index->settings->mindist_sqrt, loopnumber, i,streams);
    	    LBDfloatstreamGPU(&gsaxarray[i*index->sax_cache_size/loopnumber*16], &positionmap[i*index->sax_cache_size/loopnumber], paa, gqts, pq_bsf->knn[pq_bsf->k-1],index->sax_cache_size/loopnumber,&gpositionmap[i*index->sax_cache_size/loopnumber], gdictionary,index->settings->mindist_sqrt);
            //__sync_fetch_and_add(&gpuoffset,index->sax_cache_size/10);
            gpuoffset=(i+1)*index->sax_cache_size/loopnumber;
        }
        
	    //LBDfloatstreamGPU(gsaxarray, positionmap, paa, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap, gdictionary);
        
    
    COUNT_CAL_TIME_END
    
	//labelvalue=true;
    //pthread_barrier_wait(&lock_barrier);
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    COUNT_QUEUE_TIME_END
    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    //free(rfdata);
    //LBDcalculationnumber=(datasizerun-datastartnumber)*index->sax_cache_size/loopnumber;
    //printf("the number of node added is \t%ld\t\t and the number of node remove is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);
    //printf("the number of lower bound distance calculation is \t%ld\t\t and the delete node is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);

    return *pq_bsf;

    // Free the nodes that where not popped.

}

void* exact_search_worker_inmemory_SING_sort_new_2(void *rfdata)
{   

    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance,distance;
    int calculate_node=0,calculate_node_quque=0;
    int tnumber=rand()% N_PQUEUE;
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    //COUNT_QUEUE_TIME_START

    while (1)
    {
        current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
        if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
        break;
        current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
        insert_tree_node_m_hybridpqueue(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
    }
    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    while (1)
    {
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        if(((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]==0)
        {
            n = (query_result*)pqueue_peek(((SING_workerdata*)rfdata)->allpq[startqueuenumber]);
            if (n!=NULL)
            {
                if(n->distance > bsfdisntance || n->distance > minimum_distance)
                {
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                    break;
                }
                if(n->node->buffer->arrayposition <=*((SING_workerdata*)rfdata)->gpuoffset)
                {
                    n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]);
                }
                else
                {
                    
                    ((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]++;
                            pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                    continue;
                }
                
            }
            else
            {
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                break;
            }
            

        }
        else
        {
            n = (query_result*)pqueue_peek_n(((SING_workerdata*)rfdata)->allpq[startqueuenumber],((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]+1);
            if (n!=NULL)
            {
                if(n->distance > bsfdisntance || n->distance > minimum_distance)
                {
                    ((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]++;
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                    continue;
                }

                if(n->node->buffer->arrayposition <=*((SING_workerdata*)rfdata)->gpuoffset)
                {
                    pqueue_remove_n(((SING_workerdata*)rfdata)->allpq[startqueuenumber],((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]+1);
                }
                else
                {
                    ((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]++;
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                    continue;
                }
            }
            else
            {
                ((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]=0;
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                continue;
            }
        }
        
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        

        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

             //   checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                distance = calculate_node_distance_SING(index, n->node, ts,paa, bsfdisntance);
		//else
               // distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
        free(n);
    }

    if( (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])==1)
    {
        (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        while(n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]))
        {
            free(n);
        }
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
    }

    while(1)
    {   
        int offset=rand()% N_PQUEUE;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((SING_workerdata*)rfdata)->allqueuelabel[i])==1)
            {
                finished=false;
                while(1)
                {
                    pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[i]));
                   if(((SING_workerdata*)rfdata)->offsetvalue[i]==0)
        {
            n = (query_result*)pqueue_peek(((SING_workerdata*)rfdata)->allpq[i]);
            if (n!=NULL)
            {
                if(n->distance > bsfdisntance || n->distance > minimum_distance)
                {
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    break;
                }
                if(n->node->buffer->arrayposition <=*((SING_workerdata*)rfdata)->gpuoffset)
                {
                    n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[i]);
                }
                else
                {
                    
                    ((SING_workerdata*)rfdata)->offsetvalue[i]++;
                            pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    continue;
                }
                
            }
            else
            {
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                break;
            }
            

        }
        else
        {
            n = (query_result*)pqueue_peek_n(((SING_workerdata*)rfdata)->allpq[i],((SING_workerdata*)rfdata)->offsetvalue[i]+1);
            if (n!=NULL)
            {
                if(n->distance > bsfdisntance || n->distance > minimum_distance)
                {
                    ((SING_workerdata*)rfdata)->offsetvalue[i]++;
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    continue;
                }

                if(n->node->buffer->arrayposition <=*((SING_workerdata*)rfdata)->gpuoffset)
                {
                    pqueue_remove_n(((SING_workerdata*)rfdata)->allpq[i],((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]+1);
                }
                else
                {
                    ((SING_workerdata*)rfdata)->offsetvalue[i]++;
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    continue;
                }
            }
            else
            {
                ((SING_workerdata*)rfdata)->offsetvalue[i]=0;
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                continue;
            }
        }
                //__sync_fetch_and_add(&RDcalculationnumber,1);
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    if(n==NULL)
                    break;
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                distance = calculate_node_distance_SING(index, n->node, ts,paa, bsfdisntance);
		//else
                //distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                free(n);
                }

            }
        }
        if (finished)
        {
            break;
        }
    }
}


void* exact_search_worker_inmemory_SING_sort_new_3(void *rfdata)
{   

    struct timeval workertimestart;
    struct timeval writetiemstart;
    struct timeval workercurenttime;
    struct timeval writecurenttime;
    double worker_total_time,write_total_time;
    
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance,distance;
    int calculate_node=0,calculate_node_quque=0;
    int tnumber=rand()% N_PQUEUE;
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    //COUNT_QUEUE_TIME_START

    while (1)
    {

            //current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
          //  if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
           // break;




   // if ((&((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[current_root_node_number])->initialized) 
   // {
       // current_root_node = (&((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[current_root_node_number])->node;




            //current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
            current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
            if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
            break;

            current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
            insert_tree_node_m_hybridpqueue(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
            //insert_tree_node_mW(paa,current_root_node,index,bsfdisntance,pq,((SING_workerdata*)rfdata)->lock_queue);
    //}
    }

    //COUNT_QUEUE_TIME_END
    //calculate_node_quque=pq->size;

    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
    while (0)
    {
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]);
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((SING_workerdata*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

                checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                distance = calculate_node_distance_MplusG(index, n->node, ts,paa, lbdmap,bsfdisntance);
		//else
               // distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
        free(n);
    }

    if(0)
    {
        (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        while(n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]))
        {
            free(n);
        }
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
    }

    while(1)
    {   
        int offset=rand()% N_PQUEUE;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((SING_workerdata*)rfdata)->allqueuelabel[(i+offset)%N_PQUEUE])==1)
            {
                finished=false;
                while(1)
                {
                    
                    pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));
                    n = (query_result*)pqueue_peek(((SING_workerdata*)rfdata)->allpq[(i+offset)%N_PQUEUE]);
                    if(n==NULL)
                    {
                        (((SING_workerdata*)rfdata)->allqueuelabel[(i+offset)%N_PQUEUE])=0;
                        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));
                        break;
                    }
                    else if(n->node->buffer->arrayposition > *((SING_workerdata*)rfdata)->gpuoffset)
                    {
                        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));
                        break;
                    }
                    n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[(i+offset)%N_PQUEUE]);
                //__sync_fetch_and_add(&RDcalculationnumber,1);
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));

                    
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        (((SING_workerdata*)rfdata)->allqueuelabel[(i+offset)%N_PQUEUE])=0;
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
           // gettimeofday(&workertimestart, NULL);
                
                distance = calculate_node_distance_SING(index, n->node, ts,paa,bsfdisntance);
                 //gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
                //distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                //add
                free(n);
                }

            }
        }

        if (finished)
        {
            break;
        }
    }


    //pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    //while(n=pqueue_pop(pq))
    //{
            //free(n);
    //}
    //pqueue_free(pq);
    //
    

                                    //  printf("distance calculation time is is \t %f \n",worker_total_time );
    //printf("the check's node is\t %d\tthe local queue's node is\t%d\n",checks,calculate_node_quque);
}


void* exact_search_worker_inmemory_SING_sort_new_4(void *rfdata)
{   

    struct timeval workertimestart;
    struct timeval writetiemstart;
    struct timeval workercurenttime;
    struct timeval writecurenttime;
    double worker_total_time,write_total_time;
    
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance,distance;
    int calculate_node=0,calculate_node_quque=0;
    
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    int tnumber=startqueuenumber;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    int localqueuecounter=0;
    //COUNT_QUEUE_TIME_START
    pthread_mutex_t *alllock=((SING_workerdata*)rfdata)->alllock;
    while (1)
    {
        current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
        if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
            break;
        current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
        insert_tree_node_m_hybridpqueueBreakpoly(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
    }
    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);

int queuenumber;
int offset=startqueuenumber;
    while(1)
    { 
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            queuenumber=(i+offset)%N_PQUEUE;
            if((((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])==1)
            {
                finished=false;
                bsfdisntance=bsf_result->distance;
                pthread_mutex_lock(&alllock[queuenumber]);
                n = (query_result*)pqueue_peek(((SING_workerdata*)rfdata)->allpq[queuenumber]);
                if(n==NULL)
                {
                    (((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])=0;
                    pthread_mutex_unlock(&alllock[queuenumber]);
                    continue;
                }
                else if(n->node->buffer->arrayposition > *((SING_workerdata*)rfdata)->gpuoffset)
                {
                    pthread_mutex_unlock(&alllock[queuenumber]);
                    continue;
                }
                else 
                {
                    n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[queuenumber]);
                           //     __sync_fetch_and_add(&RDcalculationnumber,1);

                    pthread_mutex_unlock(&alllock[queuenumber]);                    
                }
           // __sync_fetch_and_add(&RDcalculationnumber,1);
                
                if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                    (((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])=0;
                    continue;
                }
                else 
                {
                    // If it is a leaf, check its real distance.
                    if (n->node->is_leaf) 
                    {
                        checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
            //gettimeofday(&workertimestart, NULL);
                
                    distance = calculate_node_distance_SING(index, n->node, ts,paa,bsfdisntance);
               // gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
                //distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                        if (distance < bsfdisntance)
                        {
                            pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                            if(distance < bsf_result->distance)
                            {
                                bsf_result->distance = distance;
                                bsf_result->node = n->node;
                            }
                            pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                        }
                    }
                }
                free(n);
            }
            else if((((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])==0)
            {
                pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[queuenumber]));
                n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[queuenumber]);
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[queuenumber]));                    
                if(n!=NULL)
                {
                    free(n);
                }
                else
                {
                    (((SING_workerdata*)rfdata)->allqueuelabel[queuenumber])=2;
                }
                
            }
            
        }

        if (finished)
        {
            break;
        }
    }


    //pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    //while(n=pqueue_pop(pq))
    //{
            //free(n);
    //}
    //pqueue_free(pq);
    //
    

                        //            printf("distance calculation time is is \t %f \n",worker_total_time );
    //printf("the check's node is\t %d\tthe local queue's node is\t%d\n",checks,calculate_node_quque);
}
void* exact_search_worker_inmemory_SING_sort_new_5(void *rfdata)
{   

    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance,distance;
    int calculate_node=0,calculate_node_quque=0;
    int tnumber=rand()% N_PQUEUE;
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    //COUNT_QUEUE_TIME_START

    while (1)
    {
        current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
        if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
        break;
        current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
        insert_tree_node_m_hybridpqueue(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
    }
    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    while (0)
    {
                bsfdisntance=bsf_result->distance;
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        if(((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]==0)
        {
            n = (query_result*)pqueue_peek(((SING_workerdata*)rfdata)->allpq[startqueuenumber]);
            if (n!=NULL)
            {
                if(n->distance > bsfdisntance || n->distance > minimum_distance)
                {
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                    break;
                }
                if(n->node->buffer->arrayposition <=*((SING_workerdata*)rfdata)->gpuoffset)
                {
                    n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]);
                }
                else
                {
                    
                    ((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]++;
                            pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                    continue;
                }
                
            }
            else
            {
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                break;
            }
            

        }
        else
        {
            n = (query_result*)pqueue_peek_n(((SING_workerdata*)rfdata)->allpq[startqueuenumber],((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]+1);
            if (n!=NULL)
            {
                if(n->distance > bsfdisntance || n->distance > minimum_distance)
                {
                    ((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]++;
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                    continue;
                }

                if(n->node->buffer->arrayposition <=*((SING_workerdata*)rfdata)->gpuoffset)
                {
                    ((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]=0;
                    pqueue_remove_n(((SING_workerdata*)rfdata)->allpq[startqueuenumber],((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]+1);
                }
                else
                {
                    ((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]++;
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                    continue;
                }
            }
            else
            {
                ((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]=0;
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
                continue;
            }
        }
        
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

             //   checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                distance = calculate_node_distance_SING(index, n->node, ts,paa, bsfdisntance);
		//else
               // distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
        free(n);
    }

    if( (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])==1)
    {
        (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        while(n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]))
        {
            free(n);
        }
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
    }

    while(1)
    {   
        int offset=rand()% N_PQUEUE;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((SING_workerdata*)rfdata)->allqueuelabel[i])==1)
            {
                finished=false;
                while(1)
                {
                    pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[i]));
                   if(((SING_workerdata*)rfdata)->offsetvalue[i]==0)
        {
            n = (query_result*)pqueue_peek(((SING_workerdata*)rfdata)->allpq[i]);
            if (n!=NULL)
            {
                if(n->distance > bsfdisntance || n->distance > minimum_distance)
                {
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    break;
                }
                if(n->node->buffer->arrayposition <=*((SING_workerdata*)rfdata)->gpuoffset)
                {
                    n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[i]);
                }
                else
                {
                    
                    ((SING_workerdata*)rfdata)->offsetvalue[i]++;
                            pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    continue;
                }
                
            }
            else
            {
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                break;
            }
            

        }
        else
        {
            n = (query_result*)pqueue_peek_n(((SING_workerdata*)rfdata)->allpq[i],((SING_workerdata*)rfdata)->offsetvalue[i]+1);
            if (n!=NULL)
            {
                if(n->distance > bsfdisntance || n->distance > minimum_distance)
                {
                    ((SING_workerdata*)rfdata)->offsetvalue[i]++;
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    continue;
                }

                if(n->node->buffer->arrayposition <=*((SING_workerdata*)rfdata)->gpuoffset)
                {
                    pqueue_remove_n(((SING_workerdata*)rfdata)->allpq[i],((SING_workerdata*)rfdata)->offsetvalue[startqueuenumber]+1);
                }
                else
                {
                    ((SING_workerdata*)rfdata)->offsetvalue[i]++;
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    continue;
                }
            }
            else
            {
                ((SING_workerdata*)rfdata)->offsetvalue[i]=0;
                pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                continue;
            }
        }
                //__sync_fetch_and_add(&RDcalculationnumber,1);
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    if(n==NULL)
                    break;
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                distance = calculate_node_distance_SING(index, n->node, ts,paa, bsfdisntance);
		//else
                //distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                free(n);
                }

            }
        }
        if (finished)
        {
            break;
        }
    }
}



void* exact_search_worker_inmemory_SING_sort_new_6(void *rfdata)
{   
    //the worker of SING sort new 2 with 10 nodes for each checking 
    struct timeval workertimestart;
    struct timeval writetiemstart;
    struct timeval workercurenttime;
    struct timeval writecurenttime;
    double worker_total_time,write_total_time;
    
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance,distance;
    int calculate_node=0,calculate_node_quque=0;
    
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    int tnumber=rand()% N_PQUEUE;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    int localqueuecounter=0;
    int maxnode_perqueue=3;
    //COUNT_QUEUE_TIME_START

    while (1)
    {

            //current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
          //  if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
           // break;




   // if ((&((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[current_root_node_number])->initialized) 
   // {
       // current_root_node = (&((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[current_root_node_number])->node;




            //current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
            current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
            if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
            break;

            current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
            insert_tree_node_m_hybridpqueueBreakpoly(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
            //insert_tree_node_mW(paa,current_root_node,index,bsfdisntance,pq,((SING_workerdata*)rfdata)->lock_queue);
    //}
    }

    //COUNT_QUEUE_TIME_END
    //calculate_node_quque=pq->size;

    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
    while (0)
    {
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]);
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((SING_workerdata*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

                checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                distance = calculate_node_distance_MplusG(index, n->node, ts,paa, lbdmap,bsfdisntance);
		//else
               // distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
        free(n);
    }

    if(0)
    {
        (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        while(n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]))
        {
            free(n);
        }
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
    }
    while(1)
    {   
        int offset=startqueuenumber;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((SING_workerdata*)rfdata)->allqueuelabel[(i+offset)%N_PQUEUE])==1)
            {
                finished=false;
                bsfdisntance=bsf_result->distance;
                localqueuecounter=0;
                for(int j=0;j<maxnode_perqueue;j++)
                {
                    pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));
                    if(localqueuecounter==0)
                    {
                        n = (query_result*)pqueue_peek(((SING_workerdata*)rfdata)->allpq[(i+offset)%N_PQUEUE]);
                        if(n==NULL)
                        {
                            (((SING_workerdata*)rfdata)->allqueuelabel[(i+offset)%N_PQUEUE])=0;
                            pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));
                            continue;
                        }
                        else if(n->node->buffer->arrayposition > *((SING_workerdata*)rfdata)->gpuoffset)
                        {
                            pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));
                            localqueuecounter++;
                            continue;
                        }
                        n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[(i+offset)%N_PQUEUE]);
                    }
                    else
                    {
                        n = (query_result*)pqueue_peek_n(((SING_workerdata*)rfdata)->allpq[(i+offset)%N_PQUEUE],localqueuecounter+1);
                        if(n==NULL)
                        {
                            (((SING_workerdata*)rfdata)->allqueuelabel[(i+offset)%N_PQUEUE])=0;
                            pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));
                            localqueuecounter=0;
                            continue;
                        }
                        else if(n->node->buffer->arrayposition > *((SING_workerdata*)rfdata)->gpuoffset)
                        {
                            pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));
                            localqueuecounter++;
                            continue;
                        }
                            pqueue_remove(((SING_workerdata*)rfdata)->allpq[(i+offset)%N_PQUEUE],n);
                            localqueuecounter=0;
                    }
               // __sync_fetch_and_add(&RDcalculationnumber,1);
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[(i+offset)%N_PQUEUE]));
                    
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        (((SING_workerdata*)rfdata)->allqueuelabel[(i+offset)%N_PQUEUE])=0;
                        continue;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
            //gettimeofday(&workertimestart, NULL);
                
                distance = calculate_node_distance_SING(index, n->node, ts,paa,bsfdisntance);
                //gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
                //distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                //add
                free(n);
                }

            }
        }

        if (finished)
        {
            break;
        }
    }


    //pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    //while(n=pqueue_pop(pq))
    //{
            //free(n);
    //}
    //pqueue_free(pq);
    //
    

                                    // printf("distance calculation time is is \t %f \n",worker_total_time );
    //printf("the check's node is\t %d\tthe local queue's node is\t%d\n",checks,calculate_node_quque);
}




void* exact_search_worker_inmemory_SING_sort_new_7(void *rfdata)
{   

    struct timeval workertimestart;
    struct timeval writetiemstart;
    struct timeval workercurenttime;
    struct timeval writecurenttime;
    double worker_total_time,write_total_time;
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((SING_workerdata*)rfdata)->index;
    ts_type *paa=((SING_workerdata*)rfdata)->paa;
    ts_type *ts=((SING_workerdata*)rfdata)->ts;
    pqueue_t *pq=((SING_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((SING_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((SING_workerdata*)rfdata)->minimum_distance;
    int limit=((SING_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((SING_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance,distance;
    int calculate_node=0,calculate_node_quque=0;
    int tnumber=rand()% N_PQUEUE;
    int startqueuenumber=((SING_workerdata*)rfdata)->startqueuenumber;
    float *lbdmap=((SING_workerdata*)rfdata)->lbdmap;
    //COUNT_QUEUE_TIME_START

    while (1)
    {

            //current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
          //  if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
           // break;




   // if ((&((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[current_root_node_number])->initialized) 
   // {
       // current_root_node = (&((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[current_root_node_number])->node;




            //current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
            current_root_node_number=__sync_fetch_and_add(((SING_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
            if(current_root_node_number>= ((SING_workerdata*)rfdata)->amountnode)
            break;

            current_root_node=((SING_workerdata*)rfdata)->nodelist[current_root_node_number];
        insert_tree_node_m_hybridpqueueBreakpoly(paa,current_root_node,index,bsfdisntance,((SING_workerdata*)rfdata)->allpq,((SING_workerdata*)rfdata)->alllock,&tnumber);
            //insert_tree_node_mW(paa,current_root_node,index,bsfdisntance,pq,((SING_workerdata*)rfdata)->lock_queue);
    //}
    }

    //COUNT_QUEUE_TIME_END
    //calculate_node_quque=pq->size;

    pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
    while (1)
    {
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]);
             //   __sync_fetch_and_add(&RDcalculationnumber,1);
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((SING_workerdata*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

                checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                       //     gettimeofday(&workertimestart, NULL);
                if(n->node->buffer->arrayposition <= *((SING_workerdata*)rfdata)->gpuoffset)
                distance = calculate_node_distance_SING(index, n->node, ts,paa,bsfdisntance);
                else
                distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                 //gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
               // distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
        free(n);
    }

    if( (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])==1)
    {
        (((SING_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
        pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
        while(n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[startqueuenumber]))
        {
            free(n);
        }
        pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[startqueuenumber]));
    }

    while(1)
    {   
        int offset=rand()% N_PQUEUE;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((SING_workerdata*)rfdata)->allqueuelabel[i])==1)
            {
                finished=false;
                while(1)
                {
                    pthread_mutex_lock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    n = (query_result*)pqueue_pop(((SING_workerdata*)rfdata)->allpq[i]);
                //__sync_fetch_and_add(&RDcalculationnumber,1);
                    pthread_mutex_unlock(&(((SING_workerdata*)rfdata)->alllock[i]));
                    if(n==NULL)
                    break;
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
		//if(*(((SING_workerdata*)rfdata)->labelvalue))
                           // gettimeofday(&workertimestart, NULL);
                
                if(n->node->buffer->arrayposition <= *((SING_workerdata*)rfdata)->gpuoffset)
                distance = calculate_node_distance_SING(index, n->node, ts,paa,bsfdisntance);
                else
                distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                // gettimeofday(&workercurenttime, NULL); \
                                      tS = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); \
                                      tE = workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec); \
                                      worker_total_time += (tE - tS); 
		//else
                //distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((SING_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((SING_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                //add
                free(n);
                }

            }
        }
        if (finished)
        {
            break;
        }
    }


    //pthread_barrier_wait(((SING_workerdata*)rfdata)->lock_barrier);
    //while(n=pqueue_pop(pq))
    //{
            //free(n);
    //}
    //pqueue_free(pq);
    //
    

                                //    printf("node calculation time is time is \t %f \n",worker_total_time );
    //printf("the check's node is\t %d\tthe local queue's node is\t%d\n",checks,calculate_node_quque);
}
void* twogapworker( void *gapworkerdata)
{
    
    int   amountnode=((gap_workerdata*)gapworkerdata)->amountnode;
    isax_node **nodelist=((gap_workerdata*)gapworkerdata)->nodelist;
    isax_index *index=((gap_workerdata*)gapworkerdata)->index;
    ts_type *paa=((gap_workerdata*)gapworkerdata)->paa;
    float bsf=((gap_workerdata*)gapworkerdata)->bsf;
    unsigned long* offsetarray=((gap_workerdata*)gapworkerdata)->offsetarray;
    bool *activechunk=((gap_workerdata*)gapworkerdata)->activechunk;
    int current_root_node_number;
    int chunknumber=((gap_workerdata*)gapworkerdata)->chunknumber;
    while(1)
    {
        current_root_node_number=__sync_fetch_and_add(((gap_workerdata*)gapworkerdata)->nodecounter,1);

        if(current_root_node_number>=amountnode)
        break;

        isax_node* node=nodelist[current_root_node_number];
        float distance =  nodedistance(paa, node->isax_values,
                                            node->isax_cardinalities,
                                            index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt); 
            if(distance<bsf)
            {
                activechunk[(int)(offsetarray[current_root_node_number]/(index->sax_cache_size/chunknumber))]=true;
                activechunk[(int)(offsetarray[current_root_node_number+1]/(index->sax_cache_size/chunknumber))]=true;
            }
    }    
}

query_result exact_search_ParISnew_inmemory_hybridgplus (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves) 
{   
     //   RDcalculationnumber=0;
    //LBDcalculationnumber=0;
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    //query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory_m(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    pqueue_t **allpq=(pqueue_t **)malloc(sizeof(pqueue_t*)*N_PQUEUE);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];

    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);


    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread];
    MESSI_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);

    
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=i%N_PQUEUE;
    }
        
    
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_worker_inmemory_hybridpqueuegplus,(void*)&(workerdata[i]));
    }
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }

    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    bsf_result=bsf_result;

    //free(rfdata);
     //       printf("the number of LB distance calculation is %ld\t\t and the Real distance calculation is %ld\n ",LBDcalculationnumber,RDcalculationnumber);
    return bsf_result;

    // Free the nodes that where not popped.

}



void isax_query_binary_file_traditionalgplus(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*,node_list*, float, int)) 
{
    fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);
    FILE * ifile;
    ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    int q_loaded = 0; 
    ts_type * ts = (ts_type*)malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = (ts_type*)malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    node_list nodelist;
    nodelist.nlist=(isax_node**)malloc(sizeof(isax_node*)*pow(2, index->settings->paa_segments));
    nodelist.node_amount=0;
    isax_node *current_root_node = index->first_node;
    //isax_node *current_root_node = index->first_node;
    while(1)
    {
        if (current_root_node!=NULL)
        {
            nodelist.nlist[nodelist.node_amount]=current_root_node;
            current_root_node=current_root_node->next;
            nodelist.node_amount++;
        }
        else
        {
            break;
        }
                    
    }
    //printf("the node node_amount is %d\n",nodelist.node_amount );
                
    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
        query_result result = search_function(ts, paa, index,&nodelist, minimum_distance, min_checked_leaves);
        COUNT_TOTAL_TIME_END
        PRINT_STATS(result.distance)
        
        fflush(stdout);
    #if VERBOSE_LEVEL >= 1
        printf("[%p]: Distance: %lf\n", result.node, result.distance);
    #endif
        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}

        q_loaded++;
    }
    free(nodelist.nlist);
    free(paa);
    free(ts);
    fclose(ifile);
    fprintf(stderr, ">>> Finished querying.\n");

}
void* exact_search_worker_inmemory_hybridpqueuegplus(void *rfdata)
{   

    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((MESSI_workerdata*)rfdata)->index;
    ts_type *paa=((MESSI_workerdata*)rfdata)->paa;
    ts_type *ts=((MESSI_workerdata*)rfdata)->ts;
    pqueue_t *pq=((MESSI_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((MESSI_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((MESSI_workerdata*)rfdata)->minimum_distance;
    int limit=((MESSI_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((MESSI_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance;
    int calculate_node=0,calculate_node_quque=0;
    int tnumber=rand()% N_PQUEUE;
    int startqueuenumber=((MESSI_workerdata*)rfdata)->startqueuenumber;
    //COUNT_QUEUE_TIME_START

    while (1) 
    {
            current_root_node_number=__sync_fetch_and_add(((MESSI_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
            if(current_root_node_number>= ((MESSI_workerdata*)rfdata)->amountnode)
            break;
            current_root_node=((MESSI_workerdata*)rfdata)->nodelist[current_root_node_number];

            insert_tree_node_m_hybridpqueue(paa,current_root_node,index,bsfdisntance,((MESSI_workerdata*)rfdata)->allpq,((MESSI_workerdata*)rfdata)->alllock,&tnumber);
            //insert_tree_node_mW(paa,current_root_node,index,bsfdisntance,pq,((MESSI_workerdata*)rfdata)->lock_queue);

            
    }

    //COUNT_QUEUE_TIME_END
    //calculate_node_quque=pq->size;

    pthread_barrier_wait(((MESSI_workerdata*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
    while (1)
    {
        pthread_mutex_lock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
        n = (query_result*)pqueue_pop(((MESSI_workerdata*)rfdata)->allpq[startqueuenumber]);
        pthread_mutex_unlock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((MESSI_workerdata*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

                checks++;

                float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
        free(n);
    }

    if( (((MESSI_workerdata*)rfdata)->allqueuelabel[startqueuenumber])==1)
    {
        (((MESSI_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
        pthread_mutex_lock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
        while(n = (query_result*)pqueue_pop(((MESSI_workerdata*)rfdata)->allpq[startqueuenumber]))
        {
            free(n);
        }
        pthread_mutex_unlock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
    }

    while(1)
    {   
        int offset=rand()% N_PQUEUE;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((MESSI_workerdata*)rfdata)->allqueuelabel[i])==1)
            {
                finished=false;
                while(1)
                {
                    pthread_mutex_lock(&(((MESSI_workerdata*)rfdata)->alllock[i]));
                    n = (query_result*)pqueue_pop(((MESSI_workerdata*)rfdata)->allpq[i]);
                    pthread_mutex_unlock(&(((MESSI_workerdata*)rfdata)->alllock[i]));
                    if(n==NULL)
                    break;
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
                            float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                //add
                free(n);
                }

            }
        }
        if (finished)
        {
            break;
        }
    }


    //pthread_barrier_wait(((MESSI_workerdata*)rfdata)->lock_barrier);
    //while(n=pqueue_pop(pq))
    //{
            //free(n);
    //}
    //pqueue_free(pq);
    //
    

                                      //printf("create pq time is %f \n",worker_total_time );
    //printf("the check's node is\t %d\tthe local queue's node is\t%d\n",checks,calculate_node_quque);
}
query_result  approximate_search_inmemory_pargis(ts_type *ts, ts_type *paa, unsigned long long *positionmap,isax_index *index) 
{
    query_result result;

    sax_type *sax = (sax_type*)malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(paa, sax, index->settings->paa_segments,
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);

    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);


    if ((&((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) root_mask])->initialized) {
        isax_node *node = (&((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) root_mask])->node;
        // Traverse tree

        // Adaptive splitting

        while (!node->is_leaf) {
            int location = index->settings->sax_bit_cardinality - 1 -
            node->split_data->split_mask[node->split_data->splitpoint];
            root_mask_type mask = index->settings->bit_masks[location];

            if(sax[node->split_data->splitpoint] & mask)
            {
                node = node->right_child;
            }
            else
            {
                node = node->left_child;
            }

            // Adaptive splitting
        }
        result.distance = calculate_node_distance_inmemory_pargis(index, node, ts,positionmap, FLT_MAX);
        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);

    return result;
}


float calculate_node_distance_inmemory_pargis (isax_index *index, isax_node *node, ts_type *query,unsigned long long *positionmap, float bsf) 
{
    COUNT_CHECKED_NODE()
    // If node has buffered data

    if (node->buffer != NULL) 
    {   
        int i;
        for (i=0; i<node->buffer->full_buffer_size; i++) 
        {
            float dist = ts_euclidean_distance(query, node->buffer->full_ts_buffer[i], 
                                               index->settings->timeseries_size, bsf);
            if (dist < bsf) {
                bsf = dist;
            }
        }

        for (i=0; i<node->buffer->tmp_full_buffer_size; i++) {
            float dist = ts_euclidean_distance(query, node->buffer->tmp_full_ts_buffer[i], 
                                               index->settings->timeseries_size, bsf);
            if (dist < bsf ) {
                bsf = dist;
            }
        }
       // RDcalculationnumber=RDcalculationnumber+node->buffer->partial_buffer_size;
        for (i=0; i<node->buffer->partial_buffer_size; i++) {

            float dist = ts_euclidean_distance_SIMD(query, &(rawfile[positionmap[*node->buffer->partial_position_buffer[i]/256]*256]), 
                                               index->settings->timeseries_size, bsf);

            if (dist < bsf) {
                bsf = dist;

            }
        }
    }
    
    return bsf;
}


query_result exact_search_SING_sort_pruned (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray ) 
{   
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    LBDcalculationnumber=0;
    RDcalculationnumber=0;

    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    bool labelvalue=false;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    query_result bsf_result = approximate_result;
    pqueue_t **allpq=(pqueue_t**)malloc(sizeof(pqueue_t*)*N_PQUEUE);

    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];

    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);
    int numberofbuffer=65536;
    int aaa=0,bbb=0;
        COUNT_CAL_TIME_START

    pthread_t threadid[maxquerythread],threadid2[maxquerythread];
    gap_workerdata gapworkerdata[maxquerythread];
int startnode=nodelist->node_amount, stopnode=0,nodecounter=0,nodecounter2=nodelist->node_amount-1;//, *gapstartnode,*gapstopnode
for (int i = 0; i < maxquerythread; i++)
{
    gapworkerdata[i].nodelist=nodelist->nlist;;
	gapworkerdata[i].amountnode=nodelist->node_amount;
    gapworkerdata[i].startnode=&startnode;
    gapworkerdata[i].stopnode=&stopnode;// *gapstartnode,*gapstopnode;

    gapworkerdata[i].nodecounter=&nodecounter;
    gapworkerdata[i].nodecounter2=&nodecounter2;
    gapworkerdata[i].index=index;
    gapworkerdata[i].bsf=approximate_result.distance;
	gapworkerdata[i].paa=paa;//,*paaU,*paaL,*ts,*uo,*lo;
    gapworkerdata[i].lockposition=&lock_queue;
}
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid2[i]),NULL,gapworker,(void*)&(gapworkerdata[i]));
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid2[i],NULL);
    }

//printf("the start is %d     the end is %d the start new is %d     the new is %d \n",aaa,bbb,startnode,stopnode);
int loopnumber=32;
//printf("the begin offset is %ld\n",offsetarray[10]);
//printf("the end offset is %ld\n",offsetarray[numberofbuffer-1]);
     //LBDstreamGPU2(gsaxarray, positionmap, paa, gqts, bsf_distance,offsetarray[aaa],offsetarray[bbb+1],gpositionmap,NULL);
	//LBDstreamGPU(gsaxarray, positionmap, paa, gqts, bsf_distance,offsetarray[bbb+1],gpositionmap,NULL);
   
 int datasirereal=((offsetarray[stopnode+1])/(index->sax_cache_size/loopnumber)+1);
 int datastartnumber=((offsetarray[startnode])/(index->sax_cache_size/loopnumber));
unsigned long int datasizerun;
int startloop,stoploop;
if( datasirereal>loopnumber)
{
//printf("the begin offset is %ld\n",offsetarray[10]);
datasizerun=loopnumber;

}
else
{
datasizerun=datasirereal;
}
COUNT_CAL_TIME_END



    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    SING_workerdata workerdata[maxquerythread];

    pthread_barrier_init(&lock_barrier, NULL, maxquerythread+1);
 
    
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;

        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=i%N_PQUEUE;
    workerdata[i].lbdmap=positionmap;
    workerdata[i].labelvalue=&labelvalue;
    }
        

    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_SING_sort_worker,(void*)&(workerdata[i]));
    }

    	    LBDfloatstreamGPUdevide(gsaxarray, positionmap, paa, gqts, approximate_result.distance,index->sax_cache_size,gpositionmap, gdictionary,datastartnumber,datasizerun,index->settings->mindist_sqrt);
	   // SIMSlowerGPUfloatdevide(gsaxarray, positionmap, paa, gqts, approximate_result.distance,datasizerun,gpositionmap, gdictionary,offsetarray[aaa],index->sax_cache_size);

	    //SIMSlowerGPUfloat(&gsaxarray[offsetarray[aaa]*16], &positionmap[offsetarray[aaa]], paa, gqts, approximate_result.distance,datasizerun,&gpositionmap[offsetarray[aaa]], gdictionary);
    
	//labelvalue=true;
    pthread_barrier_wait(&lock_barrier);
    COUNT_QUEUE_TIME_START
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    COUNT_QUEUE_TIME_END
    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    bsf_result=bsf_result;

    //free(rfdata);
         //  printf("the number of node added is \t%ld\t\t and the number of node remove is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);
    return bsf_result;

    // Free the nodes that where not popped.

}


*/