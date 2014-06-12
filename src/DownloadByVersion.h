//
//  DownloadByVersion.h
//  pod-web-stat
//
//  Created by Anar Manafov on 12/24/10.
//  Copyright 2010 GSI. All rights reserved.
//
#ifndef DownloadsByVersion_H_
#define DownloadsByVersion_H_
//=============================================================================
// STD
#include <set>
// pod-web-stat
#include "local_defs.h"
//=============================================================================
// for version like x.y
inline void Parse(int _result[2], const std::string& _input)
{
    std::istringstream parser(_input);
    parser >> _result[0];
    parser.get(); //Skip period
    parser >> _result[1];
}
struct version_comp {
    bool operator() (const std::string& _v1, const std::string & _v2) const
    {
        int parsed1[2];
        int parsed2[2];
        Parse(parsed1, _v1);
        Parse(parsed2, _v2);
        return (std::lexicographical_compare(parsed1, parsed1 + 2, parsed2, parsed2 + 2));
    }
};
//=============================================================================
class CDownloadsByVersion
{
        struct SValue
        {
            size_t m_count;
            std::set<std::string> m_uniqueIPs;
        };
        typedef std::map<std::string, SValue, version_comp > container_t;

    public:
        void operator()( const InfoByFile_t::value_type& _data );

        void print() const;
        std::string print_chart();

    private:
        container_t m_container;
};


#endif
