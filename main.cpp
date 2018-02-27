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
         // A boolean option 
         ,{
            {"p", "progress"},
               QCoreApplication::translate("main", "Display progress dots")
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

   const QStringList args = parser.positionalArguments();

   if ( !args.size() ) {
      parser.showHelp(-1);
   };

   QTextStream out( stdout );

   map < QString, QString > parameters;
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
   } else {
      parameters[ "outname" ] = "pvpout";
   }

   if ( !parser.value( "d" ).isEmpty() ) {
      parameters[ "decimate" ] = parser.value( "d" );
      out << QString( "decimate is set to %1\n" ).arg( parameters[ "decimate" ] );
   }

   if ( !parser.value( "s" ).isEmpty() ) {
      parameters[ "size" ] = parser.value( "s" );
      out << QString( "size is set to %1\n" ).arg( parameters[ "size" ] );
   } else {
      parameters[ "size" ] = "512";
   }

   out.flush();

   SASFiles sasfiles;
   if ( !sasfiles.load( args ) ) {
      qDebug() << "sasfiles::load() failed";
      out << sasfiles.errors.join( "\n" ) << "\n";
      out.flush();
      exit(-2);
   }

   if ( sasfiles.errors.size() ) {
      out << "Errors\n" << sasfiles.errors.join( "\n" ) << "\n\n";
   }

   if ( sasfiles.warnings.size() ) {
      out << "Warnings:\n" << sasfiles.errors.join( "\n" ) << "\n\n";
   }

   if ( sasfiles.notices.size() ) {
      out << "Notices:\n" << sasfiles.errors.join( "\n" ) << "\n\n";
   }

   out.flush();

   if ( !sasfiles.run_pvp( parameters ) ) {
      qDebug() << "sasfiles::run_pvp() failed";
      out << sasfiles.errors.join( "\n" ) << "\n";
      out.flush();
      exit(-2);
   }

   if ( parameters.count( "report" ) ) {
      out << parameters[ "report" ];
   } else {
      out << "sasfiles.run_pvp returned true, but no report found\n";
   }      
   out.flush();
}
