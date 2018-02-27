#ifndef VECTOR_UTILS_H
#define VECTOR_UTILS_H

#include <QtCore>

#include <map>
#include <vector>
#include <list>

using namespace std;

class Vector_Utils{
 public:

   static vector < double > intersection( vector < double > &x, vector < double > &y );
   static vector < double > intersection( vector < vector < double > > &x );
   static vector < double > vunion( vector < double > &x, vector < double > &y );
   static vector < double > vunion( vector < vector < double > > &x );

   static QStringList vector_qstring_to_qstringlist( const vector < QString > & vqs );

};

#endif
