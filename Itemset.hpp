#ifndef ITEMSET_HPP
#define ITEMSET_HPP

#include "Item.hpp"

#include <vector>
#include <algorithm>

typedef std::vector < Item > Itemset;

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


#endif // ITEMSET_HPP
