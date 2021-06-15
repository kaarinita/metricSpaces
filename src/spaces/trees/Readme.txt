
Trees Space

Author: Arnoldo Jose Muller-Molina
Institution: Kyushu Institute of Technology (Iizuka, Japan)

Introduction
-------------------------------------------------

The "trees" space can be used as a framework to develop different
tree distance functions. We currently support only one distance
function. 

Space
~~~~~

The "trees" space only includes the distance function mtd from the paper:

Arnoldo Jose Muller Molina, Kouichi Hirata and Takeshi Shinohara:
A Tree Distance Function Based on Multi-sets, Proc. JSAI PAKDD 2008

This function requires to compute the l1 distance of sparse vectors of 
dimensionality >= to the size of the dataset. 

We have other tree distance function implementations (in Java)
here:

http://treelib.berlios.de/

These distance functions are compatible with the tree format included
in the accompanying dataset.


Dataset
~~~~~~~
The included dataset is composed of program fragments (trees)  extracted from binary programs.
This set of trees is used to detect GPL license violations (binary program matching).
For more information: http://www.furiachan.org

Statistics:

* The dataset has about 350000 trees. 
* 905878 complete subtrees in the dataset.
* When computing the distance mtd, the dimensionality of each tree 
  is always at least 905878.


Requirements
-------------------------------------------------

Only one library is required:

* Antlr 2.7 for C++



Format details
-------------------------------------------------

Our trees are stored using a string representation. 

Example:
"a(b,e)" 

This string creates the tree:
    a
   / \
  b   e


Query preparation
----------------------------------------------

We provide a perl script that can generate a database
size of size N, with a query of size J. The intersection
between the resulting query set and the database set
will always be void.

Example:

./prepareTreeData.pl slices 10000 100 -1

Generate a N=10000 database and a J=100 query
append to the query the prefix -1 so that we can perform
1-nn queries. Two files slices.db and slices.query will
be created after running the script. 
