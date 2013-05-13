#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "Itemset.hpp"

/*!
 * \brief Database
 */
typedef std::vector < Itemset > Database;

/*!
 * \brief operator <<
 * \param os
 * \param database
 * \return
 */
inline std::ostream & operator << ( std::ostream & os, const Database & database )
{
    std::for_each( database.cbegin(), database.cend(), [&]( const Itemset & itemset ) {
        std::for_each( itemset.cbegin(), itemset.cend(), [&]( const Item & item ) {
            os << item << ' ';
        } );
        os << '\n';
    } );
    return os;
}


#endif // DATABASE_HPP
