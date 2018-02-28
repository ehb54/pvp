/**
 * \class SASFiles
 *
 * \brief Provide an example
 *
 * This class handles loading and processing of SASFiles
 *
 * \note Attempts at zen rarely work.
 *
 * \author (last to touch it) $Author: emre h. brookes $
 *
 * \version $Revision: 0.1 $
 *
 * \date $Date: 2018/02/01 14:16:20 $
 */

#ifndef SASFILES_H
#define SASFILES_H

#include <QtCore>
#include "pvp.h"
#include "vectorutils.h"

#include <set>
#include <list>
#include <algorithm>

using namespace std;

class SASFiles {
 public:
   /** \brief load
    * \param files a QStringList of files to process
    *
    * loads files into class
    */
   bool load( const QStringList & files, bool quiet = false );
   
   /** \brief run_pvp
    *
    * computes a pvp matrix
    */

   bool run_pvp( map < QString, QString > & parameters );
   
   /// A QString containing any errors 
   QStringList  errors;

   /// A QString containing any warnings
   QStringList  warnings;

   /// A QString containing any notices
   QStringList  notices;

 private:
   PVP pvp;

   QStringList files;
   
   bool read_one_file( const QString & file );

   void reset_messages();
   bool is_zero_vector( const vector < double > &v );

   QStringList reorder( const QStringList &files );
   QString qstring_common_head( const QStringList & qsl, bool strip_digits );
   QString qstring_common_tail( const QStringList & qsl, bool strip_digits );
   QString qstring_common_head( const QString & s1, const QString & s2 );
   QString qstring_common_tail( const QString & s1, const QString & s2 );

   map < QString, vector < QString > > f_qs_string;
   map < QString, vector < double > >  f_qs;
   map < QString, vector < double > >  f_Is;
   map < QString, vector < double > >  f_errors;
   map < QString, unsigned int >       f_pos;

   map < QString, QString >            f_name;
   map < QString, QString >            f_header;
   map < QString, bool >               f_is_time;
   map < QString, double >             f_psv;
   map < QString, double >             f_I0se;
   map < QString, double >             f_conc;
   map < QString, double >             f_extc;
   map < QString, double >             f_time;
};

#endif

