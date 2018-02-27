#include "pvpanalysis.h"

double PVPAnalysis::holm_bonferroni( vector < double > P, double alpha ) {
   vector < double > Porg = P;

   std::sort( P.begin(), P.end() );

   int m = (int) P.size();

   for ( int k = 0; k < m; ++k ) {
      double kalpha = alpha / ( m - k );
      if ( P[ k ] > kalpha ) {
         return kalpha;
      }
   }

   return 1e0;
}

bool PVPAnalysis::compute(
             map < QString, QString >     & parameters,
             vector < vector < double > > & pvaluepairs,
             vector < vector < double > > & adjpvaluepairs,
             vector < QString >           & selected_files
             ) {
   this->pparameters                          = &parameters;
   this->pvaluepairs                          = pvaluepairs;
   this->adjpvaluepairs                       = adjpvaluepairs;
   this->selected_files                       = selected_files;

   alpha        = parameters.count( "alpha" ) ? parameters[ "alpha" ].toDouble() : 0.05;
   alpha_over_5 = parameters.count( "alpha_over_5" ) ? parameters[ "alpha_over_5" ].toDouble() : 0.2 * alpha;

   QString pvdefmsg =
      QString( 
              "Alpha is %1\n\n"
              "Pairwise P value map color definitions:\n"
              "  P is the pairwise P value as determined by a CorMap analysis\n"
              "  Green corresponds to         P >= %2\n" 
              "  Yellow corresponds to %3 > P >= %4\n" 
              "  Red corresponds to    %5 > P\n"
               )
      .arg( alpha_over_5 )
      .arg( alpha )
      .arg( alpha )
      .arg( alpha_over_5 )
      .arg( alpha_over_5 )
      ;

   if ( parameters.count( "as_pairs" ) ) {
      pvdefmsg += "Axis ticks correspond to Ref. as listed below\n";
   } else {
      pvdefmsg += "Axes ticks correspond to Ref. as listed below\n";
   }

   pvdefmsg += "\n";
   QString msg;
   QString msg_headers;

   csv_headers 
      << ( parameters.count( "csv_id_header" ) ? parameters[ "csv_id_header" ] : QString( "" ) )
      << "Average one-to-all P value"
      << "Average one-to-all P value sd"
      << "Red points pct"
      << "Red contiguous points average P value"
      << "Red contiguous points average P value sd"
      << "Red contiguous points average pct"
      << "Red contiguous points maximum pct"
      << "Red cluster size average"
      << "Red cluster size average sd"
      << "Red cluster size average sd as pct"
      << "Red cluster size average as pct of total area"
      << "Red cluster size average as pct of total area sd"
      << "Red cluster size maximum size"
      << "Red cluster size maximum size as pct"
      ;

   if ( parameters.count( "save_png" ) ) {
      csv_headers << "Image file";
      csv_report[ "Image file" ] = "\"" + QFileInfo( parameters[ "save_png" ] ).fileName() + "\"";
   }
   
   csv_report[ parameters.count( "csv_id_header" ) ? parameters[ "csv_id_header" ] : QString( "" ) ] =
      parameters.count( "csv_id_data" ) ? parameters[ "csv_id_data" ] : QString( "" );

   int sfs = (int) selected_files.size();

   QString fileheader = parameters.count( "fileheader" ) ? parameters[ "fileheader" ] : "Name";

   int max_file_name_len = fileheader.length() + 1;

   for ( int i = 0; i < sfs; ++i ) {
      if ( max_file_name_len < (int) selected_files[ i ].length() ) {
         max_file_name_len = (int) selected_files[ i ].length();
      }
   }

   // ! as_pairs
   {
      int green_c  = 0;
      int yellow_c = 0;
      int red_c    = 0;

      int adj_green_c  = 0;
      int adj_yellow_c = 0;
      int adj_red_c    = 0;

      int hb_green_c  = 0;
      int hb_yellow_c = 0;
      int hb_red_c    = 0;

      int pc = (int) pvaluepairs.size();
      qi     = new QImage( pc, pc, QImage::Format_RGB32 );
      qi_adj = new QImage( pc, pc, QImage::Format_RGB32 );
      qi_hb  = new QImage( pc, pc, QImage::Format_RGB32 );

      // compute HB alpha's
      {
         vector < double > P;
         for ( int i = 0; i < pc; ++i ) {
            if ( i + 1 < pc ) {
               for ( int j = i + 1; j < pc; ++j ) {
                  P.push_back( pvaluepairs[ i ][ j ] );
               }
            }
         }
         
         hb_alpha        = holm_bonferroni( P, alpha );
         hb_alpha_over_5 = holm_bonferroni( P, alpha_over_5 );
      }

      for ( int i = 0; i < pc; ++i ) {
         qi    ->setPixel( i, i, qRgb( 255, 255, 255 ) );
         qi_adj->setPixel( i, i, qRgb( 255, 255, 255 ) );
         qi_hb ->setPixel( i, i, qRgb( 255, 255, 255 ) );

         if ( i + 1 < pc ) {
            for ( int j = i + 1; j < pc; ++j ) {
               if ( pvaluepairs[ i ][ j ] >= alpha ) {
                  green_c++;
                  qi->setPixel( i, j, qRgb( 0, 255, 0 ) );
                  qi->setPixel( j, i, qRgb( 0, 255, 0 ) );
               } else {
                  if ( pvaluepairs[ i ][ j ] >= alpha_over_5 ) {
                     yellow_c++;
                     qi->setPixel( i, j, qRgb( 255, 255, 0 ) );
                     qi->setPixel( j, i, qRgb( 255, 255, 0 ) );
                  } else {
                     red_c++;
                     qi->setPixel( i, j, qRgb( 255, 0, 0 ) );
                     qi->setPixel( j, i, qRgb( 255, 0, 0 ) );
                  }
               }
               if ( adjpvaluepairs[ i ][ j ] >= alpha ) {
                  adj_green_c++;
                  qi_adj->setPixel( i, j, qRgb( 0, 255, 0 ) );
                  qi_adj->setPixel( j, i, qRgb( 0, 255, 0 ) );
               } else {
                  if ( adjpvaluepairs[ i ][ j ] >= alpha_over_5 ) {
                     adj_yellow_c++;
                     qi_adj->setPixel( i, j, qRgb( 255, 255, 0 ) );
                     qi_adj->setPixel( j, i, qRgb( 255, 255, 0 ) );
                  } else {
                     adj_red_c++;
                     qi_adj->setPixel( i, j, qRgb( 255, 0, 0 ) );
                     qi_adj->setPixel( j, i, qRgb( 255, 0, 0 ) );
                  }
               }
               if ( pvaluepairs[ i ][ j ] >= hb_alpha ) {
                  hb_green_c++;
                  qi_hb->setPixel( i, j, qRgb( 0, 255, 0 ) );
                  qi_hb->setPixel( j, i, qRgb( 0, 255, 0 ) );
               } else {
                  if ( pvaluepairs[ i ][ j ] >= hb_alpha_over_5 ) {
                     hb_yellow_c++;
                     qi_hb->setPixel( i, j, qRgb( 255, 255, 0 ) );
                     qi_hb->setPixel( j, i, qRgb( 255, 255, 0 ) );
                  } else {
                     hb_red_c++;
                     qi_hb->setPixel( i, j, qRgb( 255, 0, 0 ) );
                     qi_hb->setPixel( j, i, qRgb( 255, 0, 0 ) );
                  }
               }
            }
         }
   
         if ( parameters.count( "size" ) &&
              parameters.count( "outname" ) ) {
            QImage *qi_use;
            if ( parameters.count( "hb" ) ) {
               qi_use = qi_hb;
            } else {
               qi_use = qi;
            }

            QString outfilename =
               parameters[ "outname" ]
               + ( parameters.count( "hb" ) ? "_hb" : "" )
               + ".png";

            
            if ( !parameters.count( "force" ) &&
                 QFile::exists( outfilename ) ) {
               errors += "file '" + outfilename + "' exists. Please remove or use the --force option";
               return false;
            }

            qi_use
               ->scaled( QSize( parameters[ "size" ].toInt(),
                                parameters[ "size" ].toInt() ),
                        Qt::KeepAspectRatio )
               .save( outfilename, "PNG" )
               ;
         }
      }

      double tot_c_pct     =  100e0 / (double) ( green_c + yellow_c + red_c );

      if ( parameters.count( "hb" ) ) {
         pvdefmsg =
            QString( 
                    "Alpha is %1\n\n"
                    "Holm-Bonferroni pairwise P value map color definitions:\n"
                    "  P is the pairwise P value as determined by a CorMap analysis\n"
                    "  Green corresponds to              P >= %2\n" 
                    "  Yellow corresponds to %3 > P >= %4\n" 
                    "  Red corresponds to    %5 > P\n"
                     )
            .arg( alpha_over_5 )
            .arg( QString("").sprintf( "%6.4g", hb_alpha ) )
            .arg( QString("").sprintf( "%6.4g", hb_alpha ) )
            .arg( QString("").sprintf( "%6.4g", hb_alpha_over_5 ) )
            .arg( QString("").sprintf( "%6.4g", hb_alpha_over_5 ) )
            ;

         pvdefmsg += "Axes ticks correspond to Ref. as listed below\n";
         pvdefmsg += "\n";
      }      
      
      msg += pvdefmsg;

      if ( !parameters.count( "hb" ) ) {
         msg += QString("").sprintf(
                                    "P values:\n"
                                    " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                    " %5.1f%% red pairs\n"
                                    ,tot_c_pct * (double) (green_c + yellow_c )
                                    ,tot_c_pct * (double) green_c
                                    ,tot_c_pct * (double) yellow_c
                                    ,tot_c_pct * (double) red_c
                                    )
            ;
      }

      msg_headers += pvdefmsg;
      
      if ( !parameters.count( "hb" ) ) {
         msg_headers += QString("").sprintf(
                                            "P values:\n"
                                            " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                            " %5.1f%% red pairs\n"
                                            ,tot_c_pct * (double) (green_c + yellow_c )
                                            ,tot_c_pct * (double) green_c
                                            ,tot_c_pct * (double) yellow_c
                                            ,tot_c_pct * (double) red_c
                                            )
            ;
      }

      csv_report[ "Red points pct" ] = QString( "" ).sprintf( "%.2f", tot_c_pct * (double) red_c );

      if ( !parameters.count( "hide_adjpvalues" ) ) {
         msg += QString("").sprintf(
                                    "Adjusted P values:\n"
                                    " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                    " %5.1f%% red pairs\n"
                                    ,tot_c_pct * (double) (adj_green_c + adj_yellow_c )
                                    ,tot_c_pct * (double) adj_green_c
                                    ,tot_c_pct * (double) adj_yellow_c
                                    ,tot_c_pct * (double) adj_red_c
                                    )
         ;
      }

      if ( !parameters.count( "hide_hb_pvalues" ) ) {
         if ( parameters.count( "hb" ) ) {
            msg += QString("").sprintf(
                                       "Holm-Bonferroni adjusted P values:\n"
                                       // "Yellow HB P value cutoff %.3g Red HB P value cutoff %.3g\n"
                                       " %5.1f%% green (%.1f%%) + yellow (%.1f%%) pairs\n"
                                       " %5.1f%% red pairs\n"
                                       // ,hb_alpha
                                       // ,hb_alpha_over_5
                                       ,tot_c_pct * (double) (hb_green_c + hb_yellow_c )
                                       ,tot_c_pct * (double) hb_green_c
                                       ,tot_c_pct * (double) hb_yellow_c
                                       ,tot_c_pct * (double) hb_red_c
                                       )
               ;
         }
         parameters[ "hb_alpha"        ] = QString( "%1" ).arg( hb_alpha );
         parameters[ "hb_alpha_over_5" ] = QString( "%1" ).arg( hb_alpha_over_5 );
      }
   }
      
   if ( parameters.count( "clusteranalysis" ) ) {
      if ( parameters.count( "hb" ) ) {
         QString save_alpha = parameters[ "alpha" ];
         
         parameters[ "alpha"        ] = parameters[ "hb_alpha" ];
         parameters[ "alpha_over_5" ] = parameters[ "hb_alpha_over_5" ];

         cluster_analysis();

         parameters[ "alpha" ] = save_alpha;
         parameters.erase( "alpha_over_5" );
      } else {
         cluster_analysis();
      }         
   }

   if ( parameters.count( "linewisesummary" ) ) {
      // build summary report per line

      if ( sfs != (int) pvaluepairs.size() ||
           sfs != (int) pvaluepairs[ 0 ].size() ) {
         msg += QString( "Internal error, cormap utility can't compute linewise summary %1 != %2 or !%3\n" )
            .arg( sfs )
            .arg( (int) pvaluepairs.size() )
            .arg( (int) pvaluepairs[0].size() )
            ;
      } else {
         QString linereport;

         linereport += 
            QString( "\n Ref. : %1   Avg. P value    Min. P Value      \% Red\n" )
            .arg( fileheader, -max_file_name_len );

         double avg_avgP    = 0e0;
         double sum2_avgP   = 0e0;
         double avg_pct_red = 0e0;
         double max_pct_red = 0e0;
         double sum2_pct_red = 0e0;

         vector < double > plot_pos;
         vector < double > plot_redpct;
         vector < double > plot_P;

         // add sd's to computation

         double use_alpha_over_5 = parameters.count( "hb" ) ? hb_alpha_over_5 : alpha_over_5;

         for ( int i = 0; i < sfs; ++i ) {

            double avgP = 0e0;
            double minP = 1e0;
            int red_count = 0;

            for ( int j = 0; j < sfs; ++j ) {
               if ( j != i ) {
                  avgP += pvaluepairs[ i ][ j ];
                  if ( minP > pvaluepairs[ i ][ j ] ) {
                     minP = pvaluepairs[ i ][ j ];
                  }
                  if ( pvaluepairs[ i ][ j ] < use_alpha_over_5 ) {
                     red_count++;
                  }
               }
            }

            avgP /= (double) (sfs - 1);

            double pct_red = 100e0 * (double) red_count / (double) ( sfs - 1 );

            avg_avgP     += avgP;
            sum2_avgP    += avgP * avgP;
            avg_pct_red  += pct_red;
            sum2_pct_red += pct_red * pct_red;

            linereport += 
               QString( "%1 : %2     %3    %4    %5\%\n" )
               .arg( i + 1, 5 )
               .arg( selected_files[ i ], -max_file_name_len )
               .arg( QString( "" ).sprintf( "%.4g", avgP ).leftJustified( 12 ) )
               .arg( QString( "" ).sprintf( "%.4g", minP ).leftJustified( 12 ) )
               .arg( QString( "" ).sprintf( "%3.1f", pct_red ), 5 )
               ;

            plot_pos   .push_back( (double) ( i + 1 ) );
            plot_redpct.push_back( pct_red );
            plot_P     .push_back( avgP );

            if ( max_pct_red < pct_red ) {
               max_pct_red = pct_red;
            }
         }
         linereport += "\n"; 

         double countinv   = 1e0 / (double) sfs;
         double avgP_sd;
         double pct_red_sd = 0e0;

         if ( sfs > 2 ) {
            double countm1inv = 1e0 / ((double) sfs - 1 );
            avgP_sd    = sqrt( countm1inv * ( sum2_avgP    - countinv * avg_avgP    * avg_avgP    ) );
            pct_red_sd = sqrt( countm1inv * ( sum2_pct_red - countinv * avg_pct_red * avg_pct_red ) );

            avg_avgP    *= countinv;
            avg_pct_red *= countinv;

            msg += QString( "\nAverage one-to-all P value %1 \u00b1%2 %3 \% red %4\% \u00b1%5 %6\n" )
               .arg( QString( "" ).sprintf( "%.4g", avg_avgP ) )
               .arg( QString( "" ).sprintf( "%.4g", avgP_sd ) )
               .arg( avg_avgP > 0 ? QString( "" ).sprintf( "(%.1f%%)", 100.0 * avgP_sd / avg_avgP ) : QString( "" ) )
               .arg( QString( "" ).sprintf( "%3.1f", avg_pct_red ) )
               .arg( QString( "" ).sprintf( "%3.1f", pct_red_sd ) )
               .arg( avg_pct_red > 0 ? QString( "" ).sprintf( "(%.1f%%)", 100.0 * pct_red_sd / avg_pct_red ) : QString( "" ) )
               + ( parameters.count( "clusterheader" ) ? parameters[ "clusterheader" ] : "" )
               + linereport;

            msg_headers += 
               QString( "\nAverage one-to-all P value %1 \u00b1%2 %3 \% red %4\% \u00b1%5 %6\n" )
               .arg( QString( "" ).sprintf( "%.4g", avg_avgP ) )
               .arg( QString( "" ).sprintf( "%.4g", avgP_sd ) )
               .arg( avg_avgP > 0 ? QString( "" ).sprintf( "(%.1f%%)", 100.0 * avgP_sd / avg_avgP ) : QString( "" ) )
               .arg( QString( "" ).sprintf( "%3.1f", avg_pct_red ) )
               .arg( QString( "" ).sprintf( "%3.1f", pct_red_sd ) )
               .arg( avg_pct_red > 0 ? QString( "" ).sprintf( "(%.1f%%)", 100.0 * pct_red_sd / avg_pct_red ) : QString( "" ) )
               + ( parameters.count( "clusterheader" ) ? parameters[ "clusterheader" ] : "" )
               ;

            csv_report[ "Average one-to-all P value" ]    = QString( "" ).sprintf( "%.4g", avg_avgP );
            csv_report[ "Average one-to-all P value sd" ] = QString( "" ).sprintf( "%.4g", avgP_sd );

         } else {
            avg_avgP    *= countinv;
            avg_pct_red *= countinv;

            msg += QString( "   average one-to-all P %1 red %2\%\n" )
               .arg( QString( "" ).sprintf( "%.4g", avg_avgP ) )
               .arg( QString( "" ).sprintf( "%3.1f", avg_pct_red ) )
               + ( parameters.count( "clusterheader" ) ? parameters[ "clusterheader" ] : "" )
               + linereport;

            msg_headers += 
               QString( "   average one-to-all P %1 red %2\%\n" )
               .arg( QString( "" ).sprintf( "%.4g", avg_avgP ) )
               .arg( QString( "" ).sprintf( "%3.1f", avg_pct_red ) )
               + ( parameters.count( "clusterheader" ) ? parameters[ "clusterheader" ] : "" );

            csv_report[ "Average one-to-all P value" ]    = QString( "" ).sprintf( "%.4g", avg_avgP );
         }

#if defined( ENABLE_PLOTTING )
         // plot data

         {
            int use_line_width = parameters.count( "linewidth" ) ? parameters[ "linewidth" ].toInt() : 1;

            {
               QwtPlotCurve *curve = new QwtPlotCurve( "pctred" );
               curve->setStyle( QwtPlotCurve::Sticks );
               curve->setPen( QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
               curve->setSamples(
                              (double *)&plot_pos[ 0 ],
                              (double *)&plot_redpct[ 0 ],
                              plot_pos.size()
                              );
               curve->attach( plot );
            }

            {

               double x[2];
               double y[2];

               x[0] = plot_pos.front() - 1;
               x[1] = plot_pos.back() + 1;

               {
                  y[0] = y[1] = avg_pct_red;

                  QwtPlotCurve *curve = new QwtPlotCurve( "avgred" );
                  curve->setStyle( QwtPlotCurve::Lines );

                  curve->setPen( QPen( Qt::green, use_line_width, Qt::DotLine ) );
                  curve->setSamples( x, y, 2 );
                  curve->attach( plot );
               }

               if ( sfs > 2 ) {
                  {
                     y[0] = y[1] = avg_pct_red + pct_red_sd;;

                     QwtPlotCurve *curve = new QwtPlotCurve( "sdredplus" );
                     curve->setStyle( QwtPlotCurve::Lines );
                     curve->setPen( QPen( Qt::yellow, use_line_width, Qt::DotLine ) );
                     curve->setSamples( x, y, 2 );
                     curve->attach( plot );
                  }
                  {
                     y[0] = y[1] = avg_pct_red - pct_red_sd;

                     QwtPlotCurve *curve = new QwtPlotCurve( "sdredminus" );
                     curve->setStyle( QwtPlotCurve::Lines );
                     curve->setPen( QPen( Qt::yellow, use_line_width, Qt::DotLine ) );
                     curve->setSamples( x, y, 2 );
                     curve->attach( plot );
                  }
               }
            }

            if ( !plot_zoomer )
            {
               plot->setAxisScale( QwtPlot::xBottom, plot_pos.front() - 1, plot_pos.back() + 1 );
               plot->setAxisScale( QwtPlot::yLeft  , 0, max_pct_red * 1.1 );
               plot_zoomer = new ScrollZoomer(plot->canvas());
               plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
            }

            plot->replot();
            plot->show();
         }
#endif // ENABLE_PLOTTING
      }
   }

   parameters[ "report" ] = msg;

   if ( parameters.count( "save_csv" ) ) {
      QString out;
      if ( !parameters.count( "csv_skip_report_header" ) ) {
         for ( int i = 0; i < (int) csv_headers.size(); ++i ) {
            out += QString( "%1\"%2\"" ).arg( i ? "," : "" ).arg( csv_headers[ i ] );
         }
         out += "\n";
      }
      for ( int i = 0; i < (int) csv_headers.size(); ++i ) {
         out += ( i ? "," : "" ) 
            + ( csv_report.count( csv_headers[ i ] ) ?
                csv_report[ csv_headers[ i ] ] : "" );
      }
      out += "\n";

      
      QFile f( parameters[ "save_csv" ] );
      if ( f.open( QIODevice::WriteOnly | QIODevice::Append ) ) {
         QTextStream tso( &f );
         tso << out;
         f.close();
      }
      // cout << out;
   }

   return true;
}

   
bool PVPAnalysis::cluster_analysis() {
   ClusterAnalysis ca;

   if ( ca.run( pvaluepairs,
                *pparameters,
                csv_report ) ) {
      
#if defined( ENABLE_PLOTTING )
      // assemble plot
      if ( ca.cluster_size_histogram.size() ) {
         vector < double > cluster_hist_x;
         vector < double > cluster_hist_y;
         double max_y = 0e0;
         for ( map < int, int >::iterator it = ca.cluster_size_histogram.begin();
               it != ca.cluster_size_histogram.end();
               ++it ) {
            cluster_hist_x.push_back( (double ) it->first );
            cluster_hist_y.push_back( (double ) it->second );
            if ( max_y < it->second ) {
               max_y = it->second;
            }
         }

         {
            int use_line_width = pparameters->count( "linewidth" ) ? (*pparameters)[ "linewidth" ].toInt() : 1;

            {
               QwtPlotCurve *curve = new QwtPlotCurve( "pctred" );
               curve->setStyle( QwtPlotCurve::Sticks );
               curve->setPen( QPen( Qt::red, 2 * use_line_width, Qt::SolidLine ) );
               curve->setSamples(
                              (double *)&cluster_hist_x[ 0 ],
                              (double *)&cluster_hist_y[ 0 ],
                              cluster_hist_x.size()
                              );
               curve->attach( plot_cluster );
            }

            if ( !plot_cluster_zoomer )
            {
               plot_cluster->setAxisScale( QwtPlot::xBottom, cluster_hist_x.front() - 1, cluster_hist_x.back() + 1 );
               plot_cluster->setAxisScale( QwtPlot::yLeft  , 0, max_y * 1.1 );
               plot_cluster_zoomer = new ScrollZoomer(plot_cluster->canvas());
               plot_cluster_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
            }

         }
      } else {
         if ( !plot_cluster_zoomer )
         {
            plot_cluster->setAxisScale( QwtPlot::xBottom, 0, 1 );
            plot_cluster->setAxisScale( QwtPlot::yLeft  , 0, 1 );
            plot_cluster_zoomer = new ScrollZoomer(plot_cluster->canvas());
            plot_cluster_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
         }

      }

      plot_cluster->replot();
      plot_cluster->show();
#endif
      return true;
   }
   return false;
}
