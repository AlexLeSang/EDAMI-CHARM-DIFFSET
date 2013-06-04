#ifndef CHARM_HPP
#define CHARM_HPP

#include "CSet.hpp"

#include "Database.hpp"

#include "Node.hpp"

#include <future>



template< typename T >
inline void replace_itemset( T & where, const T & what, const T & by_what )
{
    if ( where.size() < what.size() ) return;


    if ( what.size() == by_what.size() ) {
        auto it = by_what.cbegin();
        std::for_each( what.cbegin(), what.cend(), [&]( typename T::const_reference c_val ) {
                       std::replace( where.begin(), where.end(), c_val, (*it) );
                ++ it;
    } );
    std::sort( where.begin(), where.end() );
    return;
}

if ( what.size() < by_what.size() ) {
    auto it = by_what.cbegin();
    std::for_each( what.cbegin(), what.cend(), [&]( typename T::const_reference c_val ) {
                   std::replace( where.begin(), where.end(), c_val, (*it) );
            ++ it;
} );
const unsigned int dist = by_what.size() - what.size();

const unsigned int old_size = where.size();
where.resize( old_size + dist );
std::copy( by_what.cbegin() + dist, by_what.cend(), where.begin() + old_size );

std::sort( where.begin(), where.end() );
return;
}


if ( what.size() > by_what.size() ) {



    std::sort( where.begin(), where.end() );
    return;
}



}



/*!
 * \brief The Charm class
 */
class Charm
{
public:
    static CSet charm( const Database & database, const unsigned int min_sup );

private:
    static void charm_extend(Node &p_tree, CSet &c_set, const unsigned int min_sup);

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
        auto it = std::set_difference( Xj.diffset().cbegin(), Xj.diffset().cend(), Y.cbegin(), Y.cend(), result_diffset.begin() );
        result_diffset.resize( it - result_diffset.begin() );
        Y = std::move(result_diffset);
    }

    inline static void replace_item(Node & node_ref, const Itemset &itemset, const Itemset &itemset_to) {
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
