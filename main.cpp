#include <QtCore>
#include <QDebug>

#include <sasfiles.h>

int main( int argc, char *argv[] ) {

   QCoreApplication app( argc, argv );
   QCoreApplication::setApplicationName("PVP");
   QCoreApplication::setApplicationVersion("0.1");

   QCommandLineParser parser;
   parser.setApplicationDescription(
                                    "P Value pair utility\n"
                                    "\n"
                                    "Usage: pvp {options} filenames...\n"
                                    "\n"
                                    "File names will be processed and summary information will be written to standard output\n"
                                    "Additonal behaviour is controlled by specifying 'Options'"
                                    );
   
   parser.addOptions({
         {
            {"d", "decimate"},
               QCoreApplication::translate("main", "decimate value - if > 1, every d-th q point will be dropped (default 1)."),
                  QCoreApplication::translate("main", "decimate")
                  }
         ,{
            {"f", "force"},
               QCoreApplication::translate("main", "Overwrite existing files.")
                  }
      });
   

   parser.addHelpOption();

   parser.addOptions({
         {
            {"H", "holm_bonferroni"},
               QCoreApplication::translate("main", "Turn on the Holm-Bonferroni adjustment.")
                  }
         ,{
            {"j", "json"},
               QCoreApplication::translate("main", "Provide inputs in specified JSON object and provide JSON output."),
                  QCoreApplication::translate("main", "json")
                  }
         ,{
            {"m", "minq"},
               QCoreApplication::translate("main", "Set the minimum q value (default 0)."),
                  QCoreApplication::translate("main", "maxq")
                  }
         ,{
            {"M", "maxq"},
               QCoreApplication::translate("main", "Set the maximum q value (default 0.05)."),
                  QCoreApplication::translate("main", "minq")
                  }
         ,{
            {"n", "name"},
               QCoreApplication::translate("main", "Name prefix for the output files (default 'pvpout')."),
                  QCoreApplication::translate("main", "name")
                  }
         ,{
            {"p", "progress"},
               QCoreApplication::translate("main", "Display progress updates")
                  }
         ,{
            {"s", "size"},
               QCoreApplication::translate("main", "Set the image size (always square - default 512 [pixels])."),
                  QCoreApplication::translate("main", "size")
                  }
      });

   parser.addVersionOption();

   if ( argc == 1 ) {
      parser.showHelp(-1);
   };

   parser.process(app);

   QStringList args = parser.positionalArguments();

   QTextStream out( stdout );
   map < QString, QString > parameters =
      {
         { "size" , "512" }
         ,{ "outname" , "pvpout" }
      }
   ;


   bool is_json = false;
   QVariantMap results;

   if ( !parser.value( "j" ).isEmpty() ) {
      // JSON mode, all options are in json object
      is_json = true;

      if ( args.size() ) {
         out << "{\"error\":\"JSON mode only allows only the json option and other argument(s) specified.\"}\n";
         out.flush();
         exit(-1);
      }

      parameters[ "json" ] = parser.value( "j" );
      QJsonParseError jsonerror;
      out << parameters[ "json" ] << "\n";
      QJsonDocument jsondoc = QJsonDocument::fromJson( parameters[ "json" ].toUtf8(), &jsonerror );
      if ( jsondoc.isNull() ) {
         out << "{\"error\":\"Input JSON parse error - " << jsonerror.errorString() << "\"}\n";
         out.flush();
         exit(-1);
      }

      if ( jsondoc.isEmpty() ) {
         out << "{\"error\":\"Input JSON is empty.\"}\n";
         out.flush();
         exit(-1);
      }

      // now process JSON arguments into parameters
      if ( jsondoc.isArray() ) {
         out << "{\"error\":\"Input JSON is an array but should be an object\"}\n";
         out.flush();
         exit(-1);
      }

      if ( !jsondoc.isObject() ) {
         out << "{\"error\":\"Input JSON is not an object.\"}\n";
         out.flush();
         exit(-1);
      }
         
      QJsonObject jsonobj = jsondoc.object();
      if ( jsonobj.isEmpty() ) {
         out << "{\"error\":\"Input JSON object is empty.\"}\n";
         out.flush();
         exit(-1);
      }

      QVariantMap jsonmap = jsonobj.toVariantMap();

      map<QString, QString> short_to_full =
         {
            {"d","decimate"}
            ,{"f","force"}
            ,{"H","hb"}
            ,{"m","minq"}
            ,{"M","maxq"}
            ,{"n","outname"}
            ,{"p","progress"}
            ,{"s","size"}
         };

      set<QString> is_bool =
         {
            "force"
            ,"hb"
            ,"progress"
         };
      
      for( QVariantMap::const_iterator it = jsonmap.constBegin();
           it != jsonmap.constEnd();
           ++it ) {
         QString use_key = short_to_full.count( it.key() ) ? short_to_full[ it.key() ] : it.key();
         
         if ( is_bool.count( use_key ) ) {
            // bools are set if present
            parameters[ use_key ] = "true";
         } else {
            if ( it.value().canConvert<QString>() ) {
               parameters[ use_key ] = it.value().toString();
            } else {
               if ( it.value().canConvert<QStringList>() ) {
                  if ( use_key != "files" ) {
                     out << "{\"error\":\"Input JSON object key " << it.key() << " is an array and this is not currently supported except for key 'files'.\"}\n";
                     out.flush();
                     exit(-1);
                  }
                  args = it.value().toStringList();
               }
            }
         }
      }
   } else {
      // require arguments for files
      if ( !args.size() ) {
         parser.showHelp(-1);
      };
   
      if ( !parser.value( "d" ).isEmpty() ) {
         parameters[ "decimate" ] = parser.value( "d" );
         out << QString( "decimate is set to %1\n" ).arg( parameters[ "decimate" ] );
      }

      if ( parser.isSet( "f" ) ) {
         parameters[ "force" ] = "true";
         out << "Force overwriting of files\n";
      }

      if ( parser.isSet( "H" ) ) {
         parameters[ "hb" ] = "true";
         out << "Holm Bonferroni adjustment is on\n";
      }

      if ( !parser.value( "m" ).isEmpty() ) {
         parameters[ "minq" ] = parser.value( "m" );
         out << QString( "minq is set to %1\n" ).arg( parameters[ "minq" ] );
      }

      if ( !parser.value( "M" ).isEmpty() ) {
         parameters[ "maxq" ] = parser.value( "M" );
         out << QString( "maxq is set to %1\n" ).arg( parameters[ "maxq" ] );
      }

      if ( !parser.value( "n" ).isEmpty() ) {
         parameters[ "outname" ] = parser.value( "m" );
         out << QString( "name is set to %1\n" ).arg( parameters[ "outname" ] );
      }

      if ( parser.isSet( "p" ) ) {
         parameters[ "progress" ] = "true";
         out << "Progress updates are on\n";
      }

      if ( !parser.value( "s" ).isEmpty() ) {
         parameters[ "size" ] = parser.value( "s" );
         out << QString( "size is set to %1\n" ).arg( parameters[ "size" ] );
      }
   }

   out.flush();

   SASFiles sasfiles;
   if ( !sasfiles.load( args, parameters.count( "json" ) ) ) {
      if ( is_json ) {
         results[ "errors" ] = sasfiles.errors.join( "\n" ) + "\n";
         QJsonDocument results_doc( QJsonObject::fromVariantMap( results ) );
         out << results_doc.toJson( QJsonDocument::Compact );
      } else {
         out << sasfiles.errors.join( "\n" ) << "\n";
      }
      out.flush();
      exit(-2);
   }

   if ( sasfiles.errors.size() ) {
      if ( is_json ) {
         results[ "errors" ] = results[ "errors" ].toString() + sasfiles.errors.join( "\n" ) + "\n";
         QJsonDocument results_doc( QJsonObject::fromVariantMap( results ) );
      } else {
         out << "Errors\n" << sasfiles.errors.join( "\n" ) << "\n\n";
      }
   }

   if ( sasfiles.warnings.size() ) {
      if ( is_json ) {
         results[ "warnings" ] = sasfiles.warnings.join( "\n" ) + "\n";
      } else {
         out << "Warnings:\n" << sasfiles.warnings.join( "\n" ) << "\n\n";
      }
   }

   if ( sasfiles.notices.size() ) {
      if ( is_json ) {
         results[ "notices" ] = sasfiles.notices.join( "\n" ) + "\n";
      } else {
         out << "Notices:\n" << sasfiles.notices.join( "\n" ) << "\n\n";
      }
   }

   out.flush();

   if ( !sasfiles.run_pvp( parameters ) ) {
      if ( is_json ) {
         results[ "errors" ] = results[ "errors" ].toString() + sasfiles.errors.join( "\n" ) + "\n";
         QJsonDocument results_doc( QJsonObject::fromVariantMap( results ) );
         out << results_doc.toJson( QJsonDocument::Compact );
      } else {
         out << sasfiles.errors.join( "\n" ) << "\n";
      }
      out.flush();
      exit(-2);
   }

   // if ( parameters.count( "size" ) &&
   //      parameters.count( "outname" ) ) {
   //    results[ "imagemap" ] =
   //       parameters[ "outname" ]
   //       + ( parameters.count( "hb" ) ? "_hb" : "" )
   //       + ".png";
   //    qDebug() << "imagemap should be defined";
   // } else {
   //    qDebug() << "no size nor outname\n";
   // }

   if ( parameters.count( "outfilename" ) ) {
      results[ "imagemap" ] = parameters[ "outfilename" ];
   }

   if ( parameters.count( "report" ) ) {
      if ( is_json ) {
         results[ "results" ] = parameters[ "report" ];
         QJsonDocument results_doc( QJsonObject::fromVariantMap( results ) );
         out << results_doc.toJson( QJsonDocument::Compact );
      } else {
         out << parameters[ "report" ];
      }
   } else {
      if ( is_json ) {
         results[ "errors" ] = results[ "errors" ].toString() + "sasfiles.run_pvp returned true, but no report found\n";
         QJsonDocument results_doc( QJsonObject::fromVariantMap( results ) );
         out << results_doc.toJson( QJsonDocument::Compact );
      } else {
         out << "sasfiles.run_pvp returned true, but no report found\n";
      }         
   }      

   out.flush();
}
