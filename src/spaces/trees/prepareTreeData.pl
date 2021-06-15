#!/usr/bin/perl -w


 # Copyright (C) 2009 Arnoldo Jose Muller Molina
 #  This program is free software: you can
 # redistribute it and/or modify it under the terms of the GNU General Public
 # License as published by the Free Software Foundation, either version 3 of the
 # License, or (at your option) any later version. This program is distributed
 # in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 # the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 # See the GNU General Public License for more details. You should have received
 # a copy of the GNU General Public License along with this program. If not, see
 # <http://www.gnu.org/licenses/>.
 


# Prepare a query and a DB from a newline separated DB of data.
# we make sure also that DB and query do not have repeated objects.

# usage:

#prepareData.pl <fileName> <db_size> <querySize> <query>
# where:
# <fileName> is the file name of a database of newline separated objects.
# <db_size> the final size of the db.
# <querySize> is the size of the query.
# <query> preceded by a - sign is a k-nn and otherwise is a range query. 

# Output:
# a new file <fileName>.query will be create with the query objects.
# a new file <fileName>.db will be create with objects - the query 
#         (objects inside the generated query will not be included).


$input_file = $ARGV[0]; #input file.
$db_size = $ARGV[1]; # final size of the db file
$query_size = $ARGV[2]; # number of queries.
$query = $ARGV[3]; # query mode

open IN , "<$input_file" or die "Could not open DB file $input_file";
open DB , ">$input_file.db" or die "Could not open output DB file $input_file.db";
open Q , ">$input_file.query" or die "Could not open output query file $input_file.query";

# load unique lines into the hash
while(<IN>){
		$db{$_} = ":)";	
}

# lines of the db
$totalLines = scalar(keys %db);

print "total lines: $totalLines\n";

# generate some random numbers
$i = 0;
while($i < $query_size){
		$query{int(rand($totalLines))} = ":)";
		$i = scalar(keys %query);
}

# separate the query from the db.
$i = 0;
$final_db_size = 0; 
foreach $key (keys %db) {
		if(exists $query{$i}){
				print Q "$query,$key";
    }else{
				if($final_db_size < $db_size){
						print DB $key;
						$final_db_size++;
				}
		}
		$i++;
}

# finish the query
print Q "-0\n";

close(Q);
close(DB);
close(IN);

# count the lines of a file
sub countLines {
		my($file) = @_;
		open IN , "<$file" or die "Could not open DB file $file";
		my $totalLines = 0;
		while(<IN>){
				$totalLines++;
		}
		close(IN);
		return $totalLines;
}
