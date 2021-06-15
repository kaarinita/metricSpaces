/********************************************************
 * oracle.h
 *
 * These routines handle the inquire functions between
 * the algorithm and the data set oracle.
 *
 * Sixth Annual DIMACS Implementation Challenge
 * Author:  Michael Goldwasser (wass@cs.princeton.edu)
 * Created: Jan 16, 1998
 *******************************************************/

#ifndef _ORACLE
#define _ORACLE
 

/*  abstract object -- cast to correct type within module */
typedef void Oracle;


#define MAX_FIELD_LENGTH  50      /* max string length for InqField result */


/********************************************************
 * int InqNumPoints(Oracle *ora)
 *
 * Parameters:
 *   Oracle* ora --- an oracle connection
 *
 * Returns:
 *   The number of points in the original data set
 *
 * Description:
 *   If the routine returns 'n' this means that there are n points in
 *   the data set (not counting any queries).  When querying the
 *   oracle about individual points, they are referenced with IDs
 *   ranging from 0 to n-1.
 *  
 *******************************************************/
int  InqNumPoints(Oracle *ora);


/********************************************************
 * int InqNumQuery(Oracle *ora)
 *
 * Parameters:
 *   Oracle* ora --- an oracle connection
 *
 * Returns:
 *   The number of query points available
 *
 * Description:
 *   If the routine returns 'q' this means that there are q query
 *   points available.   When querying the oracle about these points,
 *   we assume that they are given IDs consecutively following all of
 *   the original data points.  That is, if there are 'n' original
 *   points and 'q' query points, the query points are indexed in the
 *   range [n,n+q-1].
 *
 *******************************************************/
int  InqNumQuery(Oracle *ora);



/********************************************************
 * double InqDist(Oracle *ora, int p1, int p2)
 *
 * Parameters:
 *   Oracle* ora --- an oracle connection
 *   int p1      --- index to first point
 *   int p2      --- index to second point
 *
 * Returns:
 *   The non-negative, real-numbered, distance between the points
 *
 * Description:
 *   Each point can be either an original point in the data set,
 *   or one of the query points.  The convention for indexing is
 *   identical to the above functions. 
 *
 *******************************************************/
double InqDist(Oracle *ora, int p1, int p2);


/********************************************************
 * int InqNumFields(Oracle *ora, int p)
 *
 * Parameters:
 *   Oracle* ora --- an oracle connection
 *   int p       --- index to data point
 *
 * Returns:
 *   An integer specifying how many fields describe the
 *   underlying raw data for point p. (This could potentially be
 *   a different value for different points in the underlying space.
 *
 * Description:
 *   The point can be the index for either an original point in the
 *   underlying space, or one of the query points.  The convention for
 *   indexing points is identical to the above functions.  The
 *   convention for indexing fields is from [0..InqNumField-1].
 *
 * Note:
 *   In the case of Euclidean vectors, this string should be the
 *   number of dimensions.
 *
 *******************************************************/
int InqNumFields(Oracle *ora, int p);
 
 
/********************************************************
 * char* InqField(Oracle *ora, int p, int f)
 *
 * Parameters:
 *   Oracle* ora --- an oracle connection
 *   int p       --- index to data point
 *   int f       --- which field of raw data
 *
 * Returns:
 *   A string of length at most MAX_FIELD_LENGTH which is the
 *   associated field for the raw data describing a point in the
 *   space.  The ownership of this string is passed to the caller, and
 *   it is assumed that the caller will free the memory for it.
 *
 * Description:
 *   The point can be the index for either an original point in the
 *   underlying space, or one of the query points.  The convention for
 *   indexing points is identical to the above functions.  The
 *   convention for indexing fields is from [0..InqNumField-1].
 *
 * Note:
 *   In the case of Euclidean vectors, this string should be the
 *   real-valued entry for the coordinate in question.
 *
 *******************************************************/
char*  InqField(Oracle *ora, int p, int f);
 

 
#endif
