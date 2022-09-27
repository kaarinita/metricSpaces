# ParIS_plus_and_MESSI

This is the ads isax based time-series index.

In order to install run the following commands:

./configure
make

and then run:

./ads_isax --help


This is the ads isax based time-series index.



We provide ParIS+ and MESSI. Note that the server that executes MESSI should support AVX2.



command example:

ParIS+ index creation:
./paris --dataset datafile --leaf-size 2000 --initial-lbl-size 2000 --index-path index-path --dataset-size 50000000 --flush-limit 1000000 --cpu-type 21 --function-type 1

ParIS+ query answering:
./paris --queries queryfile --use-index --index-path index-path --function-type 0 --serial --cpu-type 42 --queries-size 20



ParIS+ topk query answering:
./paris --queries queryfile --use-index --index-path index-path --function-type 0 --serial --cpu-type 42 --queries-size 20 -topk --k-size 10


ParIS+ knn query answering:
./paris --queries queryfile --knn-label-set labelfile --use-index --index-path indexpath --function-type 0 --serial --cpu-type 42 --queries-size 20 --knn --k-size 10 --knn-label-size XXX 




./MESSI --dataset dataset.bin --in-memory --initial-lbl-size 2000 --leaf-size 2000 --min-leaf-size 2000 --function-type 3 --cpu-type 82 --dataset-size 1000000 --flush-limit 300000 --read-block 20000 --queries query.bin --queries-size 10 --queue-number 2 

A help page that explains the parameters is obtained by executing ./MESSI --help

MESSI DTW is:
./MESSI --dataset dataset.bin --leaf-size 2000 --initial-lbl-size 2000 --min-leaf-size 2000 --dataset-size 10000 --flush-limit 20000000 --function-type 3 --in-memory --cpu-type 82 --queries query.bin --queries-size 10 --queue-number 24 --dtwwindowsize 12 



MESSI KNN is:
./MESSI --dataset dataset.bin --leaf-size 2000 --initial-lbl-size 2000 --min-leaf-size 2000 --dataset-size 10000 --flush-limit 20000000 --function-type 3 --in-memory --cpu-type 82 --queries query.bin --knn-label-set knnlabel.bin --k-size 5 --knn --knn-label-size 10 --queries-size 10 --queue-number 24 (--dtwwindowsize 12) 

PS：the SIMD lower bound distance calculation code is only for 16 segments.  The real distance calculation is only for Multiples of 8.
if you use other parameters please use their SISD version (remove "_SIMD").