// This utility analysis an Apache Web log file.
// Apache Web log file has the following format:
// %h %l %u %t "%r" %>s %b "%{Referer}i" "%{User-agent}i"
// where:
// %h   =  IP address of the client (remote host) which made the request
// %l   =  RFC 1413 identity of the client
// %u   =  userid of the person requesting the document
// %t   =  Time that the server finished processing the request
// %r   =  Request line from the client in double quotes
// %>s  =  Status code that the server sends back to the client
// %b   =  Size of the object returned to the client
//

// API
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
// BOOST
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
// STD
#include <fstream>
#include <numeric>
#include <iomanip>
// MiscCommon
#include "CustomIterator.h"
#include "def.h"
#include "MiscUtils.h"
#include "SysHelper.h"
// pod-web-stat
#include "MonthlyDownloads.h"
#include "DownloadsByCountry.h"
#include "DownloadByVersion.h"

using namespace std;
namespace bpo = boost::program_options;
using namespace MiscCommon;
//=============================================================================
void printVersion()
{
    //cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
    //<< "Report bugs/comments to A.Manafov@gsi.de" << endl;
}
//=============================================================================
// Command line parser
bool parseCmdLine( int _Argc, char *_Argv[], bpo::variables_map *_vm )
{
    // Generic options
    bpo::options_description visible( "Options" );
    visible.add_options()
    ( "help,h", "Produce help message" )
    ( "version,v", "Version information" )
    ( "logfile,l", bpo::value<string>(), "WEB log file to analise" )
    ( "country,c", bpo::value<string>(), "IP to Country log file" )
    ( "docbook,d", "Whether to generate a docbook output" )
    ( "figures_path", bpo::value<string>(), "Full path to the directory containing stats figures" )
    ;

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );
    bpo::notify( vm );

    if( vm.count( "help" ) || vm.empty() )
    {
        cout << visible << endl;
        return false;
    }
    if( vm.count( "version" ) )
    {
        printVersion();
        return false;
    }

    if( !vm.count( "logfile" ) )
    {
        cout << visible << endl;
        throw runtime_error( "You need to specify a logfile file at least." );
    }

    _vm->swap( vm );
    return true;
}
//=============================================================================
int add_to_totals( int _total, const InfoByFile_t::value_type& _data )
{
    return _total + _data.second.size();
}
//=============================================================================
void totals_by_file( const InfoByFile_t::value_type& _data )
{
    // we send it to cerr, so that the caller can collect this data even when generating a docbook
    cerr
            << setw( 40 ) << left
            << _data.first << ": " << _data.second.size() << '\n';
}
//=============================================================================
int main( int argc, char * argv[] )
{
    try
    {
        bpo::variables_map vm;
        if( !parseCmdLine( argc, argv, &vm ) )
            return 0;
        ifstream f( vm["logfile"].as<string>().c_str() );
        if( !f.is_open() )
        {
            string msg( "can't open logfile file \"" );
            msg += vm["logfile"].as<string>();
            msg += "\"";
            throw runtime_error( msg );
        }
        bool need_dock_book = vm.count( "docbook" );

        // read the given web log to the array of strings
        StringVector_t vec;
        std::copy( custom_istream_iterator<std::string>( f ),
                   custom_istream_iterator<std::string>(),
                   std::back_inserter( vec ) );

        InfoByFile_t info_by_file;

        StringVector_t::const_iterator iter = vec.begin();
        StringVector_t::const_iterator iter_end = vec.end();
        for( ; iter != iter_end; ++iter )
        {
            SInfo info;

            // Parsing IP
            size_t pos = iter->find( ' ' );
            // Skip errors
            if( string::npos == pos )
            {
                cerr << "skipping bad entry... " << *iter << endl;
                continue;
            }
            info.m_ip = iter->substr( 0, pos );
            // Some strange IP. Downloads every week all different versions.
            // Probably some kind of Robot
            if( info.m_ip == "213.186.122.3" )
              continue;

            // parse date/time
            pos = iter->find( '[' );
            // Skip errors
            if( string::npos == pos )
            {
                cerr << "skipping bad entry... " << *iter << endl;
                continue;
            }
            size_t pos1 = iter->find( ']', pos );
            // Skip errors
            if( string::npos == pos1 )
            {
                cerr << "skipping bad entry... " << *iter << endl;
                continue;
            }
            const string tmp( iter->substr( pos + 1, pos1 - pos - 1 ) );
            if( strptime( tmp.c_str(), "%d/%b/%Y:%T ", &info.m_tmpStamp ) == 0 )
            {
                cerr << "skipping bad entry... " << *iter << endl;
                continue;
            }

            // parsing a downloaded file name
            pos = iter->find( "PoD", pos1 );
            pos1 = iter->find( ".tar.gz", pos );
            if( string::npos == pos || string::npos == pos1 )
            {
                cerr << "skipping bad entry... " << *iter << endl;
                continue;
            }
            info.m_file = iter->substr( pos, pos1 - pos );

            // parse error code
            pos = iter->find( '\"', pos1 );
            if( string::npos == pos )
            {
                cerr << "skipping bad entry... " << *iter << endl;
                continue;
            }
            // skip a white space
            pos += 2;
            // read error code itself
            stringstream ss;
            while( isdigit(( *iter )[pos] ) )
            {
                ss << ( char )(( *iter )[pos++] );
            }
            int error_code( 0 );
            ss >> error_code;
            if( error_code < 200 || error_code > 299 )
            {
                cerr << "skipping this error code: " << *iter << endl;
                continue;
            }

            // Info by file
            InfoByFile_t::iterator found = info_by_file.find( info.m_file );
            if( info_by_file.end() == found )
            {
                InfoContainer_t tmp_vec;
                tmp_vec.push_back( info );
                info_by_file.insert( InfoByFile_t::value_type( info.m_file, tmp_vec ) );
            }
            else
                found->second.push_back( info );
        }

        if( need_dock_book )
        {
            cout
                    << "<!DOCTYPE webpage\n"
                    << "PUBLIC \"-//Norman Walsh//DTD Website V2.6.0//EN\"\n"
                    << "\"http://docbook.sourceforge.net/release/website/2.6.0/schema/dtd/website-full.dtd\"[\n"
                    << "<!ELEMENT my_chart EMPTY>\n"
                    << "<!ATTLIST my_chart name CDATA #IMPLIED>\n"
                    << "<!ENTITY % local.para.char.mix\n"
                    << "\"|my_chart\">\n"
                    << "]>\n"
                    << "\n"
                    << "<webpage id=\"download_stat\">\n"
                    << "<config param=\"desc\" value=\"Navigation Layout\"/>\n"
                    << "\n"
                    << "<head>\n"
                    << "<title>Download Statistics</title>\n"
                    << "<summary>Download Statistics</summary>\n"
                    << "<!--Load the AJAX API-->\n"
                    << "<script type=\"text/javascript\" src=\"https://www.google.com/jsapi\"></script>\n"
                    << "<script type=\"text/javascript\">\n"
                    << "google.load(\'visualization\', \'1\', {packages: [\'corechart\']});\n"
                    << "</script>\n";
        }

        // insert script for Downloads by Country
        CDownloadsByCountry countries( vm["country"].as<string>() );
        countries = for_each( info_by_file.begin(), info_by_file.end(),
                              countries );
        if( need_dock_book )
        {
            cout << countries.print_chart();
        }

        // ionsert monthly downloads stats
        // Downloads by date
        CMonthlyDownloads monthly;
        monthly = for_each( info_by_file.begin(), info_by_file.end(),
                            monthly );
        if( need_dock_book )
        {
            cout << monthly.print_chart();
        }

        // Totals by versions
        CDownloadsByVersion byversion;
        byversion = for_each( info_by_file.begin(), info_by_file.end(),
                              byversion );
        if( need_dock_book )
        {
            cout << byversion.print_chart();
        }

        // Close HEAD
        if( need_dock_book )
            cout << "</head>\n";

        // Total
        const int total = accumulate( info_by_file.begin(), info_by_file.end(),
                                      0, add_to_totals );
        if( !need_dock_book )
        {
            cout << "*** Download statistics *** \n";
            cout << "Total: " << total << '\n';
        }
        else
        {
            time_t rawtime;
            time( &rawtime );
            string dt( ctime( &rawtime ) );
            replace<string>( &dt, "\n", "" );
            cout
                    << "<section><title>Total: "
                    << total
                    << " Downloads (last auto. update: "
                    << dt
                    << ")</title>\n";
        }

        // Downloads per year
        if( need_dock_book )
        {
            cout
                    << "<section><title></title>\n"
                    << "<para>\n"
                    << "<my_chart name=\"downloads_per_year\" />"
                    << "</para>\n"
                    << "</section>\n";
        }

        // Monthly download stats
        if( need_dock_book )
        {
            cout
                    << "<section><title></title>\n"
                    << "<para>\n"
                    << "<my_chart name=\"monthly_downloads\" />"
                    << "</para>\n"
                    << "</section>\n";
        }

        // Downloads by countries
        if( need_dock_book )
        {
            cout
                    << "<section><title></title>\n"
                    << "<para>\n"
                    << "<my_chart name=\"chart_countries\" />"
                    << "<my_chart name=\"bar_chart_countries\" />"
                    << "</para>\n"
                    << "</section>\n";
        }

        // Totals by versions
        if( need_dock_book )
        {
            cout
                    << "<section><title></title>\n"
                    << "<para>\n"
                    << "<my_chart name=\"totals_by_versions\" />"
                    << "</para>\n"
                    << "</section>\n";
        }


//        SByCountry countries( vm["country"].as<string>() );
//        countries = for_each( info_by_file.begin(), info_by_file.end(),
//                              countries );
//        if( !need_dock_book )
//        {
//            cout << "\nDownloads by Country:\n";
//        }
//        else
//        {
//            cout
//                    << "<section><title>Downloads by Country</title>\n"
//                    << "<para>\n"
//                    << "<screen>\n";
//        }
//        countries.print();
//        if( need_dock_book )
//        {
//            cout
//                    << "</screen>\n"
//                    << "</para>\n"
//                    << "</section>\n";
//        }

        // Downloads by date
//        SByDate bydate;
//        bydate = for_each( info_by_file.begin(), info_by_file.end(),
//                           bydate );
//        if( !need_dock_book )
//        {
//            cout << "\nStats by Date:\n";
//        }
//        else
//        {
//            cout
//                    << "<section><title>Stats by Date</title>\n"
//                    << "<para>\n"
//                    << "<screen>\n";
//        }
//        bydate.print();
//        if( need_dock_book )
//        {
//            cout
//                    << "</screen>\n"
//                    << "</para>\n"
//                    << "</section>\n";
//        }

//        // Downloads by version number
//        if( !need_dock_book )
//        {
//            cout << "\nTotals by versions (unique IPs):\n";
//        }
//        else
//        {
//            cout
//                    << "<section><title>Totals by versions (unique IPs)</title>\n"
//                    << "<para>\n"
//                    << "<screen>\n";
//
//        }
//        SByVersionNumber byversion;
//        byversion = for_each( info_by_file.begin(), info_by_file.end(),
//                              byversion );
//        byversion.print();
//        if( need_dock_book )
//        {
//            cout
//                    << "</screen>\n"
//                    << "</para>\n"
//                    << "</section>\n";
//        }
//

        // Downloads by file
        // we send it to cerr, so that the caller can collect this data even when generating a docbook
        cerr << "\nDetailed by versions:\n";
        for_each( info_by_file.begin(), info_by_file.end(),
                  totals_by_file );
        if( need_dock_book )
        {
            cout
                    << "</section>\n"
                    << "\n</webpage>\n";
        }
        cout.flush();

    }
    catch( exception& e )
    {
        cerr <<  e.what() + string( "\n" )  << endl;
        return 1;
    }
    return 0;
}
