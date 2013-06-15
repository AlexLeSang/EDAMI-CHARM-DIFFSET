#ifndef CSET_HPP
#define CSET_HPP

#include <unordered_map>

#include "Itemset.hpp"
#include "Diffset.hpp"

#include <tuple>
#include <utility>


/*!
 * \brief cset_key_t
 */
typedef std::pair< Diffset, int > cset_key_t; // Diffset, parent hashkey

/*!
 * \brief cset_val_t
 */
typedef std::pair< Itemset, int > cset_val_t; // Itemset, support

/*!
 * \brief CSet
 */
typedef std::unordered_multimap< cset_key_t, cset_val_t, diffset_hash > CSet;

/*!
 * \brief operator <<
 * \param os
 * \param c_set
 * \return
 */
inline std::ostream & operator << ( std::ostream & os, const CSet & c_set )
{
    std::for_each( c_set.cbegin(), c_set.cend(), [&]( CSet::const_reference entries ) {
        os << entries.second.first << ' ' << entries.second.second <<  '\n';
    } );
    return os;
}

#endif // CSET_HPP
