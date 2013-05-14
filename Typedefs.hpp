#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

// Place for short typedefs
constexpr unsigned int n_of_fields = 37;


#include "Node.hpp"
inline unsigned int sup( const Node & node )
{
    return node.tidset().size();
}

#endif // TYPEDEFS_HPP
