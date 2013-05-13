#ifndef CSET_HPP
#define CSET_HPP

#include <unordered_map>

typedef std::unordered_multimap< Tidset, Itemset, tidset_hash > CSet;

inline std::ostream & operator << ( std::ostream & os, const CSet & c_set )
{
    std::for_each( c_set.cbegin(), c_set.cend(), [&]( CSet::const_reference entries ) {
        os << entries.second << ' ' << entries.first << '\n';
    } );
    return os;
}


#endif // CSET_HPP
