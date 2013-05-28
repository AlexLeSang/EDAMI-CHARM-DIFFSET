#ifndef DIFFSET_HPP
#define DIFFSET_HPP

#include <vector>
#include <iostream>
#include <tuple>
#include <algorithm>

typedef unsigned int TID; // Transaction Id

typedef std::vector< TID > Diffset;

typedef std::vector< TID > Tidset;


class diffset_hash {
public:
    /*!
     * \brief operator ()
     * \param diffset_pair first: First: node diffset, Second: parent hashkey
     * \return
     */
    int operator ()( const std::pair< const Diffset&, int > & diffset_pair ) const
    {
        const auto hash_value = diffset_pair.second - std::accumulate( diffset_pair.first.cbegin(), diffset_pair.first.cend(), 0 );
//        std::cerr << "diffset_hash: [" << diffset_pair.first << " : " << diffset_pair.second << "] = " << hash_value << std::endl;
        return hash_value;
    }

    /*!
     * \brief hash
     * \param diffset_pair
     * \return
     */
    static int hash( const std::pair< const Diffset&, int > & diffset_pair )
    {
        static const diffset_hash hash;
        return hash( diffset_pair );
    }
};

///*
#ifdef SIMPLE_TEST
inline std::ostream & operator << ( std::ostream & os, const Diffset & diffset )
{
    unsigned index = diffset.size();
    if ( index ) {
        os << '<';
        std::for_each( diffset.cbegin(), diffset.cend(), [&]( const TID & tid ) {
            os << tid << ( --index ? ',' : '>' );
        } );
    }
    return os;
}
//*/
#endif
/*
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
*/

#endif // DIFFSET_HPP
