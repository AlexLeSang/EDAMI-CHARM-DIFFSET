#include "Charm.hpp"
#include "CSet.hpp"
#include "ResultSaver.hpp"
#include "DatabaseReader.hpp"
#include "Typedefs.hpp"

#include <stdexcept>


#if !(defined CSET_TEST)

void print_usage();

/*!
 * \brief main
 * \param argc
 * \param argv
 * \return
 */
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
    { // WARNING Debug output
        std::cout << "Input args: \n";
        std::for_each ( argv_vector.cbegin(), argv_vector.cend(), []( const std::string & str ) {
            std::cout << str << ' ';
        } );
        std::cout << std::endl;
    }
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
            DatabaseReader< n_of_fields >::read_database( data_stream, database );
            std::cerr << "Database was read" << std::endl;
        }
        else {
            std::cerr << "Cannot open file: " << database_filename << std::endl;
            print_usage();
            return -1;
        }
        std::cerr << "Database size: " << database.size() << std::endl;
        //        std::cerr << database << std::endl; // TODO remvoe debug output
    }

    const auto c_set = Charm::charm( database, min_sup );
    std::cout << "Number of frequent itemsets: " << c_set.size() << std::endl;
    //    std::cout << "c_set = " << c_set << std::endl; // TODO remvoe debug output
    // Save results
    {
        std::ofstream c_set_stream;
        const std::string & result_filename( argv_vector.at( 2 ) );
        c_set_stream.open( result_filename );
        if ( c_set_stream.is_open() ) {
            ResultSaver::save(  c_set_stream, c_set );
            std::cout << "Results was saved" << std::endl;
        }
        else {
            std::cerr << "Cannot open file: " << result_filename << std::endl;
            print_usage();
            return -1;
        }
    }
    return 0;
}


#endif

/*!
 * \brief print_usage
 */
void print_usage()
{
    std::cerr << "Usage: min_sup input.dat output.res" << std::endl;
}



#ifdef CSET_TEST
#include "Node.hpp"

int main()
{
    // Universe
    const unsigned int universe_size = 13;
    Tidset universe( universe_size );
    std::for_each( universe.begin(), universe.end(), []( decltype(universe)::reference val_ref ) {
        static unsigned int counter = 1;
        val_ref = counter++;
    } );
//    std::cerr << "universe = " << universe << std::endl;

    // Nodes
    {
        const Diffset diffset = { 1, 3, 5, 7, 9 };
        const Tidset itemset = { 22, 33, 44, 55 };
        const unsigned int sup = universe_size - diffset.size();
//        std::cerr << "itemset = " << itemset << std::endl;
        Tidset local_tidset;
        std::for_each( universe.cbegin(), universe.cend(), [&]( decltype(universe)::const_reference val_ref ) {
            if ( diffset.cend() == std::find( diffset.cbegin(), diffset.cend(), val_ref ) ) {
                local_tidset.push_back( val_ref );
            }
        } );
//        std::cerr << "local_tidset = " << local_tidset << std::endl;
//        std::cerr << "diffset = " << diffset << std::endl;
        const int hash = std::accumulate( local_tidset.cbegin(), local_tidset.cend(), 0 );
//        std::cerr << "sup = " << sup << std::endl;
//        std::cerr << "hash = " << hash << std::endl;

        Node n( itemset, diffset, sup, hash );
        std::cerr << "n = " << n << std::endl;


        Tidset child_itemset( itemset.size() - 1 );
        std::copy( itemset.cbegin(), itemset.cend() - 1, child_itemset.begin() );
//        child_itemset.push_back( 55 );
//        std::cerr << "child_itemset = " << child_itemset << std::endl;

        Diffset child_diffset;
//        Diffset child_diffset( diffset.size() );
//        std::copy( diffset.cbegin(), diffset.cend(), child_diffset.begin() );
//        child_diffset.push_back( 11 );
//        child_diffset.push_back( 12 );


        Tidset child_local_tidset;
        std::for_each( universe.cbegin(), universe.cend(), [&]( decltype(universe)::const_reference val_ref ) {
            if ( child_diffset.cend() == std::find( child_diffset.cbegin(), child_diffset.cend(), val_ref ) ) {
                child_local_tidset.push_back( val_ref );
            }
        } );

//        std::cerr << "child_local_tidset = " << child_local_tidset << std::endl;
//        std::cerr << "child_diffset = " << child_diffset << std::endl;

        const Node n1( child_itemset, child_diffset, &n );
        std::cerr << "n1 = " << n1 << std::endl;

        const int man_hash = diffset_hash::hash( std::make_pair( n1.diffset(), n1.parent()->hashkey() ) );
        std::cerr << "man_hash = " << man_hash << std::endl;


        const int super_hash = std::accumulate( universe.cbegin(), universe.cend(), 0 );
        std::cerr << "super_hash = " << super_hash << std::endl;
        CSet c_set;
        c_set.insert( CSet::value_type( cset_key_t( n.diffset(), super_hash ), cset_val_t( n.itemset(), n.sup() ) ) );
//        c_set.insert( CSet::value_type( std::make_pair( n1.diffset(), n1.parent()->hashkey() ), n1.itemset()) );
        std::cerr << "c_set = " << c_set << std::endl;


        c_set.insert( CSet::value_type( cset_key_t( n1.diffset(), n1.parent()->hashkey() ), cset_val_t( n1.itemset(), n1.sup() ) ) );
        // Check all buckets
        {
            std::for_each( c_set.cbegin(), c_set.cend(), [&]( decltype(c_set)::const_reference val_ref ) {
                std::cerr << "Element [" << val_ref.second.first <<  " : " << val_ref.second.second << "] is in bucket #" << c_set.bucket( val_ref.first ) << std::endl;
            } );
        }


        const bool is_sumsumed = Charm::is_subsumed( c_set, n1 );
        std::cerr << "is_sumsumed = " << std::boolalpha << is_sumsumed << std::endl;
    }
}

#endif
