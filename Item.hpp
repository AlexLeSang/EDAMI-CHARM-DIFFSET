#ifndef ITEM_HPP
#define ITEM_HPP

#include <iostream>
#include "Typedefs.hpp"

/*!
 * \brief The item_hash class
 */
class item_hash {
public:
    inline int operator()( const Item & item ) const
    {
        return (int)( item );
    }
};
#endif // ITEM_HPP
