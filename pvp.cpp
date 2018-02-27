#include "pvp.h"

double PVP::prob_of_streak( int n, int c ) {
   if ( prob_of_streak_cache.count( n ) &&
        prob_of_streak_cache[ n ].count( c ) ) {
      return prob_of_streak_cache[ n ][ c ];
   }
   if  ( c > n || n <= 0e0 ) {
      return 0e0;
   }

   double res = pow( 0.5e0, c );

   for ( int i = 1; i <= c; ++i ) {
        double pr = prob_of_streak( n - i, c );
        res += pow( 0.5e0, i - 1 ) * 0.5e0 * pr;
   }

   prob_of_streak_cache[ n ][ c ] = res;
   return res;
}

float PVP::prob_of_streak_f( int n, int c ) {
   if ( prob_of_streak_cache_f.count( n ) &&
        prob_of_streak_cache_f[ n ].count( c ) ) {
      return prob_of_streak_cache_f[ n ][ c ];
   }
   if  ( c > n || n <= 0e0 ) {
      return 0e0;
   }

   float res = pow( 0.5e0, c );

   for ( int i = 1; i <= c; ++i ) {
        float pr = prob_of_streak_f( n - i, c );
        res += pow( 0.5e0, i - 1 ) * 0.5e0 * pr;
   }

   prob_of_streak_cache_f[ n ][ c ] = res;
   return res;
}

bool PVP::compute( 
                  const vector < double >            & q,
                  const vector < vector < double > > & I,
                  vector < vector < double > >       & rkl,
                  int                                & N,
                  int                                & S,
                  int                                & C,
                  double                             & P
                   ) {

   notices = "";
   errors = "";
   
   int m  = (int) I.size();

   if ( m < 2 ) {
         errors = 
            QString( "Error in PVP::compute: a minimum of 2 intensity vectors are required and only %1 were given" )
            .arg( m );
         return false;
   }

   int qp = (int) q.size();
   double mm1r = 1e0 / ( (double) m - 1e0 );
   double mr   = 1e0 / ( (double) m );

   for ( int i = 0; i < m; ++i ) {
      if ( (int) I[ i ].size() != qp ) {
         errors = 
            QString( "Error in PVP::compute: the intensity vectors are of different size (%1)  than the q vector size (%2)" )
            .arg( I[ i ].size() )
            .arg( q     .size() )
            ;
         return false;
      }
   }

   vector < double > Ibar = I[ 0 ];

   for ( int k = 0; k < qp; ++k ) {
      for ( int i = 1; i < m; ++i ) {
         Ibar[ k ] += I[ i ][ k ];
      }
      Ibar[ k ] *= mr;
   }

   vector < vector < double > > sigma( qp );

   for ( int k = 0; k < qp; ++k ) {
      sigma[ k ].resize( qp );
      for ( int l = 0; l < qp; ++l ) {
         sigma[ k ][ l ] = 0e0;
         for ( int i = 0; i < m; ++i ) {
            sigma[ k ][ l ] += ( I[ i ][ k ] - Ibar[ k ] ) * ( I[ i ][ l ] - Ibar[ l ] );
         }
         sigma[ k ][ l ] *= mm1r;
      }
   }

   rkl.resize( qp );

   for ( int k = 0; k < qp; ++k ) {
      rkl[ k ].resize( qp );
      for ( int l = 0; l < qp; ++l ) {
         bool altsigma = false;
         if ( sigma[ k ][ k ] == 0e0 ) {
            notices += QString( "Notice in PVP::compute: found zero sigma at %1 %2\n" ).arg( k ).arg( k );
            altsigma = true;
         }
         if ( sigma[ l ][ l ] == 0e0 ) {
            notices += QString( "Notice in PVP::compute: found zero sigma at %1 %2\n" ).arg( l ).arg( l );
            altsigma = true;
         }
         if ( altsigma ) {
            rkl[ k ][ l ] = -1e0;
         } else {
            rkl[ k ][ l ] = sigma[ k ][ l ] / ( sqrt( sigma[ k ][ k ] ) * sqrt( sigma[ l ][ l ] ) );
         }            
      }
   }

   double min = rkl[ 0 ][ 0 ];
   double max = rkl[ 0 ][ 0 ];

   for ( int k = 0; k < qp; ++k ) {
      rkl[ k ].resize( qp );
      for ( int l = 0; l < qp; ++l ) {
         if ( min > rkl[ k ][ l ] ) {
            min = rkl[ k ][ l ];
         }
         if ( max < rkl[ k ][ l ] ) {
            max = rkl[ k ][ l ];
         }
      }
   }

   // find largest patch ... go down diagonal and check 1/2 block

   {
      int longest_start_q     = 0;
      int contiguous_pts      = 1;
      bool pos_region         = rkl[ 0 ][ 0 ] > 0;

      int this_start_q        = longest_start_q;
      int this_contiguous_pts = contiguous_pts;

      for ( int k = 1; k < qp; ++k ) {
         bool new_region = false;
         if ( ( rkl[ k ][ k ] > 0 ) != pos_region ) {
            new_region = true;
         } else {
            // check 1/2 block
            for ( int k2 = this_start_q; !new_region && k2 <= k; ++k2 ) {
               for ( int l2 = k2; !new_region && l2 <= k; ++l2 ) {
                  if ( ( rkl[ k2 ][ l2 ] > 0 ) != pos_region ) {
                     new_region = true;
                  }
               }
            }
         }

         if ( !new_region ) {
            this_contiguous_pts++;
         } else {
            if ( contiguous_pts < this_contiguous_pts ) {
               contiguous_pts      = this_contiguous_pts;
               longest_start_q     = this_start_q;
            }
            this_start_q        = k;
            this_contiguous_pts = 1;
            pos_region          = rkl[ k ][ k ] > 0;
         }
      }

      if ( contiguous_pts < this_contiguous_pts ) {
         contiguous_pts      = this_contiguous_pts;
         longest_start_q     = this_start_q;
      }

      N = qp;
      S = longest_start_q + 1;
      C = contiguous_pts;
      P = (double) prob_of_streak_f( N, C );
   }

   return true;
}
