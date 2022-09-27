sh clean.sh
sh autogen.sh
rm -rf *.cache
git archive HEAD | bzip2 > ../ads_isax-2.0.tar.bz2
