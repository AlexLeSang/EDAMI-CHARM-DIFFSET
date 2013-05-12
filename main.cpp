#include "Tree.hpp"
#include "Typedefs.hpp"

#ifndef TREE_TEST

// Declarations
void print_usage();

class item_hash {
public:
    std::size_t operator()( const Item & item ) const
    {
        return std::size_t( item );
    }
};

class tidset_hash {
public:
    unsigned int operator ()( const Tidset & tidset ) const
    {
        return std::accumulate( tidset.cbegin(), tidset.cend(), 0 );
    }
} tidset_hash_functor;

typedef std::unordered_multimap< Tidset, Itemset, tidset_hash > C_Set;

inline std::ostream & operator << ( std::ostream & os, const C_Set & c_set )
{
    std::for_each( c_set.cbegin(), c_set.cend(), [&]( C_Set::const_reference entries ) {
        os << entries.second << ' ' << entries.first << '\n';
    } );
    return os;
}

// Charm algo
C_Set charm( const Database & database, const unsigned int min_sup );

void charm_extend(Tree & p_tree, C_Set &c_set , const unsigned int min_sup);

// Realizations
int main( int argc, const char * argv[] )
{
    if ( argc != 4 ) {
        print_usage();
        return -1;
    }

    std::vector < std::string > argv_vector( argc - 1 );
    auto argv_index = argc - 1;
    while ( argv_index -- ) {
        argv_vector[ argv_index ] = std::string( argv[ argv_index + 1 ] );
    }
    std::cout << "Input args: \n";
    std::for_each ( argv_vector.cbegin(), argv_vector.cend(), []( const std::string & str ) {
        std::cout << str << '\n';
    } );


    // Read support
    unsigned int min_sup = 0;
    try {
        min_sup = std::stoi( argv_vector.at( 0 ) );
    }
    catch ( const std::invalid_argument & ia ) {
        std::cerr << "Invalid argument: " << ia.what() << '\n';
        print_usage();
        return -1;
    }

    // Read database
    Database database;
    {
        std::ifstream data_stream;
        const std::string & database_filename = argv_vector.at( 1 );
        data_stream.open( database_filename );
        if ( data_stream.is_open() ) {
            // Read database
            std::string s;
            while ( !data_stream.eof() ) {
                std::getline( data_stream, s );
                if ( s.size() ) {
                    Itemset itemset( s.size() );
                    std::copy( s.cbegin(), s.cend(), itemset.begin() );
                    database.push_back( itemset );
                }
            }
        }
        else {
            std::cerr << "Cannot open file: " << database_filename << std::endl;
            print_usage();
            return -1;
        }

        // TODO remove debug output
        /*
        {
            std::cerr << "Database: \n";
            std::for_each( database.cbegin(), database.cend(), []( const Itemset & itemset ) {
                std::for_each( itemset.cbegin(), itemset.cend(), []( const Item & item ) {
                    std::cerr << item << ' ';
                } );
                std::cerr << '\n';
            } );
        }
        */
    }
    auto C = charm( database, min_sup );

    return 0;
}


void print_usage()
{
    std::cerr << "Usage: min_sup input.dat output.dat" << std::endl;
}


C_Set charm( const Database & database, const unsigned int min_sup )
{
    Tree p;
    // Create a P tree
    {
        std::unordered_map< Item, Tidset, item_hash > item_map;
        // For each itemset
        TID transaction_counter = 1;
        std::for_each( database.cbegin(), database.cend(), [&] ( const Itemset & itemset ) {
            // For each item in itemset
            std::for_each( itemset.cbegin(), itemset.cend(), [&]( const Item & item ) {
                // If there is such item increase support
                auto got = item_map.find( item );
                if ( item_map.cend() != got ) {
                    got->second.push_back( transaction_counter );
                }
                // Else add item with sup == 1
                else {
                    Tidset new_tidset( 1 );
                    new_tidset.at( 0 ) = transaction_counter;
                    item_map.insert( std::make_pair( item, new_tidset ) );
                }
            } );

            ++ transaction_counter;
        } );
        // TODO remove debug output
        /*
        {
            std::cerr << "item_map = \n";
            std::for_each( item_map.cbegin(), item_map.cend(), []( decltype(item_map)::const_reference key_value ) {
                std::cerr << "(" << key_value.first << ":<";
                unsigned int index = key_value.second.size();
                std::for_each( key_value.second.cbegin(), key_value.second.cend(), [&]( const TID & tid ) {
                    std::cerr << tid << ( --index ? ',' : '>' );
                } );
                std::cerr << ")\n";
            } );
        }
        */
        // Fill the tree
        {
            std::for_each( item_map.cbegin(), item_map.cend(), [&]( decltype(item_map)::const_reference key_value ) {
                if ( min_sup < key_value.second.size() ) {
                    Itemset itemset;
                    itemset.push_back( key_value.first );
                    p.add( itemset, key_value.second );
                }
            } );
            //            P.print_tree(); // TODO remove debug output
        }

    }
    auto c_set = C_Set();
    charm_extend( p, c_set, min_sup );
    std::cerr << "c_set = \n" << c_set << std::endl; // TODO remove debug output
    return c_set;
}

void itemset_union(Itemset & X, Node & Xi, Node & Xj)
{
    if ( ! std::is_sorted( X.cbegin(), X.cend() ) ) {
        std::sort( X.begin(), X.end() );
    }
    if ( ! std::is_sorted( Xi.itemset().cbegin(), Xi.itemset().cend() ) ) {
        std::sort( Xi.itemset().begin(), Xi.itemset().end() );
    }
    //    std::cerr << "\nX = " << X << std::endl; // TODO remove debug output
    //    std::cerr << "Xj = " << Xj.itemset() << std::endl; // TODO remove debug output
    Itemset union_itemset( X.size() + Xj.itemset().size() );
    auto it_union = std::set_union( X.cbegin(), X.cend(), Xj.itemset().cbegin(), Xj.itemset().cend(), union_itemset.begin() );
    union_itemset.resize( it_union - union_itemset.begin() );
    X = union_itemset;
    //            std::cerr << "Union = " << X << std::endl; // TODO remove debug output
}

void tidset_intersection(Node& Xj, Tidset & Y)
{
    if ( ! std::is_sorted( Y.cbegin(), Y.cend() ) ) {
        std::sort( Y.begin(), Y.end() );
    }
    //                std::cerr << "t(Xi) = " << Y << std::endl; // TODO remove debug output
    //                std::cerr << "t(Xj) = " << Xj.tidset() << std::endl; // TODO remove debug output
    const auto it = std::set_intersection( Y.cbegin(), Y.cend(), Xj.tidset().cbegin(), Xj.tidset().cend(), Y.begin() );
    Y.resize( it - Y.begin() );
    //            std::cerr << "Intersection = " << Y << std::endl; // TODO remove debug output
}

void property_1(Tree & p_i_tree, Tree & p_tree, Node & Xj, Node & Xi, Node & test_node)
{
    std::cerr << "Property 1 applyed" << std::endl; // TODO remove debug output
    p_tree.remove( Xj.itemset() );
    const auto itemset_to_replace = Xi.itemset();
    p_i_tree.replace( itemset_to_replace, test_node.itemset() );
    p_tree.replace( itemset_to_replace, test_node.itemset() );
}

void property_2(Tree & p_tree, Tree & p_i_tree, Node & test_node, Node & Xi)
{
    std::cerr << "Property 2 applyed" << std::endl; // TODO remove debug output
    const auto itemset_to_replace = Xi.itemset();
    p_i_tree.replace( itemset_to_replace, test_node.itemset() );
    p_tree.replace( itemset_to_replace, test_node.itemset() );
}

void property_3(Tree & p_i_tree, Tree & p_tree, Node & Xj, Node & test_node)
{
    std::cerr << "Property 3 applyed" << std::endl; // TODO remove debug output
    p_tree.remove( Xj.itemset() );
    p_i_tree.add( test_node.itemset(), test_node.tidset() );
}

void property_4(Tree & p_i_tree, Node & test_node)
{
    std::cerr << "Property 4 applyed" << std::endl; // TODO remove debug output
    p_i_tree.add( test_node.itemset(), test_node.tidset() );
}

bool tidset_equal(const Node& Xi, const Node& Xj)
{
    return ( ( Xi.tidset().size() == Xj.tidset().size() ) && std::equal( Xi.tidset().cbegin(), Xi.tidset().cend(), Xj.tidset().cbegin() ) );
}

bool is_subset(const Node& subset, const Node& superset)
{
    return std::includes( superset.tidset().cbegin(), superset.tidset().cend(), subset.tidset().cbegin(), subset.tidset().cend() );
}

void charm_property(Tree & p_i_tree, Tree & p_tree, Node & test_node, Node & Xi, Node & Xj, const unsigned int min_sup)
{
    if ( test_node.tidset().size() >= min_sup ) {
        // t(Xi) == t(Xj)
        if ( tidset_equal(Xi, Xj) ) {
            property_1( p_i_tree, p_tree, Xj, Xi, test_node );
        }
        else {
            // Find if t(Xi) is a subset of t(Xj)
            if ( is_subset( Xi, Xj ) ) {
                property_2( p_tree, p_i_tree, test_node, Xi );
            }
            else {
                // Find if t(Xj) is a subset of t(Xi)
                if ( is_subset( Xj, Xi ) ) {
                    property_3( p_i_tree, p_tree, Xj, test_node );
                }
                else {
                    property_4( p_i_tree, test_node );
                }
            }
        }
    }
    else {
        std::cerr << "Low support, rejected\n" << std::endl; // TODO remove debug output
    }
}

bool is_subsumed(const C_Set &c_set, const Itemset & X, const Tidset & Y)
{
    bool is_subsumed = false;
    std::cerr << "Node to add to C = Itemset:" << X << " Tidset: " << Y << std::endl; // TODO remove debug output
    std::cerr << "c_set = \n" << c_set << std::endl; // TODO remove debug output
    // Obtaine all elements with the same hash
    const auto range = c_set.equal_range( Y );
    for ( auto it = range.first; it != range.second; ++ it ) {
        const Tidset & tidset = (*it).first;
        // Check if sup(C) == sup(Y)
        if ( tidset.size() == Y.size() ) {
            // Check if X is a subset of C
            const Itemset & C = (*it).second;
            if ( std::includes( C.cbegin(), C.cend(), X.cbegin(), X.cend() ) ) {
                is_subsumed = true;
                break;
            }
        }
    }
    return is_subsumed;
}

void charm_extend(Tree &p_tree, C_Set &c_set, const unsigned int min_sup)
{
    auto root = p_tree.root_node().lock();
    // For each Xi in P
    for ( auto it = root->children().begin(); it != root->children().end(); ++ it ) {
        Tree p_i_tree;
        Node & Xi = (*(*it));
        Itemset X = Xi.itemset();
        Tidset Y = Xi.tidset();

        // For each Xj
        for (  auto internal_it = it + 1; internal_it != root->children().end(); ++ internal_it ) {
            if ( !(*internal_it) ) break; // WARNING iterator magic

            X = Xi.itemset();

            Node & Xj = (*(*internal_it));
            itemset_union( X, Xi, Xj );

            Y = Xi.tidset();

            tidset_intersection( Xj, Y );

            Node test_node( X, Y );
            // std::cerr << "test_node = " << test_node << std::endl; // TODO remove debug output
            // Charm property
            charm_property( p_i_tree, p_tree, test_node, Xi, Xj, min_sup );
        }

        // If |Pi|
        if ( ! ( root->children().empty() ) ) {
            charm_extend( p_i_tree, c_set, min_sup );
        }
        // Delete |Pi|
        // Check for subsumed and add to C
        if ( ! is_subsumed( c_set, X, Y ) ) {
            c_set.insert( C_Set::value_type(Y, X) );
        }
    }
}

#endif
