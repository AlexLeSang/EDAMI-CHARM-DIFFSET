#ifndef ITEM_HPP
#define ITEM_HPP

#include <iostream>
#include "Typedefs.hpp"

/*!
 * \brief The item_hash class
 */
class item_hash {
public:
    inline std::size_t operator()( const Item & item ) const
    {
        return std::size_t( item );
    }
};
#endif // ITEM_HPP
