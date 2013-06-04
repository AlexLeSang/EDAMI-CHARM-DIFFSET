#ifndef NODE_HPP
#define NODE_HPP

#include "Itemset.hpp"
#include "Diffset.hpp"

#include <memory>

#include <mutex>

/*!
 * \brief The Node class
 */
class Node
{
public:
    Node() :
        _itemset( Itemset() ),
        _diffset( Diffset() ),
        _parent( nullptr ),
        _is_erased( false ),
        _sup( 0 ),
        _hash_key_setted( false ),
        _hashkey( 0 ) {}
    Node(Itemset && rv_itemset, Diffset && rv_diffset, Node * parent_ptr) :
        _itemset( std::move(rv_itemset) ),
        _diffset( std::move(rv_diffset) ),
        _parent( parent_ptr ),
        _is_erased( false ),
        _sup( 0 ),
        _hash_key_setted( false ),
        _hashkey( 0 ) {
        calculate_support();
        calculate_hashkey();
    }
    Node(const Itemset & itemset, const Diffset & diffset, Node * parent_ptr) :
        _itemset( itemset ),
        _diffset( diffset ),
        _parent( parent_ptr ),
        _is_erased( false ),
        _sup( 0 ),
        _hash_key_setted( false ),
        _hashkey( 0 ) {
        calculate_support();
        calculate_hashkey();
    }
    Node(const Itemset & itemset, const Diffset & diffset, const unsigned int sup, const unsigned int hash) :
        _itemset( itemset ),
        _diffset( diffset ),
        _parent( nullptr ),
        _is_erased( false ),
        _sup( sup ),
        _hash_key_setted( true ),
        _hashkey( hash ) {
    }
    Node(const Node & r_node) :
        _itemset( r_node.itemset() ),
        _diffset( r_node.diffset() ),
        _parent( r_node.parent() ),
        _children( r_node.children() ),
        _is_erased( r_node.is_erased() ),
        _sup( r_node.sup() ),
        _hash_key_setted( true ),
        _hashkey( r_node.hashkey() ) {}
    Node(Node && m_node) :
        _itemset( std::move( m_node.itemset() ) ),
        _diffset( std::move( m_node.diffset() ) ),
        _parent( m_node.parent() ),
        _children( std::move( m_node.children() ) ),
        _is_erased( m_node.is_erased() ),
        _sup( m_node.sup() ),
        _hash_key_setted( true ),
        _hashkey( m_node.hashkey() ) {
    }

    inline Node & operator = ( const Node & r_node ) {
        if ( this != &r_node ) {
            _itemset = r_node.itemset();
            _diffset = r_node.diffset();
            _children = r_node.children();
            _parent = r_node.parent();
            _is_erased = r_node.is_erased();
            _sup = r_node.sup();
            _hash_key_setted = true;
            _hashkey = r_node.hashkey();
        }
        return *this;
    }

    inline void add_child(const Node & node_ref ) {
        std::shared_ptr< Node > node( new Node( node_ref ) );
        node->set_parent( this );
        _children.push_back( node );
        std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
            return ( ch1->sup() < ch2->sup() ); // Sup
        } );
    }
    inline void add_child(Itemset && itemset, Diffset && diffset) {

//        static std::mutex m;
//        std::lock_guard< std::mutex > l( m );


        std::shared_ptr< Node > node( new Node( itemset, diffset, this ) );
        _children.push_back( node );
        std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
            return ( ch1->sup() < ch2->sup() ); // Sup
        } );
    }
    inline const std::vector< std::shared_ptr < Node > > & children() const  {
        return _children;
    }
    inline std::vector< std::shared_ptr < Node > > & children_ref() {
        return _children;
    }

    inline Node * parent() const {
        return _parent;
    }
    inline void set_parent(Node *parent_ptr ) {
        //    assert( nullptr != parent_ptr );
        _parent = parent_ptr;
    }

    inline const Itemset & itemset() const {
        return _itemset;
    }
    inline Itemset & itemset() {
        return _itemset;
    }

    inline bool is_erased() const {
        return _is_erased;
    }
    inline void set_erased() {
        _is_erased = true;
    }

    unsigned int sup() const {
        return _sup;
    }

    inline bool equal( const Node & r_node) const {
        const auto r_mist = r_node.mistakes( _diffset );
        const auto mist = mistakes( r_node.diffset() );
        if (  (r_mist == mist) && ( mist == 0 ) ) {
            return true;
        }
        else {
            return false;
        }
    }

    inline bool is_superset_of(const Node r_node) const {
        const auto r_mist = r_node.mistakes( _diffset );
        const auto mist = mistakes( r_node.diffset() );

        if ( ( mist > 0 ) && ( r_mist == 0 ) ) {
            // is superset
            return true;
        }
        else {
            // isn't superset
            return false;
        }
    }

    inline const Diffset & diffset() const {
        return _diffset;
    }

    inline unsigned int mistakes(const Diffset & other) const {
        /*
        unsigned int mistake_counter = 0;
        for ( const auto & tid : other ) {
            if ( _diffset.cend() == std::find( _diffset.cbegin(), _diffset.cend(), tid ) ) {
                mistake_counter ++;
            }
        }
        */
        unsigned int mistake_counter = 0;
        for ( const auto & tid : other ) {
            if ( !std::binary_search( _diffset.cbegin(), _diffset.cend(), tid ) ) {
                mistake_counter ++;
            }
        }


        return mistake_counter;
    }

    int hashkey() const {
        //        assert( _hash_key_setted );
        return _hashkey;
    }
    void setHashkey(const int hashkey) {
        //        assert( ! _hash_key_setted );
        _hashkey = hashkey;
        _hash_key_setted = true;
    }

private:
    inline void calculate_support() {
        //        assert( _sup == 0 ); // A default value of the supp, support recalculation is allowed only once
        //        assert( _parent != nullptr ); // Parent should be avaible
        _sup = _parent->sup() - _diffset.size();
        //    assert( _sup >= 0 ); // Support must be positive
    }
    inline void calculate_hashkey() {
        //        assert( _parent != nullptr );
        //        assert( !_hash_key_setted );
        _hashkey = diffset_hash::hash( std::make_pair( _diffset, _parent->hashkey() ) );
        _hash_key_setted = true;
    }

private:
    Itemset _itemset;
    Diffset _diffset;
    Node * _parent;
    std::vector < std::shared_ptr < Node > > _children;
    bool _is_erased;
    unsigned int _sup;

    bool _hash_key_setted;
    int _hashkey;
};

/*!
 * \brief operator <<
 * \param os
 * \param node
 * \return
 */
inline std::ostream & operator << ( std::ostream & os, const Node & node )
{
    os << "Node: ";
    os << "Itemset: ";

    unsigned int index = node.itemset().size();
    if ( index ) {
        os << '(';
        std::for_each( node.itemset().cbegin(), node.itemset().cend(), [&]( const Item & item ) {
            os << item << ( --index ? ' ' : ')' );
        } );
    }

    os << " Diffset: ";
    index = node.diffset().size();
    if ( index ) {
        os << '<';
        std::for_each( node.diffset().cbegin(), node.diffset().cend(), [&]( const TID & tid ) {
            os << tid << ( --index ? ',' : '>' );
        } );
    }

    if (  node.itemset().size() ) {
        os << " Sup: " << node.sup();
        os << " Hashkey: " << node.hashkey();
    }
    return os;
}

#endif // NODE_HPP
