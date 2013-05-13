#ifndef TIDSET_HPP
#define TIDSET_HPP

#include <vector>
#include <istream>

typedef unsigned int TID; // Transaction Id
typedef std::vector < TID > Tidset;

class tidset_hash {
public:
    unsigned int operator ()( const Tidset & tidset ) const
    {
        return std::accumulate( tidset.cbegin(), tidset.cend(), 0 );
    }
};

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

#endif // TIDSET_HPP
