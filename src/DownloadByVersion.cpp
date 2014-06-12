//
//  DownloadByVersion.cpp
//  pod-web-stat
//
//  Created by Anar Manafov on 12/24/10.
//  Copyright 2010 GSI. All rights reserved.
//
//=============================================================================
// STD
#include <iostream>
#include <iomanip>
#include <sstream>
// pod-web-stat
#include "DownloadByVersion.h"
//=============================================================================
using namespace std;
//=============================================================================
void CDownloadsByVersion::operator()( const InfoByFile_t::value_type& _data )
{

    InfoByFile_t::value_type::second_type::const_iterator iter =
        _data.second.begin();
    InfoByFile_t::value_type::second_type::const_iterator iter_end =
        _data.second.end();
    for( ; iter != iter_end; ++iter )
    {
        string tmp( iter->m_file );
        string::size_type pos = tmp.find( "PoD-" );
        if( string::npos == pos )
            continue;
        pos += 4;

        string::size_type pos_dot1 = tmp.find( '.', pos );
        if( string::npos == pos_dot1 )
            continue;

        string::size_type pos_dot2 = tmp.find( '.', pos_dot1 + 1 );
        if( string::npos == pos_dot2 )
        {
            pos_dot2 = tmp.find( '-', pos_dot1 + 1 );
            if( string::npos == pos_dot2 )
                continue;
        }
        tmp = tmp.substr( pos, pos_dot2 - pos );

        container_t::iterator found = m_container.find( tmp );
        if( m_container.end() == found )
        {
            SValue value;
            value.m_count = 1;
            value.m_uniqueIPs.insert( iter->m_ip );
            m_container.insert( container_t::value_type( tmp, value ) );
        }
        else
        {
            ++( found->second.m_count );
            found->second.m_uniqueIPs.insert( iter->m_ip );
        }
    }
}
//=============================================================================
void CDownloadsByVersion::print() const
{
    container_t::const_iterator iter = m_container.begin();
    container_t::const_iterator iter_end = m_container.end();
    for( ; iter != iter_end; ++iter )
    {
        cout
                << 'v' << setw( 6 ) << left << iter->first << ": "
                << setw( 3 ) << left << iter->second.m_count << "(" << iter->second.m_uniqueIPs.size() << ")" << '\n';
    }
}
//=============================================================================
string CDownloadsByVersion::print_chart()
{
    stringstream chart;
    chart
            << "<script type=\"text/javascript\">\n"
            << "function drawVisualization() {\n"
            << "    // Create and populate the data table.\n"
            << "    var data = new google.visualization.DataTable();\n"
            << "    data.addColumn('string', 'Version');\n"
            << "    data.addColumn('number', 'unique IP addresses');\n"
            << "    data.addColumn('number', 'duplicate IP addresses');\n";

    chart << "data.addRows(" << m_container.size() << ");\n";

    container_t::const_iterator iter = m_container.begin();
    container_t::const_iterator iter_end = m_container.end();
    for( size_t index = 0; iter != iter_end; ++iter, ++index )
    {
        chart
                << "data.setValue(" << index
                << ", " << 0
                << ", 'v" << iter->first << ".x');\n"
                << "data.setValue(" << index
                << ", " << 2
                << ", " << iter->second.m_count - iter->second.m_uniqueIPs.size() << ");\n"
                << "data.setValue(" << index
                << ", " << 1
                << ", " << iter->second.m_uniqueIPs.size() << ");\n";
    }

    chart
            << "// Create and draw the visualization.\n"
            << "new google.visualization.ColumnChart(document.getElementById(\'totals_by_versions\')).\n"
            << "draw(data, {reverseCategories: 'false',\n"
            << "width: 900, height: 400,\n"
            << "isStacked: true,\n"
            << "title: \"Downloads by Versions (accumulated by minor numbers)\" }\n"
            << "     );\n"
            << "}\n"
            << "google.setOnLoadCallback(drawVisualization);\n"
            << "</script>\n";

    return chart.str();
}
//=============================================================================
