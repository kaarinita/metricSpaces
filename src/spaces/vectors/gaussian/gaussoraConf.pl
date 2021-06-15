#!/usr/bin/perl

use strict;

if ($#ARGV != 2) {
  print "\n$0: generates configuration file for gaussora.\n";
  print "The probability of belonging to any cluster is uniform..\n";
  print "The labels of clusters are Ci, for i = 1, 2,...\n";
  print "Sends to standard output.\n\n";
  print "Usage:\n";
  print "$0 nclus dim var > configFile\n";
  print "nclus: number of clusters\n";
  print "dim:   dimension of the vectors\n";
  print "var:   variance of the clusters\n\n";
  exit;
}

my $nclus = $ARGV[0];    # numero de clusters
my $dim =   $ARGV[1];    # la dimension de los vectores
my $var =   $ARGV[2];    # la varianza (devstd^2)

#print stderr "nclus $nclus, dim $dim, var $var\n";

srand();

for (my $i = 1; $i <= $nclus; $i++) {
  print "GAUSS C$i $dim Diag\n";
  for (my $j = 0; $j < $dim; $j++) {
    printf "%8.6f ", rand();
  }
  print "\n";
  for (my $j = 0; $j < $dim; $j++) { print "$var "; }
  print "\n\n";
}

