#include <sasfiles.h>
#include <pvpanalysis.h>

bool SASFiles::load( const QStringList & infiles, bool quiet ) {
   // load up the files into I,q,e
   reset_messages();

   files = reorder( infiles );

   if ( !files.size() ) {
      errors << "no files specified?";
      return false;
   }

   QTextStream out( stdout );

   // read the files

   {
      bool any_errors = false;
   
      for ( int i = 0; i < (int) files.size(); ++i ) {
         any_errors |= !read_one_file( files[ i ] );
         if ( !quiet ) {
            out << files[ i ] << "\n";
            out.flush();
         }
      }

      if ( any_errors ) {
         return false;
      }
      if ( !quiet ) {
         out << QString( "%1 files loaded\n" ).arg( files.size() );
         out.flush();
      }
   }

   return true;
}

// --------- file read utilities --------

bool SASFiles::read_one_file( const QString & filename ) {
   QFile f( filename );
   if ( !f.exists() ) {
      errors << QString("Error: file %1 does not exist").arg( filename );
      return false;
   }
   // cout << QString( "opening %1\n" ).arg( filename ) << flush;
   
   QString ext = QFileInfo( filename ).suffix().toLower();

   QRegExp rx_valid_ext (
                         "^("
                         "dat|"
                         "int|"
                         "txt|"
                         "csv|"
                         "iq|"
                         "ciq|"
                         // "out|"
                         "ssaxs)$" );

   if ( rx_valid_ext.indexIn( ext ) == -1 ) {
      errors << QString("Error: file %1 unsupported file extension %2").arg( filename ).arg( ext );
      return false;
   }
      
   if ( !f.open( QIODevice::ReadOnly ) ) {
      errors << QString("Error: can not open file %1, check permissions ").arg( filename );
      return false;
   }

   QTextStream ts(&f);
   vector < QString > qv;
   QStringList qsl;

   while ( !ts.atEnd() ) {
      QString qs = ts.readLine();
      qv.push_back( qs );

      qsl << qs;
   }
   f.close();

   if ( !qv.size() ) {
      errors << QString("Error: the file %1 is empty ").arg( filename );
      return false;
   }

   bool is_time = false;

   double this_conc = 0e0;
   double this_psv  = 0e0;
   double this_I0se = 0e0;
   bool   has_time = false;
   double this_time = 0e0;

   if ( ext == "dat" )
   {
      QRegExp rx_conc      ( "Conc:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_psv       ( "PSV:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_I0se      ( "I0se:\\s*(\\S+)(\\s|$)" );
      QRegExp rx_time      ( "Time:\\s*(\\S+)(\\s|$)" );
      if ( rx_conc.indexIn( qv[ 0 ] ) )
      {
         this_conc = rx_conc.cap( 1 ).toDouble();
      }
      if ( rx_psv.indexIn( qv[ 0 ] ) )
      {
         this_psv = rx_psv.cap( 1 ).toDouble();
      }
      if ( rx_I0se.indexIn( qv[ 0 ] ) )
      {
         this_I0se = rx_I0se.cap( 1 ).toDouble();
      }
      if ( rx_time.indexIn( qv[ 0 ] ) )
      {
         has_time = true;
         this_time = rx_time.cap( 1 ).toDouble();
      }
   }

   // we should make some configuration for matches & offsets or column mapping
   // just an ad-hoc fix for APS 5IDD
   int q_offset   = 0;
   int I_offset   = 1;
   int e_offset   = 2;
   int row_offset = 1;
   if ( ( ext == "dat" || ext == "txt" ) && qv[ 0 ].toLower().contains( QRegExp( "frame\\s*data" ) ) ) {
      is_time = true;
   }

   // load csv columns as time curves
   if ( ext == "csv" ) {
      // first column is time
      qv[ 0 ].replace( "(", "" ).replace( ")", "" ).replace( "/", "_per_" ).replace( QRegExp( "\\s+" ), "_" ).replace( ":", "_" ).replace( QRegExp( "\\_+" ), "_" ) ;

      QStringList headers = (qv[ 0 ] ).split( "," , QString::SkipEmptyParts );
      
      if ( !headers.size() ||
           !headers[ 0 ].toLower().contains( "time" ) ) {
         errors << "The first line, first column of the .csv file must contain 'time'";
         return false;
      }

      vector < vector < double > > csv_data;
      vector < double > q;
      vector < QString > q_string;
      for ( int i = 1; i < (int) qv.size(); i++ ) {
         QStringList data = (qv[ i ] ).split( "," , QString::SkipEmptyParts );
         vector < double > this_csv_data;
         if ( data.size() ) {
            q.push_back( data[ 0 ].toDouble() );
            q_string.push_back( data[ 0 ] );
         }
         for ( int j = 1; j < (int) data.size(); j++ ) {
            this_csv_data.push_back( data[ j ].toDouble() );
         }
         csv_data.push_back( this_csv_data );
      }

      map < QString, bool > current_files;

      for ( int i = 1; i < (int) headers.size(); i++ ) {
         QString name = headers[ i ];
         unsigned int ext = 0;
         while ( current_files.count( name ) ) {
            name = headers[ i ] + QString( "-%1" ).arg( ++ext );
         }
         vector < double > I;
         vector < double > use_q;
         vector < QString > use_q_string;
         for ( int j = 0; j < (int) csv_data.size(); j++ ) {
            if ( i - 1 < (int) csv_data[ j ].size() ) {
               I.push_back( csv_data[ j ][ i - 1 ] );
               use_q.push_back( q[ j ] );
               use_q_string.push_back( q_string[ j ] );
            }
         }

         if ( I.size() ) {
            f_pos       [ name ] = f_qs.size();
            f_qs_string [ name ] = use_q_string;
            f_qs        [ name ] = use_q;
            f_Is        [ name ] = I;
            f_is_time   [ name ] = true;
            f_psv       [ name ] = 0e0;
            f_I0se      [ name ] = 0e0;
            f_conc      [ name ] = 0e0;
         } else {
            warnings << QString( "csv file %1 column %2 \"%3\" doesn't seem to be complete, skipped" ).arg( filename ).arg( i + 1 ).arg( name );
         } 
      }

      return false;
   }

   if ( ext == "txt" && qv[ 0 ].contains( "# File Encoding (File origin in Excel)" ) ) {
      q_offset = 1;
      I_offset = 2;
      e_offset = 3;
      notices << "APS SIDD format";
   }      
   
   // ad-hoc for soleil hplc time/uv/conc data

   // cout << "load: <" << qv[ 0 ] << ">" << endl;

   if ( ext == "txt" && qv[ 0 ].contains( "temps depuis le debut" ) ) {
      I_offset   = 2;
      e_offset   = 10;
      row_offset = 4;
      is_time    = true;
      notices << "SOLEIL HPLC time/uv format";
   }      

   vector < QString > q_string;
   vector < double >  q;
   vector < double >  I;
   vector < double >  e;

   QRegExp rx_ok_line("^(\\s+(-|)|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
   rx_ok_line.setMinimal( true );
   for ( int i = row_offset; i < (int) qv.size(); i++ ) {
      if ( qv[i].contains(QRegExp("^#")) ||
           rx_ok_line.indexIn( qv[i] ) == -1 ) {
         continue;
      }
      
      // cout << "line: <" << qv[ i ] << ">" << endl;

      // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , QString::SkipEmptyParts ),""));
      QStringList tokens;
      {
         QString qs = qv[i].replace(QRegExp("^\\s+"),"");
         tokens = (qs ).split( QRegExp("\\s+") , QString::SkipEmptyParts );
      }

      if ( (int)tokens.size() > I_offset ) {
         QString this_q_string = tokens[ q_offset ];
         double this_q         = tokens[ q_offset ].toDouble();
         double this_I         = tokens[ I_offset ].toDouble();
         double this_e = 0e0;
         if ( (int)tokens.size() > e_offset) {
            this_e = tokens[ e_offset ].toDouble();
            if ( this_e < 0e0 ) {
               this_e = 0e0;
            }
         }
         if ( q.size() && this_q <= q[ q.size() - 1 ] ) {
            notices << QString("Internal note: breaking %1 %2\n").arg( this_q ).arg( q[ q.size() - 1 ] );
            break;
         }
         if ( is_time || !std::isnan( this_I ) ) {
            q_string.push_back( this_q_string );
            q       .push_back( this_q );
            I       .push_back( this_I );
            if ( (int)tokens.size() > e_offset ) // && this_e )
            {
               e.push_back( this_e );
            }
         }
      }
   }

   if ( !q.size() ) {
      errors << QString( "Error: File %1 has no data" ).arg( filename );
      return false;
   }
                  
   if ( is_zero_vector( I ) ) {
      errors << QString( "Error: File %1 has only zero signal" ).arg( filename );
      return false;
   }

   // cout << QString( "opened %1\n" ).arg( filename ) << flush;
   QString basename = QFileInfo( filename ).completeBaseName();
   f_name      [ basename ] = filename;
   f_pos       [ basename ] = f_qs.size();
   f_qs_string [ basename ] = q_string;
   f_qs        [ basename ] = q;
   f_Is        [ basename ] = I;
   if ( e.size() == q.size() ) {
      f_errors        [ basename ] = e;
   } else {
      if ( e.size() ) {
         warnings <<
            QString( "Notice: File %1 appeared to have standard deviations, but some were zero or less, so all were dropped" ).arg( filename );
      }
      f_errors    .erase( basename );
   }
   f_is_time    [ basename ] = is_time;
   f_conc       [ basename ] = this_conc;
   f_psv        [ basename ] = this_psv;
   f_I0se       [ basename ] = this_I0se;
   if ( has_time ) {
      f_time       [ basename ] = this_time;
   }
   return true;
}

bool SASFiles::is_zero_vector( const vector < double > &v ) {
   bool is_zero = true;
   for ( int i = 0; i < (int)v.size(); i++ ) {
      if ( v[ i ] != 0e0 ) {
         is_zero = false;
         break;
      }
   }
   return is_zero;
}

void SASFiles::reset_messages() {
   errors.clear();
   warnings.clear();
   notices.clear();
}   

// --------- file name sorting utilities --------

class sortable_qstring {
public:
   double       x;
   QString      name;
   bool operator < (const sortable_qstring & objIn) const {
      return x < objIn.x;
   }
};


QStringList SASFiles::reorder( const QStringList &files ) {
   reset_messages();

   bool reorder = true;
   QStringList filenames = files;
   
   QRegExp rx_cap( "(\\d+)_(\\d+)(\\D|$)" );
   QRegExp rx_clear_nonnumeric( "^(\\d?.?\\d+)\\D" );

   list < sortable_qstring > svals;

   QString head = qstring_common_head( filenames, true );
   QString tail = qstring_common_tail( filenames, true );

   bool do_prepend = false;
   QString prepend_tmp = "";
   {
      QRegExp rx_dp( "_q(\\d)_$" );
      if ( rx_dp.indexIn( head ) != -1 ) {
         prepend_tmp = rx_dp.cap( 1 ) + ".";
         do_prepend = true;
      }
   }

   set < QString > used;

   bool added_dp = false;

   for ( int i = 0; i < (int) filenames.size(); ++i ) {
      QString tmp = filenames[ i ].mid( head.length() );
      tmp = tmp.mid( 0, tmp.length() - tail.length() );

      if ( rx_clear_nonnumeric.indexIn( tmp ) != -1 ) {
         tmp = rx_clear_nonnumeric.cap( 1 );
      }

      added_dp = false;

      if ( rx_cap.indexIn( tmp ) != -1 ) {
         tmp = rx_cap.cap( 1 ) + "." + rx_cap.cap( 2 );
         added_dp = true;
      }

      if ( do_prepend ) {
         if ( added_dp ) {
            warnings <<
               "There is a problem decoding the frame numbers or q values from the file names\n"
               "Please email a list of the file names you are trying to load to emre@biochem.uthscsa.edu";
            return filenames;
         }
         tmp = prepend_tmp + tmp;
      }

      if ( used.count( tmp ) ) {
         reorder = false;
         break;
      }

      used.insert( tmp );

      sortable_qstring sval;
      sval.x     = tmp.toDouble();
      sval.name  = filenames[ i ];
      svals      .push_back( sval );
   }
   if ( reorder ) {
      svals.sort();

      filenames.clear( );
      for ( list < sortable_qstring >::iterator it = svals.begin();
            it != svals.end();
            ++it ) {
         filenames << it->name;
      }
   }
   return filenames;
}

QString SASFiles::qstring_common_head( const QStringList & qsl, bool strip_digits ) {
   if ( !qsl.size() ) {
      return "";
   }
   if ( qsl.size() == 1 ) {
      return qsl[ 0 ];
   }
   QString s = qsl[ 0 ];
   for ( int i = 1; i < (int)qsl.size(); i++ ) {
      s = qstring_common_head( s, qsl[ i ] );
   }

   if ( strip_digits ) {
      s.replace( QRegExp( "\\d+$" ), "" );
   }
   return s;
}

QString SASFiles::qstring_common_tail( const QStringList & qsl, bool strip_digits ) {
   if ( !qsl.size() ) {
      return "";
   }
   if ( qsl.size() == 1 ) {
      return qsl[ 0 ];
   }
   QString s = qsl[ 0 ];
   for ( int i = 1; i < (int)qsl.size(); i++ ) {
      s = qstring_common_tail( s, qsl[ i ] );
   }
   if ( strip_digits ) {
      s.replace( QRegExp( "^\\d+" ), "" );
   }
   return s;
}

QString SASFiles::qstring_common_head( const QString & s1, const QString & s2 ) {
   int min_len = (int)s1.length();
   if ( min_len > (int)s2.length() ) {
      min_len = (int)s2.length();
   }

   // could do this more efficiently via "divide & conquer"
   // i.e. split the distance in halfs and compare 
   
   int match_max = 0;
   for ( int i = 1; i <= min_len; i++ ) {
      match_max = i;
      if ( s1.left( i ) != s2.left( i ) ) {
         match_max = i - 1;
         break;
      }
   }
   return s1.left( match_max );
}

QString SASFiles::qstring_common_tail( const QString & s1, const QString & s2 ) {
   int min_len = (int)s1.length();
   if ( min_len > (int)s2.length() )
   {
      min_len = (int)s2.length();
   }

   // could do this more efficiently via "divide & conquer"
   // i.e. split the distance in halfs and compare 
   
   int match_max = 0;
   for ( int i = 1; i <= min_len; i++ ) {
      match_max = i;
      if ( s1.right( i ) != s2.right( i ) ) {
         match_max = i - 1;
         break;
      }
   }
   return s1.right( match_max );
}

// --------- run_pvp --------

bool SASFiles::run_pvp( map < QString, QString > & parameters ) {
   // build up all pairs

   parameters[ "msg" ] = "";
   parameters[ "hide_adjpvalues" ] = "true";

   if ( !parameters.count( "alpha" ) ) {
      parameters[ "alpha" ] = "0.05";
   }
   
   double maxq = parameters.count( "maxq" ) ? parameters[ "maxq" ].toDouble() : 0.05;

   vector < QString > selected_files;

   {
      vector < vector < double > > grids;

      for ( int i = 0; i < (int) files.size(); ++i ) {
         QString this_file = QFileInfo( files[ i ] ).completeBaseName();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) &&
              f_qs[ this_file ].size() &&
              f_Is[ this_file ].size() ) {
            selected_files    .push_back( this_file );
            grids.push_back( f_qs[ this_file ] );
         }
      }

      vector < double > v_union = Vector_Utils::vunion( grids );
      vector < double > v_int   = Vector_Utils::intersection( grids );

      bool any_differences = v_union != v_int;

      if ( any_differences ) {
         errors << "Pvp: curves must be on the same q grid";
         return false;
      }
   }

   if ( !selected_files.size() ) {
      errors << "Pvp has no files";
      return false;
   }

   double minq = parameters.count( "minq" ) ? parameters[ "minq" ].toDouble() : 0e0;

   int decimate = parameters.count( "decimate" ) ? parameters[ "decimate" ].toInt() : 0;

   // if files are time, then grab alternates, otherwise grab alternate q points
   if ( !f_is_time.count( selected_files[ 0 ] ) ) {
      errors << "Internal error: Pvp file selected not in files?";
      return false;
   }

   bool files_are_time = f_is_time[ selected_files[ 0 ] ];

   vector < QString > use_preq_selected_files;

   if ( files_are_time && decimate ) {
      for ( int i = 0; i < (int) selected_files.size(); i += decimate ) {
         use_preq_selected_files.push_back( selected_files[ i ] );
      }
   } else {
      use_preq_selected_files = selected_files;
   }

   vector < QString > use_selected_files;

   if ( files_are_time ) {
      QRegExp rx_cap( "_It_q(\\d+_\\d+)" );
      for ( int i = 0; i < (int) use_preq_selected_files.size(); ++i ) {
         if ( rx_cap.indexIn( use_preq_selected_files[ i ] ) == -1 ) {
            errors << 
               QString( "Pvp Analysis: Could not extract q value from file name %1" )
               .arg( use_preq_selected_files[ i ] );
            return false;
         }                  
         double qv = rx_cap.cap( 1 ).replace( "_", "." ).toDouble();
         if ( qv <= maxq && qv >= minq ) {
            use_selected_files.push_back( use_preq_selected_files[ i ] );
         }
      }
   } else {
      use_selected_files = use_preq_selected_files;
   }
            
   if ( use_selected_files.size() < 2 ) {
      errors << "Insufficient curves remaining for Pvp Analysis";
      return false;
   }            

   vector < double >            q = f_qs[ use_selected_files[ 0 ] ];
   vector < vector < double > > I( 2 );
   vector < vector < double > > rkl;

   int    N;
   int    S;
   int    C;
   double P;

   int    fcount = (int) use_selected_files.size();
   int    m      = 0;

   for ( int i = 0; i < fcount - 1; ++i ) {
      for ( int j = i + 1; j < fcount; ++j ) {
         ++m;
      }
   }

   vector < vector < double > > pvaluepairs( fcount );
   vector < vector < double > > adjpvaluepairs( fcount );

   int use_names_max_len = 10;

   for ( int i = 0; i < fcount; ++i ) {
      pvaluepairs   [ i ].resize( fcount );
      pvaluepairs   [ i ][ i ] = 1;
      adjpvaluepairs[ i ].resize( fcount );
      adjpvaluepairs[ i ][ i ] = 1;
      if ( use_names_max_len < (int) use_selected_files[ i ].length() ) {
         use_names_max_len = (int) use_selected_files[ i ].length();
      }
   }

   parameters[ "msg" ] = QString( "%1\t%2\t    N  Start point  C   P-value\n" )
      .arg( "File", -use_names_max_len )
      .arg( "File", -use_names_max_len )
      ;

   vector < double > undecimated_q = q;
   int q_points = (int) q.size();
   bool do_q_decimate = !files_are_time;
   int use_decimate = decimate ? decimate : 1;
   if ( do_q_decimate ) {
      vector < double > new_q;
      for ( int k = 0; k < q_points; k += use_decimate ) {
         if ( q[ k ] <= maxq &&
              q[ k ] >= minq ) {
            new_q.push_back( q[ k ] );
         }
      }
      q = new_q;
   }

   int compute_count = ( fcount - 1 ) * fcount / 2;
   int this_compute = 0;
   bool do_progress = parameters.count( "progress" );
   QTextStream out( stdout );

   for ( int i = 0; i < fcount - 1; ++i ) {
      I[ 0 ] = f_Is[ use_selected_files[ i ] ];

      if ( do_q_decimate ) {
         vector < double > new_I;
         for ( int k = 0; k < q_points; k += use_decimate ) {
            if ( undecimated_q[ k ] <= maxq &&
                 undecimated_q[ k ] >= minq ) {
               new_I.push_back( I[ 0 ][ k ] );
            }
         }
         I[ 0 ] = new_I;
      }

      for ( int j = i + 1; j < fcount; ++j ) {
         I[ 1 ] = f_Is[ use_selected_files[ j ] ];
         if ( do_q_decimate ) {
            vector < double > new_I;
            for ( int k = 0; k < q_points; k += use_decimate ) {
               if ( undecimated_q[ k ] <= maxq &&
                    undecimated_q[ k ] >= minq ) {
                  new_I.push_back( I[ 1 ][ k ] );
               }
            }
            I[ 1 ] = new_I;
         }

         if ( do_progress ) {
            if ( !( ++this_compute % 50 ) ) {
               out << QString( "%1%\r" ).arg( int( 1000 * this_compute / compute_count ) / 10 );
               out.flush();
            }
         }

         if ( !pvp.compute( q, I, rkl, N, S, C, P ) ) {
            errors << pvp.errors;
            return false;
         }
         double adjP = (double) m * P;
         if ( adjP > 1e0 ) {
            adjP = 1e0;
         }
         pvaluepairs[ i ][ j ] = P;
         pvaluepairs[ j ][ i ] = P;
         adjpvaluepairs[ i ][ j ] = adjP;
         adjpvaluepairs[ j ][ i ] = adjP;

         parameters[ "msg" ] += 
            QString( "%1\t%2\t%3\t%4\t%5\t%6"
                     // "\t%7"
                     "\n" )
            .arg( use_selected_files[ i ], -use_names_max_len )
            .arg( use_selected_files[ j ], -use_names_max_len )
            .arg( N, 6 )
            .arg( S, 6 )
            .arg( C, 6 )
            .arg( QString( "" ).sprintf( "%.4g", P ).leftJustified( 12 ) )
            // .arg( adjP ) 
            ;
      }
   }
   if ( do_progress ) {
      out << "\n";
      out.flush();
   }

   {
      parameters[ "title" ]     = 
         QString( "Maximum q limit is %1 [A^-1].")
         .arg( maxq );

      parameters[ "ppvm_title"     ] = "Pairwise P value map";
      parameters[ "ppvm_title_adj" ] = "Pairwise adjusted P value map";

      parameters[ "title_adj" ] = parameters[ "title" ];

      if ( minq > 0e0 ) {
         parameters[ "title"     ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( minq );
         parameters[ "title_adj" ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( minq );
      }

      if ( parameters.count( "decimate" ) ) {
         int decimate = parameters[ "decimate" ].toInt();
         if ( decimate > 1 ) {
            QString nth_tag;
            switch ( decimate ) {
            case 2 : nth_tag = "nd"; break;
            case 3 : nth_tag = "rd"; break;
            default : nth_tag = "th"; break;
            }
               
            parameters[ "title" ] += QString( " Only every %1%2 q value sampled." )
               .arg( decimate ).arg( nth_tag );
            parameters[ "title_adj" ] += QString( " Only every %1%2 q value sampled." )
               .arg( decimate ).arg( nth_tag );
         }
      }

      parameters[ "adjusted" ] = "true";
      parameters[ "linewisesummary" ] = "true";
      parameters[ "clusteranalysis"   ] = "true";

      parameters[ "global_width" ] = "1000";
      parameters[ "global_height" ] = "700";
      parameters[ "image_height" ] = "300";

      {
         QStringList qsl = Vector_Utils::vector_qstring_to_qstringlist( use_selected_files );
         QString head = qstring_common_head( qsl, true );
         QString tail = qstring_common_tail( qsl, true );
         parameters[ "name" ] = QString( "%1_%2%3_%4%5_cqmx%6" )
            .arg( head )
            .arg( tail )
            .arg( parameters.count( "decimate" ) ? QString( "_s%1" ).arg( parameters[ "decimate" ] ) : QString( "" ) )
            .arg( files_are_time ? "q" : "f" )
            .arg( pvaluepairs.size() )
            .arg( maxq )
            .replace( ".", "_" )
            ;
      }

      PVPAnalysis pvpa;
      if ( !pvpa.compute( parameters, pvaluepairs, adjpvaluepairs, use_selected_files ) ) {
         errors << pvpa.errors;
         return false;
      } else {
         return true;
      }
   }
   return false;
}
