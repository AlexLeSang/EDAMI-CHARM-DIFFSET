#ifndef CHARM_HPP
#define CHARM_HPP

#include "CSet.hpp"

#include "Database.hpp"

#include "Node.hpp"

#include <iterator>

#include <thread>
#include <mutex>
#include <future>
#include <list>


/*!
 * \brief ItemMap
 */
typedef std::unordered_map< Item, Diffset, item_hash > ItemMap;


/*!
 * \brief The Charm class
 */
class Charm
{
public:
    /*!
     * \brief charm
     * \param database
     * \param min_sup
     * \return
     */
    static inline CSet charm( const Database & database, const unsigned int min_sup )
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
//        std::cerr << "transaction_counter = " << transaction_counter << std::endl;

        // Translate tidset into diffset
        std::for_each( item_map.begin(), item_map.end(), [&]( ItemMap::reference key_value ) {
            const auto tidset = std::move( key_value.second );
            // Transfort to a diffset
            for ( auto index = 1; index <= transaction_counter; ++ index ) {
                if ( tidset.cend() == std::find( tidset.cbegin(), tidset.cend(), index ) ) {
                    key_value.second.push_back( index );
                }
            }
        } );

        // Fill the tree
        const int sum_of_trans_id = transaction_counter * (transaction_counter - 1) / 2;
        Node root_node( Itemset(), Diffset(), transaction_counter, sum_of_trans_id );
        //    Tree p( root_node );
        {
            std::for_each( item_map.cbegin(), item_map.cend(), [&]( ItemMap::const_reference key_value ) {
                if ( min_sup <= (transaction_counter - key_value.second.size()) ) {
                    Itemset itemset;
                    itemset.push_back( key_value.first );
                    Diffset diffset = key_value.second;
                    root_node.add_child( std::move( itemset ), std::move( diffset ) );
                }
            } );
        }
        auto c_set = CSet();
        charm_extend( root_node, c_set, min_sup );
        return c_set;
    }

private:
    /*!
     * \brief charm_extend
     * \param p_tree
     * \param c_set
     * \param min_sup
     */
    static inline void charm_extend(Node &p_tree, CSet &c_set, const unsigned int min_sup)
    {
        static unsigned int recursion_level = 0;
        ++ recursion_level;
        for ( auto it = p_tree.children().cbegin(); it != p_tree.children().cend(); ++ it ) {
            Node & current_child = (*(*it));
            if ( current_child.is_erased() ) continue;
            for ( auto int_it = it + 1; int_it != p_tree.children().cend(); ++ int_it ) {
                Node & internal_child = (*(*int_it));
                if ( internal_child.is_erased() ) continue;
                Charm_internal( min_sup, current_child, internal_child );
            }
            charm_extend( current_child, c_set, min_sup );
        }
        if ( ! p_tree.itemset().empty() ) {
            check_subsumption_and_insert( c_set, p_tree );
        }
        -- recursion_level;
    }

    /*!
     * \brief charm_extend_c
     * \param p_tree
     * \param c_set
     * \param min_sup
     */
    inline static void charm_extend_c(Node p_tree, CSet &c_set, const unsigned int min_sup)
    {
        Charm::charm_extend( p_tree, c_set, min_sup );
    }

    /*!
     * \brief check_subsumption_and_insert
     * \param c_set
     * \param node
     */
    inline static void check_subsumption_and_insert(CSet &c_set, const Node &node)
    {
        const Itemset & X = node.itemset();
        const Diffset & Y = node.diffset();
        bool is_subsumed = false;
        const int hashkey = diffset_hash::hash( std::make_pair( Y, node.parent()->hashkey() ) );
        const auto range = c_set.equal_range( std::make_pair( Y, hashkey ) );
        for ( auto it = range.first; it != range.second; ++ it ) {
            const Itemset & C = (*it).second.first;
            const auto sup = (*it).second.second;
            if ( node.sup() == sup ) {
                const bool includes = std::includes( C.cbegin(), C.cend(), X.cbegin(), X.cend() );
                if ( includes ) {
                    is_subsumed = true;
                    break;
                }
            }
        }
        if ( ! is_subsumed ) {
            c_set.insert( CSet::value_type( cset_key_t( node.diffset(), node.parent()->hashkey() ), cset_val_t( node.itemset(), node.sup() ) ) );
        }
    }

    /*!
     * \brief itemset_union
     * \param X
     * \param Xj
     */
    inline static void itemset_union(Itemset & X, const Node &Xj)
    {
        Itemset union_itemset( X.size() + Xj.itemset().size() );
        auto it_union = std::set_union( X.cbegin(), X.cend(), Xj.itemset().cbegin(), Xj.itemset().cend(), union_itemset.begin() );
        union_itemset.resize( it_union - union_itemset.begin() );
        X = std::move(union_itemset);
    }

    /*!
     * \brief diffset_difference
     * \param Y
     * \param Xj
     */
    inline static void diffset_difference(Diffset &Y, const Node & Xj)
    {
        Diffset result_diffset( Y.size() + Xj.diffset().size() );
        auto it = std::set_difference( Xj.diffset().cbegin(), Xj.diffset().cend(), Y.cbegin(), Y.cend(), result_diffset.begin() );
        result_diffset.resize( it - result_diffset.begin() );
        Y = std::move(result_diffset);
    }

    /*!
     * \brief replace_item
     * \param node_ref
     * \param itemset
     * \param itemset_to
     */
    inline static void replace_item(Node & node_ref, const Itemset &itemset, const Itemset &itemset_to)
    {
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
     * \brief Charm_internal
     * \param min_sup
     * \param current_child
     * \param internal_child
     */
    inline static void Charm_internal(const unsigned int min_sup, Node &current_child, Node &internal_child)
    {
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
    }
};

#endif // CHARM_HPP
