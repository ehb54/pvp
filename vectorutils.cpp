#include "vectorutils.h"

vector < double > Vector_Utils::intersection( vector < double > & x,  vector < double > & y ) {
   map < double, bool > map_x;
   for ( unsigned int i = 0; i < ( unsigned int ) x.size(); i++ ) {
      map_x[ x[ i ] ] = true;
   }
   vector < double > result;
   for ( unsigned int i = 0; i < ( unsigned int ) y.size(); i++ ) {
      if ( map_x.count( y[ i ] ) ) {
         result.push_back( y[ i ] );
      }
   }
   return result;
}

vector < double > Vector_Utils::intersection( vector < vector < double > > & x ) {
   // make a common grid
   
   vector < double > result;

   if ( !x.size() ) {
      return result;
   }

   if ( x.size() == 1 ) {
      return x[ 0 ];
   }

   result = x[ 0 ];
   for ( unsigned int i = 1; i < ( unsigned int ) x.size(); i++ ) {
      result = intersection( result, x[ i ] );
   }
   return result;
}

vector < double > Vector_Utils::vunion( vector < double > & x,  vector < double > & y ) {
   list < double > lx;
   list < double > ly;
   
   for ( unsigned int i = 0; i < ( unsigned int ) x.size(); i++ ) {
      lx.push_back( x[ i ] );
   }
   for ( unsigned int i = 0; i < ( unsigned int ) y.size(); i++ ) {
      ly.push_back( y[ i ] );
   }

   lx.merge( ly );
   lx.unique();

   vector < double > result;
   for ( list < double >::iterator it = lx.begin();
         it != lx.end();
         it++ ) {
      result.push_back( *it );
   }
   return result;
}

vector < double > Vector_Utils::vunion( vector < vector < double > > & x ) {
   // make a common grid
   
   vector < double > result;

   if ( !x.size() ) {
      return result;
   }

   if ( x.size() == 1 ) {
      return x[ 0 ];
   }

   result = x[ 0 ];
   for ( unsigned int i = 1; i < ( unsigned int ) x.size(); i++ ) {
      result = vunion( result, x[ i ] );
   }
   return result;
}

QStringList Vector_Utils::vector_qstring_to_qstringlist( const vector < QString > & vqs ) {
   QStringList qsl;
   for ( int i = 0; i < (int) vqs.size(); ++i ) {
      qsl << vqs[ i ];
   }
   return qsl;
}
