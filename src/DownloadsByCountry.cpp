//
//  DownloadsByCountry.cpp
//  pod-web-stat
//
//  Created by Anar Manafov on 12/24/10.
//  Copyright 2010 GSI. All rights reserved.
//
//=============================================================================
// STD
#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <cstring>
// MiscCommon
#include "def.h"
#include "MiscUtils.h"
// pod-web-stat
#include "DownloadsByCountry.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
CDownloadsByCountry::CDownloadsByCountry( const string &_ipToCountryFileName )
{
    ifstream f_country( _ipToCountryFileName.c_str() );
    if( f_country.is_open() )
    {
        StringVector_t vals;
        copy( istream_iterator<string>( f_country ),
              istream_iterator<string>(),
              back_inserter( vals ) );

        StringVector_t::const_iterator iter = vals.begin();
        StringVector_t::const_iterator iter_end = vals.end();
        for( ; iter != iter_end; iter += 2 )
        {
            string tmp( *( iter + 1 ) ); // country code
            to_upper( tmp );
            m_IPtoCountry.insert( strToStrMap_t::value_type( *iter, tmp ) );
        }
    }
}
//=============================================================================
void CDownloadsByCountry::operator()( const InfoByFile_t::value_type& _data )
{
    InfoByFile_t::value_type::second_type::const_iterator iter =
        _data.second.begin();
    InfoByFile_t::value_type::second_type::const_iterator iter_end =
        _data.second.end();
    for( ; iter != iter_end; ++iter )
    {
        strToStrMap_t::iterator found = m_IPtoCountry.find( iter->m_ip );
        string country_code;
        // If there is no country for this ip, we use "??" as a code.
        // This could happen if it is a private net address 10.*.*.* or something
        country_code = ( m_IPtoCountry.end() == found ) ? "??" : found->second;

        container_t::iterator item = m_container.find( country_code );
        if( m_container.end() == item )
        {
            m_container.insert( container_t::value_type( country_code, 1 ) );
        }
        else
            item->second = ( item->second + 1 );
    }
}
//=============================================================================
void CDownloadsByCountry::print()
{
    containerSorted_t containerSorted;
    container_t::const_iterator iter = m_container.begin();
    container_t::const_iterator iter_end = m_container.end();
    for( ; iter != iter_end; ++iter )
    {
        containerSorted.insert( containerSorted_t::value_type( iter->second, iter->first ) );
    }
    containerSorted_t::const_iterator i = containerSorted.begin();
    containerSorted_t::const_iterator i_end = containerSorted.end();
    for( ; i != i_end; ++i )
    {
        string country( i->second );
        for( int j = 0; domain[j].code; j++ )
            if( !strncmp( country.c_str(), domain[j].code, 2 ) )
                country = domain[j].country;
        cout << setw( 32 ) << left << country << ": " << i->first << '\n';
    }
}
//=============================================================================
string CDownloadsByCountry::print_chart()
{
    stringstream chart;
    chart
            << "<script type=\"text/javascript\">\n"
            << "function drawVisualization() {\n"
            << "    // Create and populate the data table.\n"
            << "    var data = new google.visualization.DataTable();\n"
            << "    data.addColumn(\'string\', \'Country\');\n"
            << "    data.addColumn(\'number\', \'Download\');\n";

    containerSorted_t containerSorted;
    container_t::const_iterator iter = m_container.begin();
    container_t::const_iterator iter_end = m_container.end();
    for( ; iter != iter_end; ++iter )
    {
        containerSorted.insert( containerSorted_t::value_type( iter->second, iter->first ) );
    }

    chart << "data.addRows(" << containerSorted.size() << ");\n";

    containerSorted_t::const_iterator i = containerSorted.begin();
    containerSorted_t::const_iterator i_end = containerSorted.end();
    stringstream vals;
    stringstream countries;
    for( size_t index = 0 ; i != i_end; ++i, ++index )
    {
        string country( i->second );
        for( int j = 0; domain[j].code; j++ )
            if( !strncmp( country.c_str(), domain[j].code, 2 ) )
                country = domain[j].country;
        chart
                << "data.setValue(" << index << ", 0, \'" << country << "\');\n"
                << "data.setValue(" << index << ", 1, " << i->first << ");\n";

        vals << ", " << i->first;
        if( containerSorted.begin() != i )
            countries << ", ";
        countries << "'" << country << "'";
    }

    chart
            << "// Create and draw the visualization.\n"
            << "new google.visualization.PieChart(document.getElementById(\'chart_countries\')).\n"
            << "draw(data, {width: 700, height: 500, title:\"Downloads by Country\"});\n"
            << "// Create and draw the visualization.\n"
            << "new google.visualization.BarChart(document.getElementById('bar_chart_countries')).\n"
            << "draw(data,\n"
            << "     {title:\"Downloads by Country\",\n"
            << "     legend:\"none\",\n"
            << "     width:900, height:700,\n"
            << "         hAxis: {title: \"Downloads\"}}\n"
            << "     );\n"
            << "}\n"
            << "google.setOnLoadCallback(drawVisualization);\n"
            << "</script>\n";

    return chart.str();
}
