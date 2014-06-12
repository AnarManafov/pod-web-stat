//
//  MonthlyDownloads.h
//  pod-web-stat
//
//  Created by Anar Manafov on 12/24/10.
//  Copyright 2010 GSI. All rights reserved.
//
#ifndef MonthlyDownloads_H_
#define MonthlyDownloads_H_
//=============================================================================
// STD
#include <map>
#include <set>
// pod-web-stat
#include "local_defs.h"
//=============================================================================
struct SYearsInfo
{
    size_t m_downloads;
    std::set<std::string> m_uniqueIPs;
};
//=============================================================================
class CMonthlyDownloads
{
        typedef std::map<tm, size_t, std::greater<tm> > container_t;
        typedef std::map<int, size_t> years_t;
        typedef std::map<int, SYearsInfo> yearsEx_t;

    public:
        CMonthlyDownloads();

    public:
        void operator()( const InfoByFile_t::value_type &_data );

        void print() const;
        std::string print_chart();

    private:
        container_t m_container;
        years_t m_years;
        yearsEx_t m_yearsDownloads;
};

#endif
