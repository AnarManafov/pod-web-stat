//
//  DownloadsByCountry.h
//  pod-web-stat
//
//  Created by Anar Manafov on 12/24/10.
//  Copyright 2010 GSI. All rights reserved.
//
#ifndef DownloadsByCountry_H_
#define DownloadsByCountry_H_
//=============================================================================
// STD
#include <map>
// pod-web-stat
#include "local_defs.h"
//=============================================================================
class CDownloadsByCountry
{
        typedef std::multimap<std::string, size_t > container_t;
        typedef std::multimap<size_t, std::string, std::greater<size_t> > containerSorted_t;
        typedef std::map<std::string, std::string> strToStrMap_t;

    public:
        CDownloadsByCountry( const std::string &_ipToCountryFileName );

        void operator()( const InfoByFile_t::value_type& _data );

        void print();
        std::string print_chart();

    private:
        strToStrMap_t m_IPtoCountry;
        container_t m_container;
};


#endif
