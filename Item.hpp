#ifndef ITEM_HPP
#define ITEM_HPP

#include <iostream>

/*!
 * \brief Item
 */
typedef unsigned int Item;

/*!
 * \brief The item_hash class
 */
class item_hash {
public:
    std::size_t operator()( const Item & item ) const
    {
        return std::size_t( item );
    }
};

/* For more advanced Item struct
inline std::ostream & operator << ( std::ostream & os, const Item & item )
{
    os << item;
    return os;
}
*/

#endif // ITEM_HPP
