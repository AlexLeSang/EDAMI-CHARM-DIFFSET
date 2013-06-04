#ifndef DIFFSET_HPP
#define DIFFSET_HPP

#include <vector>
#include <iostream>
#include <tuple>
#include <algorithm>

/*!
 * \brief TID
 */
typedef int TID; // Transaction Id

/*!
 * \brief Diffset
 */
typedef std::vector< TID > Diffset;

/*!
 * \brief Tidset
 */
typedef std::vector< TID > Tidset;

class diffset_hash {
public:
    /*!
     * \brief operator ()
     * \param diffset_pair
     * \return
     */
    inline int operator ()( const std::pair< const Diffset&, int > & diffset_pair ) const
    {
        return std::accumulate( diffset_pair.first.cbegin(), diffset_pair.first.cend(), diffset_pair.second, std::minus<TID>() );
    }

    /*!
     * \brief hash
     * \param diffset_pair
     * \return
     */
    inline static int hash( const std::pair< const Diffset&, int > & diffset_pair )
    {
        static const diffset_hash hash;
        return hash( diffset_pair );
    }
};
#endif // DIFFSET_HPP
