#ifndef CHARM_HPP
#define CHARM_HPP

#include "CSet.hpp"

#include "Database.hpp"

#include "Node.hpp"

/*!
 * \brief The Charm class
 */
class Charm
{
public:
    static CSet charm( const Database & database, const unsigned int min_sup );

private:
    inline static void charm_extend(Node &p_tree, CSet &c_set, const unsigned int min_sup) {
        static unsigned int recursion_level = 0;
        ++ recursion_level;

        //    std::cerr << "\nrecursion_level = " << recursion_level << std::endl;

        //    std::cerr << "p_tree: " << p_tree << std::endl;
        //    std::for_each( p_tree.children().cbegin(), p_tree.children().cend(), []( const std::shared_ptr<Node> & val_ref ) { std::cerr << (*val_ref) << std::endl; } );

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


                //            auto union_future = std::async( std::launch::async, itemset_union, std::ref(X), std::cref(internal_child) );
                //            diffset_difference( Y, internal_child );
                //            union_future.get();
                //            const Node test_node( X, Y, &current_child );
                //            const Node test_node( std::move(X), std::move(Y), &current_child );
                //            std::cerr << "  test_node = " << test_node << std::endl;
                const unsigned int sup = current_child.sup() - Y.size();
                if ( sup >= min_sup ) {
                    if ( current_child.equal( internal_child ) ) {
                        //                    std::cerr << "   property_1" << std::endl;
                        //                    std::cerr << "Erase " << internal_child << std::endl;
                        internal_child.set_erased();
                        const Itemset replased_itemset = current_child.itemset();
                        //                    replace_item( current_child, replased_itemset, test_node.itemset() );
                        replace_item( current_child, replased_itemset, X );
                    }
                    else {
                        if ( internal_child.is_superset_of( current_child ) ) {
                            //                        std::cerr << "   property_2" << std::endl;
                            const Itemset replased_itemset = current_child.itemset();
                            //                        replace_item( current_child, replased_itemset, test_node.itemset() );
                            replace_item( current_child, replased_itemset, X );
                        }
                        else {
                            if ( current_child.is_superset_of( internal_child ) ) {
                                //                            std::cerr << "   property_3" << std::endl;
                                //                            std::cerr << "Erase " << internal_child << std::endl;
                                internal_child.set_erased();
                                //                            current_child.add_child( test_node );
                                current_child.add_child( std::move(X), std::move(Y) );
                            }
                            else {
                                //                            std::cerr << "   property_4" << std::endl;
                                //                            current_child.add_child( test_node );
                                current_child.add_child( std::move(X), std::move(Y) );
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
            check_subsumption_and_insert( c_set, p_tree );
        }

        -- recursion_level;
    }
    inline static void charm_extend_c(Node p_tree, CSet &c_set, const unsigned int min_sup) {
        Charm::charm_extend( p_tree, c_set, min_sup );
    }
    inline static void check_subsumption_and_insert(CSet &c_set, const Node &node) {
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
        if ( ! is_subsumed ) {
            c_set.insert( CSet::value_type( cset_key_t( node.diffset(), node.parent()->hashkey() ), cset_val_t( node.itemset(), node.sup() ) ) );
        }

    }
    inline static void itemset_union(Itemset & X, const Node &Xj) {
        Itemset union_itemset( X.size() + Xj.itemset().size() );
        auto it_union = std::set_union( X.cbegin(), X.cend(), Xj.itemset().cbegin(), Xj.itemset().cend(), union_itemset.begin() );
        union_itemset.resize( it_union - union_itemset.begin() );
        X = std::move(union_itemset);
    }
    inline static void diffset_difference(Diffset &Y, const Node & Xj) {
        Diffset result_diffset( Y.size() + Xj.diffset().size() );
        //        auto it = std::set_difference( Y.cbegin(), Y.cend(), Xj.diffset().cbegin(), Xj.diffset().cend(), result_diffset.begin() );
        auto it = std::set_difference( Xj.diffset().cbegin(), Xj.diffset().cend(), Y.cbegin(), Y.cend(), result_diffset.begin() );
        result_diffset.resize( it - result_diffset.begin() );
        Y = result_diffset;
    }

    inline static void replace_item(Node & node_ref, const Itemset &itemset, const Itemset &itemset_to) {
        //    assert( itemset.size() != 0 );
        //    assert( itemset_to.size() != 0 );
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
};

#endif // CHARM_HPP
