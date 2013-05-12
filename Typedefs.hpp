#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

typedef char Item;
typedef unsigned int TID; // Transaction Id

typedef std::vector < Item > Itemset;
typedef std::vector < Itemset > Database;
typedef std::vector < TID > Tidset;

//typedef Itemset C_Set;

inline std::ostream & operator << ( std::ostream & os, const Itemset & itemset )
{
    unsigned int index = itemset.size();
    if ( index ) {
        os << '(';
        std::for_each( itemset.cbegin(), itemset.cend(), [&]( const Item & item ) {
            os << item << ( --index ? ' ' : ')' );
        } );
    }
    return os;
}


inline std::ostream & operator << ( std::ostream & os, const Tidset & tidset )
{
    unsigned index = tidset.size();
    if ( index ) {
        os << '<';
        std::for_each( tidset.cbegin(), tidset.cend(), [&]( const TID & tid ) {
            os << tid << ( --index ? ',' : '>' );
        } );
    }
    return os;
}

#endif // TYPEDEFS_HPP
