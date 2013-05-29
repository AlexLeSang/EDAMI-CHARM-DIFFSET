#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

// Place for short typedefs

#include <string>

//#define CHESS
//#define MUSHROOM
//#define CONNECT

#ifdef CHESS
constexpr unsigned int n_of_fields = 37; // Chess
typedef unsigned int Item;
#endif

#ifdef MUSHROOM
constexpr unsigned int n_of_fields = 23; // Mushroom
typedef char Item;
#endif


#ifdef CONNECT
constexpr unsigned int n_of_fields = 43; // Connect
typedef char Item;
#endif

#ifdef SIMPLE_TEST
constexpr unsigned int n_of_fields = 22;
typedef char Item;
#endif

#endif // TYPEDEFS_HPP
