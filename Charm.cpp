#include "Charm.hpp"

#include "Diffset.hpp"

#include <iterator>
#include <cassert>

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
                const Node n( itemset, key_value.second, & root_node );
                root_node.add_child( n );
            }
        } );
    }
    auto c_set = CSet();
    charm_extend( root_node, c_set, min_sup );

//        std::cerr << "c_set: \n" << c_set << std::endl;
    return c_set;
}


void Charm::charm_extend(Node &p_tree, CSet &c_set, const unsigned int min_sup)
{
    static unsigned int recursion_level = 0;
    ++ recursion_level;

//    std::cerr << "\nrecursion_level = " << recursion_level << std::endl;

//    std::cerr << "p_tree: " << p_tree << std::endl;
//    std::for_each( p_tree.children().cbegin(), p_tree.children().cend(), []( const std::shared_ptr<Node> & val_ref ) { std::cerr << (*val_ref) << std::endl; } );

    /*
    auto root = p_tree.root_node().lock();
    for ( auto it = root->children().begin(); it != root->children().end(); ++ it ) {
        Node & Xi = (*(*it));
        if ( Xi.is_erased() ) continue;
        Tree p_i_tree( Xi );
        Itemset X = Xi.itemset();
        Diffset Y = Xi.diffset();
        Node test_node; // It also should be updated
        // For each Xj
        for ( auto internal_it = it + 1; internal_it != root->children().end(); ++ internal_it ) {
            Node & Xj = (*(*internal_it));
            if ( Xj.is_erased() ) continue;
            X = Xi.itemset();
            Y = Xi.diffset();
            itemset_union( X, Xj );
            diffset_difference( Y, Xj );
            Node test_node = Node( X, Y, &Xi );
            charm_property( p_i_tree, p_tree, test_node, Xi, Xj, min_sup );
        }
        if ( ! ( root->children().empty() ) ) {
            charm_extend( p_i_tree, c_set, min_sup );
        }
        if ( test_node.itemset().empty() ) {
            if ( ! is_subsumed( c_set, Xi ) ) {
                c_set.insert( CSet::value_type( cset_key_t( Xi.diffset(), Xi.parent()->hashkey() ), cset_val_t( Xi.itemset(), Xi.sup() ) ) );
            }
        }
        else {
            if ( ! is_subsumed( c_set, test_node ) ) {
                c_set.insert( CSet::value_type( cset_key_t( test_node.diffset(), test_node.parent()->hashkey() ), cset_val_t( test_node.itemset(), test_node.sup() ) ) );
            }
        }
    }
    */


    for ( auto it = p_tree.children().cbegin(); it != p_tree.children().cend(); ++ it ) {
        Node & current_child = (*(*it));
        if ( current_child.is_erased() ) continue;
//        std::cerr << "\ncurrent_child = " << current_child << std::endl;
        for ( auto int_it = it + 1; int_it != p_tree.children().cend(); ++ int_it ) {
            Node & internal_child = (*(*int_it));
            if ( internal_child.is_erased() ) continue;
//            std::cerr << "\n  internal_child = " << internal_child << std::endl;
            Itemset X = current_child.itemset();
            Diffset Y = current_child.diffset();

            itemset_union( X, internal_child );
            diffset_difference( Y, internal_child );
            Node test_node( X, Y, &current_child );
//            std::cerr << "  test_node = " << test_node << std::endl;

            if ( test_node.sup() >= min_sup ) {
                if ( current_child.equal( internal_child ) ) {
//                    std::cerr << "   property_1" << std::endl;
//                    std::cerr << "Erase " << internal_child << std::endl;
                    internal_child.set_erased();
                    const Itemset replased_itemset = current_child.itemset();
                    replace_item( current_child, replased_itemset, test_node.itemset() );
                }
                else {
                    if ( internal_child.is_superset_of( current_child ) ) {
//                        std::cerr << "   property_2" << std::endl;
                        const Itemset replased_itemset = current_child.itemset();
                        replace_item( current_child, replased_itemset, test_node.itemset() );
                    }
                    else {
                        if ( current_child.is_superset_of( internal_child ) ) {
//                            std::cerr << "   property_3" << std::endl;
//                            std::cerr << "Erase " << internal_child << std::endl;
                            internal_child.set_erased();
                            current_child.add_child( test_node );
                        }
                        else {
//                            std::cerr << "   property_4" << std::endl;
                            current_child.add_child( test_node );
                        }
                    }
                }
            }
            else{
//                std::cerr << "   rejected" << std::endl;
            }
        }

        charm_extend( current_child, c_set, min_sup );
    }

    if ( ! p_tree.itemset().empty() ) {
        if ( ! is_subsumed( c_set, p_tree ) ) {
//            std::cerr << "\nInsert: " << p_tree << std::endl;
            c_set.insert( CSet::value_type( cset_key_t( p_tree.diffset(), p_tree.parent()->hashkey() ), cset_val_t( p_tree.itemset(), p_tree.sup() ) ) );
        }
    }

    -- recursion_level;
}


void Charm::replace_item(Node& node_ref, const Itemset &itemset, const Itemset &itemset_to)
{
    assert( itemset.size() != 0 );
    assert( itemset_to.size() != 0 );
    if ( node_ref.itemset().size() >= itemset.size() ) {
        if ( ! std::includes( node_ref.itemset().cbegin(), node_ref.itemset().cend(), itemset_to.cbegin(), itemset_to.cend() ) ) {
            // Look for all items
            if ( std::includes( node_ref.itemset().cbegin(), node_ref.itemset().cend(), itemset.cbegin(), itemset.cend() ) ) {
                // Remove all occurence of itemset
                std::for_each( itemset.cbegin(), itemset.cend(), [&]( const Item & item ) {
                    node_ref.itemset().erase( std::find( node_ref.itemset().begin(), node_ref.itemset().end(), item ) );
                } );
                // Insert new sequence
                std::for_each( itemset_to.cbegin(), itemset_to.cend(), [&]( const Item & item_to ) {
                    node_ref.itemset().push_back( item_to );
                } );
                // Sort it
                std::sort( node_ref.itemset().begin(), node_ref.itemset().end() );
            }
        }
    }
    // Apply for each children
    std::for_each( node_ref.children_ref().begin(), node_ref.children_ref().end(), [&]( std::shared_ptr<Node> & shared_node ) {
        replace_item( (*shared_node), itemset, itemset_to );
    } );
}



/*!
 * \brief Charm::is_subsumed
 * \param c_set
 * \param node
 * \return
 */
bool Charm::is_subsumed(const CSet & c_set, const Node & node)
{
    //    std::cerr << "Candidate for insertion : " << node << std::endl; // TODO remvoe debug output

    const Itemset & X = node.itemset();
    const Diffset & Y = node.diffset();

    bool is_subsumed = false;
    //    const int parent_hashkey = node.hashkey() + std::accumulate( node.diffset().cbegin(), node.diffset().cend(), 0 );
    //    std::cerr << "parent_hashkey = " << parent_hashkey << std::endl;
    const int hashkey = diffset_hash::hash( std::make_pair( Y, node.parent()->hashkey() ) );
    //    std::cerr << "hashkey = " << hashkey << std::endl;
    const auto range = c_set.equal_range( std::make_pair( Y, hashkey ) );
    for ( auto it = range.first; it != range.second; ++ it ) {
        const Itemset & C = (*it).second.first;
        const auto sup = (*it).second.second;
        //        std::cerr << "C = " << C << std::endl;
        //        std::cerr << "sup = " << sup << std::endl;
        if ( node.sup() == sup ) {
            const bool includes = std::includes( C.cbegin(), C.cend(), X.cbegin(), X.cend() );
            if ( includes ) {
                is_subsumed = true;
                break;
            }
        }
    }
    return is_subsumed;

    // Based on diffset
    /*
    {
        bool is_subsumed = false;
        const auto range = c_set.equal_range( Y );
        for ( auto it = range.first; it != range.second; ++ it ) {
            const Tidset & tidset = (*it).first;
            if ( tidset.size() == Y.size() ) {
                const Itemset & C = (*it).second;
                if ( std::includes( C.cbegin(), C.cend(), X.cbegin(), X.cend() ) ) {
                    is_subsumed = true;
                    break;
                }
            }
        }
        return is_subsumed;
    }
    */

}

/*!
 * \brief Charm::itemset_union
 * \param X
 * \param Xi
 * \param Xj
 */
void Charm::itemset_union(Itemset &X, const Node &Xj)
{
    Itemset union_itemset( X.size() + Xj.itemset().size() );
    auto it_union = std::set_union( X.cbegin(), X.cend(), Xj.itemset().cbegin(), Xj.itemset().cend(), union_itemset.begin() );
    union_itemset.resize( it_union - union_itemset.begin() );
    X = union_itemset;
}

/*!
 * \brief Charm::diffset_difference
 * \param Y
 * \param Xj
 */
void Charm::diffset_difference(Diffset & Y, const Node &Xj)
{
    Diffset result_diffset( Y.size() + Xj.diffset().size() );
    //        auto it = std::set_difference( Y.cbegin(), Y.cend(), Xj.diffset().cbegin(), Xj.diffset().cend(), result_diffset.begin() );
    auto it = std::set_difference( Xj.diffset().cbegin(), Xj.diffset().cend(), Y.cbegin(), Y.cend(), result_diffset.begin() );
    result_diffset.resize( it - result_diffset.begin() );
//    Y = result_diffset;
    std::swap( Y, result_diffset );
}
