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

void Charm::charm_extend(Node &p_tree, CSet &c_set, const unsigned int min_sup) {
    static unsigned int recursion_level = 0;
    ++ recursion_level;

    //    std::cerr << "\nrecursion_level = " << recursion_level << std::endl;

    //    std::cerr << "p_tree: " << p_tree << std::endl;
    //    std::for_each( p_tree.children().cbegin(), p_tree.children().cend(), []( const std::shared_ptr<Node> & val_ref ) { std::cerr << (*val_ref) << std::endl; } );

    for ( auto it = p_tree.children().cbegin(); it != p_tree.children().cend(); ++ it ) {
        Node & current_child = (*(*it));
        if ( current_child.is_erased() ) continue;

        for ( auto int_it = it + 1; int_it != p_tree.children().cend(); ++ int_it ) {
            Node & internal_child = (*(*int_it));
            if ( internal_child.is_erased() ) continue;

            static Itemset X;
            X = current_child.itemset();
            static Diffset Y;
            Y = current_child.diffset();

            itemset_union( X, internal_child );
            diffset_difference( Y, internal_child );

            const unsigned int sup = current_child.sup() - Y.size();
            if ( sup >= min_sup ) {
                if ( current_child.equal( internal_child ) ) {
                    internal_child.set_erased();
                    const Itemset replased_itemset = current_child.itemset();
                    replace_item( current_child, replased_itemset, X );
                }
                else {
                    if ( internal_child.is_superset_of( current_child ) ) {
                        const Itemset replased_itemset = current_child.itemset();
                        replace_item( current_child, replased_itemset, X );
                    }
                    else {
                        if ( current_child.is_superset_of( internal_child ) ) {
                            internal_child.set_erased();
                            current_child.add_child( std::move(X), std::move(Y) );
                        }
                        else {
                            current_child.add_child( std::move(X), std::move(Y) );
                        }
                    }
                }
            }
            else {
                //                std::cerr << "   rejected" << std::endl;
            }
        }

        charm_extend( current_child, c_set, min_sup );
    }

    if ( ! p_tree.itemset().empty() ) {
        check_subsumption_and_insert( c_set, p_tree );
    }

    -- recursion_level;
}
