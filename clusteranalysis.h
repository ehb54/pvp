/**
 * \class ClusterAnalysis
 *
 * \brief Provide an example
 *
 * This class does an analysis of "clusters" in the PVP map
 *
 * \author (last to touch it) $Author: emre h. brookes $
 *
 * \version $Revision: 0.1 $
 *
 * \date $Date: 2018/02/01 14:16:20 $
 */

#ifndef CLUSTER_ANALYSIS_H
#define CLUSTER_ANALYSIS_H

#include <QtCore>
#include <vector>
#include <map>
#include <set>

using namespace std;

class ca_index_pair {
public:
   int r;
   int c;
   bool operator < (const ca_index_pair& objIn) const
   {
      return r < objIn.r || ( r == objIn.r && c < objIn.c );
   }
   bool operator == (const ca_index_pair& objIn) const
   {
      return r == objIn.r &&  c == objIn.c;
   }
};

class ClusterAnalysis {
 public:

   bool                        run( 
                                   vector < vector < double > >  & pvaluepairs,
                                   map < QString, QString >      & parameters,
                                   map < QString, QString >      & csv_report
                                    );

   bool                        sliding( 
                                       vector < vector < double > >  & pvaluepairs,
                                       map < QString, QString >      & parameters,
                                       map < QString, double >       & sliding_results
                                        );

   bool                        sliding( 
                                       vector < vector < double > >  & pvaluepairs,
                                       map < QString, QString >      & parameters,
                                       map < QString, double >       & sliding_results,
                                       map < QString, double >       & hb_sliding_results
                                        );

   map < int, int >            cluster_size_histogram;
   QString                     errors;
   
 private:
   
   void                        cluster_expand( ca_index_pair x );
   map < ca_index_pair, int >  cluster_data;
   set < ca_index_pair >       cluster_marked;
   set < ca_index_pair >       cluster_red;
};

#endif
