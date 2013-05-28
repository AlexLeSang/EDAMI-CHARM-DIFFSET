#include "Charm.hpp"

#include "Diffset.hpp"

#include <iterator>

typedef std::unordered_map< Item, Diffset, item_hash > ItemMap;

/*!
 * \brief Charm::charm
 * \param database
 * \param min_sup
 * \return
 */
CSet Charm::charm(const Database &database, const unsigned int min_sup)
{
    Tree p;
    // Create a P tree
    {
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

        /*
        {
            std::cerr << "item_map = \n";
            for ( const auto & item : item_map ) {
                std::cerr << item.first << ' ' << item.second << '\n';
            }
            std::cerr << std::endl;
        }
                */

        // Fill the tree
        {
            //            std::cerr << "transaction_counter = " << transaction_counter << std::endl;
            const unsigned int sum_of_trans_id = transaction_counter * (transaction_counter - 1) / 2;
            std::for_each( item_map.cbegin(), item_map.cend(), [&]( ItemMap::const_reference key_value ) {
                if ( min_sup < (transaction_counter - key_value.second.size()) ) {
                    Itemset itemset;
                    itemset.push_back( key_value.first );
                    const unsigned int hash = sum_of_trans_id - std::accumulate( key_value.second.cbegin(), key_value.second.cend(), 0 );
                    const unsigned int sup = (transaction_counter - key_value.second.size());
                    //                    std::cerr << "Item: " << key_value.first << " hash: " << hash << " sup: " << sup << std::endl; // TODO remove debug putput
                    p.add( itemset, key_value.second, sup, hash );
                }
            } );
        }
        //        std::cerr << "p = \n"; p.print_tree(); std::cerr << std::endl; // Chech if the tree was correctly constructed
        //        exit(-1);
    }
    auto c_set = CSet();
    charm_extend( p, c_set, min_sup );

    std::cerr << "c_set: \n" << c_set << std::endl;
    return c_set;
}

/*!
 * \brief Charm::charm_extend
 * \param p_tree
 * \param c_set
 * \param min_sup
 */
void Charm::charm_extend(Tree &p_tree, CSet &c_set, const unsigned int min_sup)
{
    auto root = p_tree.root_node().lock();
    for ( auto it = root->children().begin(); it != root->children().end(); ++ it ) {
        Node & Xi = (*(*it));
        if ( Xi.is_erased() ) continue;
        std::cerr << "Parent node = " << Xi << std::endl;
        Tree p_i_tree;
        Itemset X = Xi.itemset();
        Diffset Y = Xi.diffset();
        Node test_node( Xi );
        // For each Xj
        for ( auto internal_it = it + 1; internal_it != root->children().end(); ++ internal_it ) {
            Node & Xj = (*(*internal_it));
            if ( Xj.is_erased() ) continue;
            std::cerr << "Right node = " << Xj << std::endl;
            X = Xi.itemset();
            Y = Xi.diffset();
            itemset_union( X, Xj );
            diffset_difference( Y, Xj );
//            const Node test_node( X, Y, &Xi );
            Node test_node = Node( X, Y, &Xi );
            std::cerr << "test_node: " << test_node << std::endl; // TODO remove debug output
            charm_property( p_i_tree, p_tree, test_node, Xi, Xj, min_sup );
        }
        if ( ! ( root->children().empty() ) ) {
            charm_extend( p_i_tree, c_set, min_sup );
        }
        if ( ! is_subsumed( c_set, test_node ) ) {
            std::cerr << "Insert node " << test_node << std::endl; // TODO remvoe debug output
            c_set.insert( CSet::value_type( cset_key_t( test_node.diffset(), test_node.parent()->hashkey() ), cset_val_t( test_node.itemset(), test_node.sup() ) ) );
        }
    }
}

/*!
 * \brief Charm::charm_property
 * \param p_i_tree
 * \param p_tree
 * \param test_node
 * \param Xi
 * \param Xj
 * \param min_sup
 */
void Charm::charm_property(Tree &p_i_tree, Tree &p_tree, const Node &test_node, Node &Xi, Node &Xj, const unsigned int min_sup)
{
    if ( test_node.sup() > min_sup ) {
        if ( Xi.equal( Xj ) ) {
            std::cerr << "prorerty 1\n" << std::endl;
            property_1( p_i_tree, p_tree, Xj, Xi, test_node );
        }
        else {
            if ( Xj.is_superset_of( Xi ) ) {
                std::cerr << "prorerty 2\n" << std::endl;
                property_2( p_tree, p_i_tree, test_node, Xi );
            }
            else {
                if ( Xi.is_superset_of( Xj ) ) {
                    std::cerr << "prorerty 3\n" << std::endl;
                    property_3( p_i_tree, Xj, test_node );
                }
                else {
                    std::cerr << "prorerty 4\n" << std::endl;
                    property_4( p_i_tree, test_node );
                }
            }
        }
    }
    else {
        std::cerr << "rejected\n" << std::endl;
    }
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
    const int hashkey = diffset_hash::hash( std::make_pair( Y, node.parent()->hashkey() ) );
    std::cerr << "hashkey = " << hashkey << std::endl;
    const auto range = c_set.equal_range( std::make_pair( Y, hashkey ) );
    for ( auto it = range.first; it != range.second; ++ it ) {
        const Itemset & C = (*it).second.first;
        const auto sup = (*it).second.second;
        std::cerr << "C = " << C << std::endl;
        std::cerr << "sup = " << sup << std::endl;
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
    Y = result_diffset;
}

/*!
 * \brief Charm::property_1
 * \param p_i_tree
 * \param p_tree
 * \param Xj
 * \param Xi
 * \param test_node
 */
void Charm::property_1(Tree &p_i_tree, Tree &p_tree, Node &Xj, Node &Xi, const Node &test_node)
{
    //    p_tree.remove( Xj.itemset() );
    Xj.set_erased();
    const auto itemset_to_replace = Xi.itemset();
    p_i_tree.replace( itemset_to_replace, test_node.itemset() );
    p_tree.replace( itemset_to_replace, test_node.itemset() );
}

/*!
 * \brief Charm::property_2
 * \param p_tree
 * \param p_i_tree
 * \param test_node
 * \param Xi
 */
void Charm::property_2(Tree &p_tree, Tree &p_i_tree, const Node &test_node, Node &Xi)
{
    const auto itemset_to_replace = Xi.itemset();
    p_i_tree.replace( itemset_to_replace, test_node.itemset() );
    p_tree.replace( itemset_to_replace, test_node.itemset() );
}

/*!
 * \brief Charm::property_3
 * \param p_i_tree
 * \param p_tree
 * \param Xj
 * \param test_node
 */
void Charm::property_3(Tree &p_i_tree, Node &Xj, const Node &test_node)
{
    Xj.set_erased();
    p_i_tree.add( test_node );
}

/*!
 * \brief Charm::property_4
 * \param p_i_tree
 * \param test_node
 */
void Charm::property_4(Tree &p_i_tree, const Node &test_node)
{
    p_i_tree.add( test_node );
}
