#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "immintrin.h"



void lower_upper_lemire(float *t, int len, int r, float *l, float *u)
{
    //calculate the envelop of DTW 
    //t: time series raw data
    //len: time series length
    //r: warping window size
    //l: output lower envelope
    //u: output upper envelope
    // PS: also need to transfer the 2 envelope to paa.
    struct deque du, dl;

    init(&du, 2*r+2);
    init(&dl, 2*r+2);

    push_back(&du, 0);
    push_back(&dl, 0);
    int i;
    
    for (i = 1; i < len; i++)
    {
        if (i > r)
        {
            u[i-r-1] = t[front(&du)];
            l[i-r-1] = t[front(&dl)];
        }
        if (t[i] > t[i-1])
        {
            pop_back(&du);
            while (!empty(&du) && t[i] > t[back(&du)])
                pop_back(&du);
        }
        else
        {
            pop_back(&dl);
            while (!empty(&dl) && t[i] < t[back(&dl)])
                pop_back(&dl);
        }
        push_back(&du, i);
        push_back(&dl, i);
        if (i == 2 * r + 1 + front(&du))
            pop_front(&du);
        else if (i == 2 * r + 1 + front(&dl))
            pop_front(&dl);
    }
    for (i= len; i < len+r+1; i++)
    {
        u[i-r-1] = t[front(&du)];
        l[i-r-1] = t[front(&dl)];
        if (i-front(&du) >= 2 * r + 1)
            pop_front(&du);
        if (i-front(&dl) >= 2 * r + 1)
            pop_front(&dl);
    }
    destroy(&du);
    destroy(&dl);


}



float   minidist_paa_to_isax_raw_DTW(float *paaU, float *paaL , sax_type *sax, 
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

        sax_type region_lower = (v >> (c_m - c_c)) <<  (c_m - c_c);//shift operation 
        sax_type region_upper = (~((int)MAXFLOAT << (c_m - c_c)) | region_lower);
		//printf("[%d, %d] %d -- %d\n", sax[i], c_c, region_lower, region_upper);

        float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        float breakpoint_upper = 0; // <-- - || -
        
        
        if (region_lower == 0) {
            breakpoint_lower = min_val;
        }
        else
        {
            breakpoint_lower = sax_breakpoints[offset + region_lower - 1];
        }
        if (region_upper == max_cardinality - 1) {
            breakpoint_upper = max_val;
        }
        else
        {
            breakpoint_upper = sax_breakpoints[offset + region_upper];//search in a list(why?)
        }

        //printf("\n%d.%d is from %d to %d, %lf - %lf\n", v, c_c, region_lower, region_upper,
        //       breakpoint_lower, breakpoint_upper);

        //printf("FROM: \n");
        //sax_print(&region_lower, 1, c_m);
        //printf("TO: \n");
        //sax_print(&region_upper, 1, c_m);
		
		//printf ("\n---------\n");
        
        if (breakpoint_lower > paaU[i] ) {
            distance += pow(breakpoint_lower - paaU[i], 2);
        }
        else if(breakpoint_upper < paaL[i]) {
            distance += pow(breakpoint_upper - paaL[i], 2);
        }


//        else {
//            printf("%lf is between: %lf and %lf\n", paa[i], breakpoint_lower, breakpoint_upper);
//        }
    }
    
    //distance = ratio_sqrt * sqrtf(distance);
    distance = ratio_sqrt * distance;
    return distance;
}




float   minidist_paa_to_isax_raw_DTW_SIMD(float *paaU,float *paaL, sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt)
{
   
   int region_upper[16],region_lower[16];
    float distancef[16];
    int offset = ((max_cardinality - 1) * (max_cardinality - 2)) / 2;

    __m256i vectorsignbit = _mm256_set1_epi32 (0xffffffff);
    __m256i vloweroffset = _mm256_set1_epi32 (offset-1);
    __m256i vupperoffset = _mm256_set1_epi32 (offset);

        //__m256i c_cv_0 = _mm256_set_epi32 ( sax_cardinalities[7] , sax_cardinalities[6] ,sax_cardinalities[5] ,sax_cardinalities[4] , sax_cardinalities[3] ,sax_cardinalities[2] ,sax_cardinalities[1],sax_cardinalities[0]);  
        //__m256i c_cv_1 = _mm256_set_epi32 ( sax_cardinalities[15], sax_cardinalities[14],sax_cardinalities[13],sax_cardinalities[12], sax_cardinalities[11],sax_cardinalities[10],sax_cardinalities[9],sax_cardinalities[8]);
        __m128i sax_cardinalitiesv8 = _mm_lddqu_si128 ((const void*)sax_cardinalities);
        __m256i sax_cardinalitiesv16= _mm256_cvtepu8_epi16 (sax_cardinalitiesv8);
        __m128i sax_cardinalitiesv16_0=_mm256_extractf128_si256 (sax_cardinalitiesv16,0);
        __m128i sax_cardinalitiesv16_1=_mm256_extractf128_si256 (sax_cardinalitiesv16,1);
        __m256i c_cv_0 = _mm256_cvtepu16_epi32 (sax_cardinalitiesv16_0);
        __m256i c_cv_1 = _mm256_cvtepu16_epi32 (sax_cardinalitiesv16_1);

        //__m256i v_0    = _mm256_set_epi32 (sax[7],sax[6],sax[5],sax[4],sax[3],sax[2],sax[1],sax[0]);
        //__m256i v_1    = _mm256_set_epi32 (sax[15],sax[14],sax[13],sax[12],sax[11],sax[10],sax[9],sax[8]);
        __m128i saxv8= _mm_lddqu_si128 ((const void*)sax);
        __m256i saxv16= _mm256_cvtepu8_epi16 (saxv8);
        __m128i saxv16_0 =_mm256_extractf128_si256 (saxv16,0);
        __m128i saxv16_1=_mm256_extractf128_si256 (saxv16,1);
        __m256i v_0= _mm256_cvtepu16_epi32 (saxv16_0);
        __m256i v_1 = _mm256_cvtepu16_epi32 (saxv16_1);


        __m256i c_m    = _mm256_set1_epi32 (max_bit_cardinality);
        __m256i cm_ccv_0 = _mm256_sub_epi32 (c_m, c_cv_0);
        __m256i cm_ccv_1 = _mm256_sub_epi32 (c_m, c_cv_1);

        //__m256i _mm256_set_epi32 (int e7, int e6, int e5, int e4, int e3, int e2, int e1, int e0)    
        //  __m256i _mm256_set1_epi32 (int a)
        __m256i region_lowerv_0 = _mm256_srlv_epi32 (v_0, cm_ccv_0);
        __m256i region_lowerv_1 = _mm256_srlv_epi32 (v_1, cm_ccv_1);
        region_lowerv_0 =  _mm256_sllv_epi32 (region_lowerv_0, cm_ccv_0);
        region_lowerv_1 =  _mm256_sllv_epi32 (region_lowerv_1, cm_ccv_1);

        
        __m256i v1= _mm256_andnot_si256 (_mm256_setzero_si256 (), vectorsignbit);

        __m256i region_upperv_0 = _mm256_sllv_epi32 (v1,cm_ccv_0);
        __m256i region_upperv_1 = _mm256_sllv_epi32 (v1,cm_ccv_1);
        region_upperv_0 = _mm256_andnot_si256 (region_upperv_0, vectorsignbit);
        region_upperv_1 = _mm256_andnot_si256 (region_upperv_1, vectorsignbit);

        region_upperv_0 = _mm256_or_si256 (region_upperv_0, region_lowerv_0);

        region_upperv_1 = _mm256_or_si256 (region_upperv_1, region_lowerv_1);

        region_lowerv_0 = _mm256_add_epi32 (region_lowerv_0, vloweroffset);
        region_lowerv_1 = _mm256_add_epi32 (region_lowerv_1, vloweroffset);
        region_upperv_0 = _mm256_add_epi32 (region_upperv_0, vupperoffset);
        region_upperv_1 = _mm256_add_epi32 (region_upperv_1, vupperoffset);
        _mm256_storeu_si256 ((void*)&(region_lower[0]),region_lowerv_0);
        _mm256_storeu_si256 ((void*)&(region_lower[8]),region_lowerv_1);
        _mm256_storeu_si256 ((void*)&(region_upper[0]),region_upperv_0);
        _mm256_storeu_si256 ((void*)&(region_upper[8]),region_upperv_1);

        
    //lower

        __m256i lower_juge_zerov_0 = _mm256_cmpeq_epi32 (region_lowerv_0, _mm256_setzero_si256 ());
        __m256i lower_juge_zerov_1 = _mm256_cmpeq_epi32 (region_lowerv_1, _mm256_setzero_si256 ());

        __m256i lower_juge_nzerov_0 = _mm256_andnot_si256 (lower_juge_zerov_0, vectorsignbit);
        __m256i lower_juge_nzerov_1 = _mm256_andnot_si256 (lower_juge_zerov_1, vectorsignbit);

        __m256 minvalv = _mm256_set1_ps (min_val);



        
        __m256 lsax_breakpoints_shiftv_0 =_mm256_i32gather_ps (sax_breakpoints,region_lowerv_0, 4);
        //__m256 lsax_breakpoints_shiftv_0= _mm256_set_ps (sax_breakpoints[region_lower[7]],
                                                        //sax_breakpoints[region_lower[6]], 
                                                        //sax_breakpoints[region_lower[5]],
                                                        //sax_breakpoints[region_lower[4]],
                                                        //sax_breakpoints[region_lower[3]],
                                                        //sax_breakpoints[region_lower[2]],
                                                        //sax_breakpoints[region_lower[1]],
                                                        //sax_breakpoints[region_lower[0]]);
        __m256 lsax_breakpoints_shiftv_1 =_mm256_i32gather_ps (sax_breakpoints,region_lowerv_1, 4);
        //__m256 lsax_breakpoints_shiftv_1= _mm256_set_ps (sax_breakpoints[region_lower[15]],
                                                        //sax_breakpoints[region_lower[14]], 
                                                        //sax_breakpoints[region_lower[13]],
                                                        //sax_breakpoints[region_lower[12]],
                                                        //sax_breakpoints[region_lower[11]],
                                                        //sax_breakpoints[region_lower[10]],
                                                        //sax_breakpoints[region_lower[9]],
                                                        //sax_breakpoints[region_lower[8]]);


        __m256 breakpoint_lowerv_0 = (__m256)_mm256_or_si256 (_mm256_and_si256(lower_juge_zerov_0,(__m256i)minvalv),_mm256_and_si256(lower_juge_nzerov_0,(__m256i)lsax_breakpoints_shiftv_0));
        __m256 breakpoint_lowerv_1 = (__m256)_mm256_or_si256 (_mm256_and_si256(lower_juge_zerov_1,(__m256i)minvalv),_mm256_and_si256(lower_juge_nzerov_1,(__m256i)lsax_breakpoints_shiftv_1));

    //uper
        __m256 usax_breakpoints_shiftv_0 =_mm256_i32gather_ps (sax_breakpoints,region_upperv_0, 4);
        //__m256 usax_breakpoints_shiftv_0= _mm256_set_ps (sax_breakpoints[region_upper[7]],
                                                        //sax_breakpoints[region_upper[6]], 
                                                        //sax_breakpoints[region_upper[5]],
                                                        //sax_breakpoints[region_upper[4]],
                                                        //sax_breakpoints[region_upper[3]],
                                                        //sax_breakpoints[region_upper[2]],
                                                        //sax_breakpoints[region_upper[1]],
                                                        //sax_breakpoints[region_upper[0]]);
        __m256 usax_breakpoints_shiftv_1 =_mm256_i32gather_ps (sax_breakpoints,region_upperv_1, 4);
        //__m256 usax_breakpoints_shiftv_1= _mm256_set_ps (sax_breakpoints[region_upper[15]],
                                                        //sax_breakpoints[region_upper[14]], 
                                                        //sax_breakpoints[region_upper[13]],
                                                        //sax_breakpoints[region_upper[12]],
                                                        //sax_breakpoints[region_upper[11]],
                                                        //sax_breakpoints[region_upper[10]],
                                                        //sax_breakpoints[region_upper[9]],
                                                        //sax_breakpoints[region_upper[8]]);


        __m256i upper_juge_maxv_0 = _mm256_cmpeq_epi32 (region_upperv_0,  _mm256_set1_epi32 (max_cardinality - 1));
        __m256i upper_juge_maxv_1 = _mm256_cmpeq_epi32 (region_upperv_1,  _mm256_set1_epi32 (max_cardinality - 1));

        __m256i upper_juge_nmaxv_0 = _mm256_andnot_si256 (upper_juge_maxv_0, vectorsignbit);
        __m256i upper_juge_nmaxv_1 = _mm256_andnot_si256 (upper_juge_maxv_1, vectorsignbit);

        __m256 breakpoint_upperv_0 = (__m256)_mm256_or_si256 (_mm256_and_si256(upper_juge_maxv_0,(__m256i)_mm256_set1_ps (max_val)),_mm256_and_si256(upper_juge_nmaxv_0,(__m256i)usax_breakpoints_shiftv_0));
        __m256 breakpoint_upperv_1 = (__m256)_mm256_or_si256 (_mm256_and_si256(upper_juge_maxv_1,(__m256i)_mm256_set1_ps (max_val)),_mm256_and_si256(upper_juge_nmaxv_1,(__m256i)usax_breakpoints_shiftv_1));







    //dis
            __m256 paaUv_0,paaUv_1,paaLv_0,paaLv_1;



            ///paav_0 =_mm256_loadu_ps (paa);
           // paav_1 =_mm256_loadu_ps (&(paa[8]));
            paaUv_0 =_mm256_loadu_ps (paaU);
            paaUv_1 =_mm256_loadu_ps (&(paaU[8]));            
            paaLv_0 =_mm256_loadu_ps (paaL);
            paaLv_1 =_mm256_loadu_ps (&(paaL[8]));


            __m256 dis_juge_upv_0=_mm256_cmp_ps (breakpoint_lowerv_0, paaUv_0, _CMP_GT_OS);
            __m256 dis_juge_upv_1=_mm256_cmp_ps (breakpoint_lowerv_1, paaUv_1, _CMP_GT_OS);

            __m256 dis_juge_lov_0=_mm256_cmp_ps (breakpoint_upperv_0, paaLv_0, _CMP_LT_OS);
            __m256 dis_juge_lov_1=_mm256_cmp_ps (breakpoint_upperv_1, paaLv_1, _CMP_LT_OS);



            //__m256 dis_juge_lov_0=(__m256)_mm256_and_si256 ((__m256i)_mm256_cmp_ps (breakpoint_lowerv_0, paaUv_0, _CMP_NGT_US),(__m256i)_mm256_cmp_ps (breakpoint_upperv_0, paav_0, _CMP_LT_OS))  ;
            //__m256 dis_juge_lov_1=(__m256)_mm256_and_si256 ((__m256i)_mm256_cmp_ps (breakpoint_lowerv_1, paaUv_1, _CMP_NGT_US),(__m256i)_mm256_cmp_ps (breakpoint_upperv_1, paav_1, _CMP_LT_OS));

            __m256 dis_juge_elv_0=(__m256)_mm256_andnot_si256 (_mm256_or_si256 ((__m256i)dis_juge_upv_0, (__m256i)dis_juge_lov_0),vectorsignbit);
            __m256 dis_juge_elv_1=(__m256)_mm256_andnot_si256 (_mm256_or_si256 ((__m256i)dis_juge_upv_1, (__m256i)dis_juge_lov_1),vectorsignbit);

            __m256 dis_lowv_0 =_mm256_sub_ps (breakpoint_lowerv_0, paaUv_0);
            __m256 dis_lowv_1 =_mm256_sub_ps (breakpoint_lowerv_1, paaUv_1);
            __m256 dis_uppv_0 =_mm256_sub_ps (breakpoint_upperv_0, paaLv_0);
            __m256 dis_uppv_1 =_mm256_sub_ps (breakpoint_upperv_1, paaLv_1);


            __m256 distancev_0=(__m256)_mm256_or_si256(_mm256_or_si256(_mm256_and_si256((__m256i)dis_juge_upv_0,(__m256i)dis_lowv_0),_mm256_and_si256((__m256i)dis_juge_lov_0,(__m256i)dis_uppv_0)),_mm256_and_si256((__m256i)dis_juge_elv_0,(__m256i)_mm256_set1_ps (0.0)));
            __m256 distancev_1=(__m256)_mm256_or_si256(_mm256_or_si256(_mm256_and_si256((__m256i)dis_juge_upv_1,(__m256i)dis_lowv_1),_mm256_and_si256((__m256i)dis_juge_lov_1,(__m256i)dis_uppv_1)),_mm256_and_si256((__m256i)dis_juge_elv_1,(__m256i)_mm256_set1_ps (0.0)));


            __m256 distancesum_0 = _mm256_dp_ps (distancev_0, distancev_0, 0xff);
            __m256 distancesum_1 = _mm256_dp_ps (distancev_1, distancev_1, 0xff);
            __m256 distancevf = _mm256_add_ps (distancesum_0, distancesum_1);
            //__m256 distancev2 = _mm256_hadd_ps (distancev, distancev);
            //__m256 distancevf = _mm256_hadd_ps (distancev2, distancev2);
            //__m256 _mm256_dp_ps (__m256 a, __m256 b, const int imm8);

            _mm256_storeu_ps (distancef ,distancevf);
        //_mm256_storeu_ps (&checkvalue[8] ,distancev_1);


            return (distancef[0]+distancef[4])*ratio_sqrt ;
}

