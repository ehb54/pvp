/**
 * \class PVP
 *
 * \brief Provide an example
 *
 * This class takes q,I data and computes a p value pair matrix
 *
 * \author (last to touch it) $Author: emre h. brookes $
 *
 * \version $Revision: 0.1 $
 *
 * \date $Date: 2018/02/01 14:16:20 $
 */

#ifndef PVP_H
#define PVP_H

#include <QtCore>

#include <map>
#include <vector>

using namespace std;

class PVP {

 public:

   /** \brief SASFiles
    * \param q a vector of q values
    * \param I a matrix of I values of size 2,length of q
    * \param rkl returns a matrix of p values
    * \param N returns the total length
    * \param S returns the start position of the longest streak
    * \param C returns the longest number of contiguous points
    * \param P returns the probability of streak
    * \return True if computation was successful, otherwise, check errors
    */

   bool    compute( 
                   const vector < double >            & q,
                   const vector < vector < double > > & I,
                   vector < vector < double > >       & rkl,
                   int                                & N,
                   int                                & S,
                   int                                & C,
                   double                             & P
                    );

   /// A QString containing any errors 
   QString  errors;
   /// A QString containing any notices
   QString  notices;

 private:

   double   prob_of_streak( int n, int c );
   float    prob_of_streak_f( int n, int c );

   map < int, map < int, double > > prob_of_streak_cache;
   map < int, map < int, float > >  prob_of_streak_cache_f;

};

#endif
