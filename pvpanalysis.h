/**
 * \class PVPAnalysis
 *
 * \brief Provide an example
 *
 * This class does the work of analyzing the PVP data
 *
 * \author (last to touch it) $Author: emre h. brookes $
 *
 * \version $Revision: 0.1 $
 *
 * \date $Date: 2018/02/01 14:16:20 $
 */

#ifndef PVP_ANALYSIS_H
#define PVP_ANALYSIS_H

#include <QtCore>
#include <QImage>
#include <QPixmap>

#include <map>
#include <vector>
#include <algorithm>

#include "clusteranalysis.h"

using namespace std;

class PVPAnalysis {

 public:

   /** \brief PVPAnalysis
    * \param parameters a map of parameter settings
    * \param pvaluepairs a matrix of p value pairs
    * \param adjpvaluepairs a matrix of adjusted p values pairs
    * \param selected_files a vector of files names
    * \return True if computation was successful, otherwise, check errors
    */

   bool compute(
                map < QString, QString >     & parameters,
                vector < vector < double > > & pvaluepairs,
                vector < vector < double > > & adjpvaluepairs,
                vector < QString >           & selected_files
                );

   /// A QString containing any errors 
   QString  errors;
   /// A QString containing any notices
   QString  notices;

   static double                   holm_bonferroni( vector < double > P, double alpha );

 private:

   bool                            cluster_analysis();

   map < QString, QString >  *     pparameters;
   vector < vector < double > >    pvaluepairs;
   vector < vector < double > >    adjpvaluepairs;
   vector < QString >              selected_files;

   double                          alpha;
   double                          alpha_over_5;
   double                          hb_alpha;
   double                          hb_alpha_over_5;

   QStringList                     csv_headers;
   map < QString, QString >        csv_report;

   QImage                   *      qi;
   QImage                   *      qi_adj;
   QImage                   *      qi_hb;
};

#endif
