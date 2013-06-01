#include "Charm.hpp"

#include "Diffset.hpp"

#include <iterator>
//#include <cassert>

#include <thread>
#include <mutex>
#include <future>
#include <list>


typedef std::unordered_map< Item, Diffset, item_hash > ItemMap;

/*!
 * \brief Charm::charm
 * \param database
 * \param min_sup
 * \return
 */
CSet Charm::charm(const Database &database, const unsigned int min_sup)
{
    // Create a P tree
    ItemMap item_map;
    // For each itemset
    TID transaction_counter = 1;
    std::for_each( database.cbegin(), database.cend(), [&] ( const Itemset & itemset ) {
        // For each item in the itemset
        std::for_each( itemset.cbegin(), itemset.cend(), [&]( const Item & item ) {
            // If there is such item increase it's support
            const auto got = item_map.find( item );
            if ( item_map.cend() != got ) {
                got->second.push_back( transaction_counter );
            }
            else {
                Tidset new_tidset( 1 );
                new_tidset.at( 0 ) = transaction_counter;
                item_map.insert( std::make_pair( item, new_tidset ) );
            }
        } );
        ++ transaction_counter;
    } );
    --transaction_counter;
    std::cerr << "transaction_counter = " << transaction_counter << std::endl;

    // Translate tidset into diffset
    std::for_each( item_map.begin(), item_map.end(), [&]( ItemMap::reference key_value ) {
        const auto tidset = std::move( key_value.second );
        // Transfort to a diffset
        for ( unsigned int index = 1; index <= transaction_counter; ++ index ) {
            if ( tidset.cend() == std::find( tidset.cbegin(), tidset.cend(), index ) ) {
                key_value.second.push_back( index );
            }
        }
    } );

    // Fill the tree
    const unsigned int sum_of_trans_id = transaction_counter * (transaction_counter - 1) / 2;
    Node root_node( Itemset(), Diffset(), transaction_counter, sum_of_trans_id );
    //    Tree p( root_node );
    {
        std::for_each( item_map.cbegin(), item_map.cend(), [&]( ItemMap::const_reference key_value ) {
            if ( min_sup <= (transaction_counter - key_value.second.size()) ) {
                Itemset itemset;
                itemset.push_back( key_value.first );
//                const Node n( itemset, key_value.second, & root_node );
//                root_node.add_child( n );
                Diffset diffset = key_value.second;
                root_node.add_child( std::move( itemset ), std::move( diffset ) );
            }
        } );
    }
    auto c_set = CSet();
    auto t1 = std::chrono::high_resolution_clock::now();
    charm_extend( root_node, c_set, min_sup );
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "\ncharm_extend( root_node, c_set, min_sup ) took " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " milliseconds\n";

    //        std::cerr << "c_set: \n" << c_set << std::endl;
    return c_set;
}
