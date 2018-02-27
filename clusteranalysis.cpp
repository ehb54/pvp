#include "clusteranalysis.h"
#include "pvpanalysis.h"

void ClusterAnalysis::cluster_expand( ca_index_pair x ) {
   // check neighbors
   // cout << QString( "cluster expand [%1,%2]\n" ).arg( x.r ).arg( x.c );
   ca_index_pair xp;
   {
      xp = x;
      xp.r++;
      if ( cluster_data.count( xp ) && 
           !cluster_marked.count( xp ) &&
           cluster_data[ xp ] == -1 ) {
         cluster_red   .insert( xp );
         cluster_marked.insert( xp );
         cluster_expand( xp );
      }
   }

   {
      xp = x;
      xp.r--;
      if ( cluster_data.count( xp ) && 
           !cluster_marked.count( xp ) &&
           cluster_data[ xp ] == -1 ) {
         cluster_red   .insert( xp );
         cluster_marked.insert( xp );
         cluster_expand( xp );
      }
   }

   {
      xp = x;
      xp.c++;
      if ( cluster_data.count( xp ) && 
           !cluster_marked.count( xp ) &&
           cluster_data[ xp ] == -1 ) {
         cluster_red   .insert( xp );
         cluster_marked.insert( xp );
         cluster_expand( xp );
      }
   }

   {
      xp = x;
      xp.c--;
      if ( cluster_data.count( xp ) && 
           !cluster_marked.count( xp ) &&
           cluster_data[ xp ] == -1 ) {
         cluster_red   .insert( xp );
         cluster_marked.insert( xp );
         cluster_expand( xp );
      }
   }
}

bool ClusterAnalysis::sliding(
                               vector < vector < double > >  & pvaluepairs,
                               map < QString, QString >      & parameters,
                               map < QString, double >       & sliding_results,
                               map < QString, double >       & hb_sliding_results
                               ) {
   if ( !parameters.count( "hb" ) ) {
      if ( !sliding(
                    pvaluepairs,
                    parameters,
                    sliding_results
                    ) ) {
         return false;
      }

      parameters[ "hb" ] = "true";
      
      if ( !sliding(
                    pvaluepairs,
                    parameters,
                    hb_sliding_results
                    ) ) {
         parameters.erase( "hb" );
         return false;
      }
      parameters.erase( "hb" );
      return true;
   }

   if ( !sliding(
                 pvaluepairs,
                 parameters,
                 sliding_results
                 ) ) {
      hb_sliding_results = sliding_results;
      return false;
   }

   hb_sliding_results = sliding_results;
   return true;
}

bool ClusterAnalysis::sliding(
                               vector < vector < double > >  & pvaluepairs,
                               map < QString, QString >      & parameters,
                               map < QString, double >       & sliding_results
                               ) {

   double alpha        = parameters.count( "alpha" ) ? parameters[ "alpha" ].toDouble() : 0.05;
   double alpha_over_5 = parameters.count( "alpha_over_5" ) ? parameters[ "alpha_over_5" ].toDouble() : 0.2 * alpha;

   sliding_results.clear( );

   int pc = (int) pvaluepairs.size();

   if ( pc != (int) pvaluepairs[ 0 ].size() ) {
      errors = "Internal error: cormap of brookes selected, but pvaluepairs not square";
      return false;
   }

   int start_size =
      parameters.count( "sliding_minimum_size" ) ?
      parameters[ "sliding_minimum_size" ].toInt() : 10;

   if ( start_size > pc ) {
      errors = "Brookes map sliding cluster analysis: too few frames for analysis";
      return false;
   }
      
   int pos_base = parameters.count( "sliding_baseline_pos_base" ) ? parameters[ "sliding_baseline_pos_base" ].toInt() : 0;

   // build sub matricies of pvaluepairs and average red cluster size over

   int largest_window = parameters.count( "sliding_maximum_size" ) ? parameters[ "sliding_maximum_size" ].toInt() : pc;
   if ( largest_window > pc ) {
      largest_window = pc;
   }

   QString save_alpha = parameters.count( "alpha" ) ? parameters[ "alpha" ] : "0.05";

   for ( int i = start_size; i <= largest_window; ++i ) {
      vector < vector < double > > sliding_window( i );
      for ( int j = 0; j < i; ++j ) {
         sliding_window[ j ].resize( i );
      }

      QString prefix    = QString( "%1:"      ).arg( i );
      QString prefix2   = QString( "%1 sd:"   ).arg( i );
      QString prefixcnt = QString( "%1 count" ).arg( i );

      int     count   = 0;

      map < QString, QString >  csv_report;

      for ( int j = 0; j <= pc - i; ++j ) {
         for ( int k = 0; k < i; ++k ) {
            for ( int l = 0; l < i; ++l ) {
               sliding_window[ k ][ l ] = pvaluepairs[ j + k ][ j + l ];
            }
         }
         
         csv_report.clear( );

         if ( parameters.count( "hb" ) ) {
            vector < double > P;
            int sls = (int) sliding_window.size();
            for ( int ii = 0; ii < sls; ++ii ) {
               if ( ii + 1 < sls ) {
                  for ( int jj = ii + 1; jj < sls; ++jj ) {
                     P.push_back( sliding_window[ ii ][ jj ] );
                  }
               }
            }
            parameters[ "alpha" ]        = QString( "%1" ).arg( PVPAnalysis::holm_bonferroni( P, alpha ) );
            parameters[ "alpha_over_5" ] = QString( "%1" ).arg( PVPAnalysis::holm_bonferroni( P, alpha_over_5 ) );
         }

         if ( !run( sliding_window,
                    parameters,
                    csv_report ) ) {
            errors = "Brookes map sliding cluster analysis:" + errors;
            return false;
         }

         if ( parameters.count( "sliding_baseline_mode" ) ) {
            for ( map < QString, QString >::iterator it = csv_report.begin();
                  it != csv_report.end();
                  ++it ) {
               QString this_tag  = QString( "%1:" ).arg( pos_base + j ) + it->first;
               sliding_results[ this_tag  ] = it->second.toDouble();
            }
         } else {
            for ( map < QString, QString >::iterator it = csv_report.begin();
                  it != csv_report.end();
                  ++it ) {
               QString this_tag  = prefix  + it->first;
               QString this_tag2 = prefix2 + it->first;
               double  value    = it->second.toDouble();

               if ( sliding_results.count( this_tag ) ) {
                  sliding_results[ this_tag  ] += value;
                  sliding_results[ this_tag2 ] += value * value;
               } else {
                  sliding_results[ this_tag  ] = value;
                  sliding_results[ this_tag2 ] = value * value;
               }
            }
         }
         count++;
      }

      sliding_results[ prefixcnt ] = (double) count;

      if ( !count ) {
         errors = "Brookes map sliding cluster analysis: internal error count is zero?";
         return false;
      }         

      if ( !parameters.count( "sliding_baseline_mode" ) ) {
         for ( map < QString, QString >::iterator it = csv_report.begin();
               it != csv_report.end();
               ++it ) {
            QString this_tag  = prefix  + it->first;
            QString this_tag2 = prefix2 + it->first;

            double countinv   = 1e0 / (double) count;
            if ( count > 1 ) {
               double countm1inv = 1e0 / ((double) count - 1 );
               sliding_results[ this_tag2 ] = sqrt( countm1inv * ( sliding_results[ this_tag2 ] - countinv * sliding_results[ this_tag ] * sliding_results[ this_tag ] ) );
            } else {
               sliding_results.erase( this_tag2 );
            }
            sliding_results[ this_tag ] *= countinv;
         }
      }
   }

   parameters[ "alpha" ] = save_alpha;
   if ( parameters.count( "alpha_over_5" ) ) {
      parameters.erase( "alpha_over_5" );
   }

   return true;
}

bool ClusterAnalysis::run(
                           vector < vector < double > >  & pvaluepairs,
                           map < QString, QString >      & parameters,
                           map < QString, QString >      & csv_report
                           ) {
   double alpha        = parameters.count( "alpha" ) ? parameters[ "alpha" ].toDouble() : 0.05;
   double alpha_over_5 = parameters.count( "alpha_over_5" ) ? parameters[ "alpha_over_5" ].toDouble() : 0.2 * alpha;

   int pc = (int) pvaluepairs.size();

   if ( pc != (int) pvaluepairs[ 0 ].size() ) {
      errors = "Internal error: cormap of brookes selected, but pvaluepairs not square";
      return false;
   }

   // cout << QString( "cluster analysis pc = %1\n" ).arg( pc );

   cluster_data  .clear( );
   cluster_marked.clear( );

   // build above diagonal of rows

   int hb_count_red    = 0;
   int hb_count_points = 0;

   ca_index_pair x;
   for ( int i = 0; i < pc - 1; ++i ) {
      for ( int j = i + 1; j < pc; ++j ) {
         x.r = i;
         x.c = j;
         if ( pvaluepairs[ i ][ j ] >= alpha_over_5 ) {
            cluster_data[ x ] = 1;
         } else {
            cluster_data[ x ] = -1;
            ++hb_count_red;
         }
         ++hb_count_points;
      }
   }

   csv_report[ "% red pairs" ] = QString( "" ).sprintf( "%.2f", hb_count_points ? 100.0 * ( (double) hb_count_red / (double) hb_count_points ) : 0e0 );

   // map < int, int >            cluster_size_histogram;
   cluster_size_histogram.clear( );
   map < ca_index_pair, int > cluster_sizes;
   map < int, ca_index_pair > cluster_size_to_pos;

   double avg_cluster_size = 0e0;
   double sum2_cluster_size = 0e0;
   double avg_cluster_size_pct = 0e0;
   double sum2_cluster_size_pct = 0e0;

   double dpc = (double) pc;
   double area = ( dpc * dpc - dpc ) * 0.5;

   for ( int i = 0; i < pc - 1; ++i ) {
      for ( int j = i + 1; j < pc; ++j ) {
         x.r = i;
         x.c = j;
         
         // cout << QString( "loop checking point [%1,%2]\n" ).arg( x.r ).arg( x.c );

         if ( !cluster_marked.count( x ) && cluster_data[ x ] == -1 ) {
            cluster_red   .clear( );
            cluster_red   .insert( x );
            cluster_marked.insert( x );
            cluster_expand( x );
            cluster_sizes[ x ] = (int) cluster_red.size();
            avg_cluster_size += (double) cluster_red.size();
            sum2_cluster_size += (double) cluster_red.size() * (double) cluster_red.size();
            if ( area > 0 ) {
               double this_cluster_size_pct = 100.0 * (double) cluster_red.size() / area;
               avg_cluster_size_pct += this_cluster_size_pct;
               sum2_cluster_size_pct += this_cluster_size_pct * this_cluster_size_pct;
            }

            if ( !cluster_size_to_pos.count( (int) cluster_red.size() ) ) {
               cluster_size_to_pos[ (int) cluster_red.size() ] = x;
            }
            cluster_size_histogram[ (int) cluster_red.size() ] =
               1 + 
               ( cluster_size_histogram.count( (int) cluster_red.size() ) ?
                 cluster_size_histogram[ (int) cluster_red.size() ] : 0 );
         }
               
      }
   }

   double avg_cluster_size_sd = 0e0;
   double avg_cluster_size_sd_as_pct = 0e0;
   double avg_cluster_size_pct_sd = 0e0;

   if ( cluster_sizes.size() ) {
      double countinv   = 1e0 / (double) cluster_sizes.size();
      if ( cluster_sizes.size() > 2 ) {
         double countm1inv = 1e0 / ((double) cluster_sizes.size() - 1 );
         avg_cluster_size_sd     = sqrt( countm1inv * ( sum2_cluster_size     - countinv * avg_cluster_size * avg_cluster_size ) );
         avg_cluster_size_pct_sd = sqrt( countm1inv * ( sum2_cluster_size_pct - countinv * avg_cluster_size_pct * avg_cluster_size_pct ) );
         if ( avg_cluster_size > 0 ) {
            avg_cluster_size_sd_as_pct = 100.0 * avg_cluster_size_sd / (avg_cluster_size * countinv);
         }
      }
      avg_cluster_size     *= countinv;
      avg_cluster_size_pct *= countinv;
   }

   // build cluster report
   {
      // find max size

      int max_cluster_size =  
         cluster_size_histogram.rbegin() != cluster_size_histogram.rend() ?
         cluster_size_histogram.rbegin()->first : 0 
         ;

      double avg_cluster_size_pct =
         area > 0 ?
         100e0 * (double) avg_cluster_size / area : 0e0;

      double max_cluster_size_pct =
         area > 0 ?
         100e0 * (double) max_cluster_size / area : 0e0;

      parameters[ "clusterheader" ] =
         cluster_sizes.size() ?
         QString( "Red cluster count %1, average size %2 \u00b1%3 %4, average size as pct of total area %5\% \u00b1%6\n"
                  "Red cluster maximum size %7 (%8\%)%9.\n" )
         .arg( cluster_sizes.size() )
         .arg( QString( "" ).sprintf( "%.2f", avg_cluster_size ) )
         .arg( QString( "" ).sprintf( "%.2f", avg_cluster_size_sd ) )
         .arg( avg_cluster_size > 0 ? QString( "" ).sprintf( "(%.1f%%)", avg_cluster_size_sd_as_pct ) : QString( "" ) )
         .arg( QString( "" ).sprintf( "%.1f", avg_cluster_size_pct ) )
         .arg( QString( "" ).sprintf( "%.1f", avg_cluster_size_pct_sd ) )
         .arg( max_cluster_size )
         .arg( QString( "" ).sprintf( "%3.1f", max_cluster_size_pct ) )
         .arg( max_cluster_size && cluster_size_to_pos.count( max_cluster_size ) 
               ?
               QString( " has %1 occurrence%2 and %3begins at [%4,%5]" )
               .arg( cluster_size_histogram.rbegin()->second )
               .arg( cluster_size_histogram.rbegin()->second > 1 ? "s" : "" )
               .arg( cluster_size_histogram.rbegin()->second > 1 ? "first occurrence " : "" )
               .arg( cluster_size_to_pos[ max_cluster_size ].r + 1)
               .arg( cluster_size_to_pos[ max_cluster_size ].c + 1) 
               : QString( "" ) )
         
         :
         QString( "No red clusters found.\n" )
         ;
      
      csv_report[ "Red cluster size average"                         ] = QString( "" ).sprintf( "%.3f", avg_cluster_size );
      csv_report[ "Red cluster size average sd"                      ] = QString( "" ).sprintf( "%.3f", avg_cluster_size_sd );
      csv_report[ "Red cluster size average sd as pct"               ] = QString( "" ).sprintf( "%.3f", avg_cluster_size_sd_as_pct );
      csv_report[ "Red cluster size average as pct of total area"    ] = QString( "" ).sprintf( "%.3f", avg_cluster_size_pct );
      csv_report[ "Red cluster size average as pct of total area sd" ] = QString( "" ).sprintf( "%.3f", avg_cluster_size_pct_sd );
      csv_report[ "Red cluster size maximum size"                    ] = QString( "%1" ).arg( max_cluster_size );
      csv_report[ "Red cluster size maximum size as pct"             ] = QString( "" ).sprintf( "%.3f", max_cluster_size_pct );
   }
   return true;
}
