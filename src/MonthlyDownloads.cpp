//
//  MonthlyDownloads.cpp
//  pod-web-stat
//
//  Created by Anar Manafov on 12/24/10.
//  Copyright 2010 GSI. All rights reserved.
//
//=============================================================================
// STL
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
// pod-web-stat
#include "MonthlyDownloads.h"
//=============================================================================
using namespace std;
//=============================================================================
const int g_start_year = 2010;
const int g_start_month = 6;
const char* months[] = { "Jan.",
                         "Feb.",
                         "Mar.",
                         "Apr.",
                         "Maj.",
                         "Jun.",
                         "Jul.",
                         "Avg.",
                         "Sep.",
                         "Okt.",
                         "Nov.",
                         "Dec."
                       };
//=============================================================================
bool operator!= ( const tm &_val, const tm &_val2 )
{
    return ( _val.tm_mon != _val2.tm_mon || _val.tm_year != _val2.tm_year );
}
//=============================================================================
CMonthlyDownloads::CMonthlyDownloads()
{
}
//=============================================================================
void CMonthlyDownloads::operator()( const InfoByFile_t::value_type &_data )
{
    InfoByFile_t::value_type::second_type::const_iterator iter =
        _data.second.begin();
    InfoByFile_t::value_type::second_type::const_iterator iter_end =
        _data.second.end();
    for( ; iter != iter_end; ++iter )
    {
        // collect years
        m_years.insert( years_t::value_type( iter->m_tmpStamp.tm_year, 0 ) );

        // collect downloads and unique IPs
        yearsEx_t::iterator y = m_yearsDownloads.find( iter->m_tmpStamp.tm_year );
        if( m_yearsDownloads.end() == y )
        {
            SYearsInfo info;
            info.m_downloads = 1;
            info.m_uniqueIPs.insert( iter->m_ip );
            m_yearsDownloads.insert( yearsEx_t::value_type( iter->m_tmpStamp.tm_year, info ) );
        }
        else
        {
            ++( y->second.m_downloads );
            y->second.m_uniqueIPs.insert( iter->m_ip );
        }

        tm tm_tmp;
        tm_tmp.tm_year = iter->m_tmpStamp.tm_year;
        tm_tmp.tm_mon = iter->m_tmpStamp.tm_mon;
        container_t::iterator found = m_container.find( tm_tmp );
        if( m_container.end() == found )
            m_container.insert( container_t::value_type( tm_tmp, 1 ) );
        else
            ++( found->second );
    }
}
//=============================================================================
void CMonthlyDownloads::print() const
{
    container_t::const_iterator iter = m_container.begin();
    container_t::const_iterator iter_end = m_container.end();
    for( ; iter != iter_end; ++iter )
    {
        stringstream ss;
        ss
                << iter->first.tm_year + 1900
                << "/"
                << iter->first.tm_mon + 1
                << "/"
                << iter->first.tm_mday;
        cout << setw( 7 ) << left << ss.str() << ": " << iter->second << '\n';
    }
}
//=============================================================================
string CMonthlyDownloads::print_chart()
{
    stringstream chart;
    chart
            << "<script type=\"text/javascript\">\n"
            << "function drawVisualization() {\n"
            << "    // Create and populate the data table.\n"
            << "    var data = new google.visualization.DataTable();\n"
            << "    data.addColumn(\'string\', \'Month\');\n";

    years_t::iterator y = m_years.begin();
    years_t::iterator y_end = m_years.end();
    for( size_t index = 1 ; y != y_end; ++y, ++index )
    {
        y->second = index;

        stringstream y_ss;
        y_ss << y->first + 1900;
        chart << "data.addColumn(\'number\', \'" << y_ss.str() << "\');\n";
    }

    chart << "data.addRows(12);\n";

    for( size_t i = 0; i < 12; ++i )
    {
        chart
                << "data.setValue(" << i << ", "
                << "0 , \'" << months[i] << "\');\n";
    }

    container_t::const_reverse_iterator iter = m_container.rbegin();
    container_t::const_reverse_iterator iter_end = m_container.rend();
    for( ; iter != iter_end; ++iter )
    {
        chart
                << "data.setValue(" << iter->first.tm_mon
                << ", " << ( m_years[iter->first.tm_year] )
                << ", " << iter->second << ");\n";
    }

    chart
            << "// Create and draw the visualization.\n"
            << "new google.visualization.LineChart(document.getElementById(\'monthly_downloads\')).\n"
            << "draw(data, {interpolateNulls: 'true',\n"
            << "width: 900, height: 400, pointSize:7,\n"
            << "title: \"Monthly Download Statistics\" }\n"
            << "     );\n"
            << "}\n"
            << "google.setOnLoadCallback(drawVisualization);\n"
            << "</script>\n";

    // Downloads per year
    chart
            << "<script type=\"text/javascript\">\n"
            << "function drawVisualization() {\n"
            << "    // Create and populate the data table.\n"
            << "    var data = new google.visualization.DataTable();\n"
            << "    data.addColumn('string', 'Date');\n"
            << "    data.addColumn('number', 'unique IP addresses');\n"
            << "    data.addColumn('number', 'duplicate IP addresses');\n";

    chart << "data.addRows(" << m_yearsDownloads.size() << ");\n";

    yearsEx_t::const_iterator ir = m_yearsDownloads.begin();
    yearsEx_t::const_iterator ir_end = m_yearsDownloads.end();
    for( size_t count = 0 ; ir != ir_end; ++ir, ++count )
    {
        chart
                << "data.setCell(" << count << ", 0, '"
                << ir->first + 1900 << "');\n";
        chart
                << "data.setCell(" << count << ", 2, " << ir->second.m_downloads - ir->second.m_uniqueIPs.size() << ");\n";

        chart
                << "data.setCell(" << count << ", 1, " << ir->second.m_uniqueIPs.size() << ");\n";

    }

    chart
            << "// Create and draw the visualization.\n"
            << "new google.visualization.ColumnChart(document.getElementById(\'downloads_per_year\')).\n"
            << "draw(data, {\n"
            << "width: 900, height: 400,\n"
            << "isStacked:true,\n"
            << "title: \"Downloads per year\" }\n"
            << "     );\n"
            << "}\n"
            << "google.setOnLoadCallback(drawVisualization);\n"
            << "</script>\n";

    return chart.str();
}
