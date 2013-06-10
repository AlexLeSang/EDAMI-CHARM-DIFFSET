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
    /*!
     * \brief Node
     */
    Node() :
        _itemset( Itemset() ),
        _diffset( Diffset() ),
        _parent( nullptr ),
        _is_erased( false ),
        _sup( 0 ),
        _hash_key_setted( false ),
        _hashkey( 0 ) {}

    /*!
     * \brief Node
     * \param rv_itemset
     * \param rv_diffset
     * \param parent_ptr
     */
    Node(Itemset && rv_itemset, Diffset && rv_diffset, const Node * parent_ptr) :
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

    /*!
     * \brief Node
     * \param itemset
     * \param diffset
     * \param parent_ptr
     */
    Node(const Itemset & itemset, const Diffset & diffset, const Node * parent_ptr) :
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

    /*!
     * \brief Node
     * \param itemset
     * \param diffset
     * \param sup
     * \param hash
     */
    Node(const Itemset & itemset, const Diffset & diffset, const unsigned int sup, const int hash) :
        _itemset( itemset ),
        _diffset( diffset ),
        _parent( nullptr ),
        _is_erased( false ),
        _sup( sup ),
        _hash_key_setted( true ),
        _hashkey( hash ) {
    }

    /*!
     * \brief Node
     * \param r_node
     */
    Node(const Node & r_node) :
        _itemset( r_node.itemset() ),
        _diffset( r_node.diffset() ),
        _parent( r_node.parent() ),
        _children( r_node.children() ),
        _is_erased( r_node.is_erased() ),
        _sup( r_node.sup() ),
        _hash_key_setted( true ),
        _hashkey( r_node.hashkey() ) {}

    /*!
     * \brief Node
     * \param m_node
     */
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

    /*!
     * \brief operator =
     * \param r_node
     * \return
     */
    Node & operator = ( const Node & r_node ) = delete;

    /*!
     * \brief add_child
     * \param node_ref
     */
    inline void add_child(const Node & node_ref )
    {
        std::shared_ptr< Node > node( new Node( node_ref ) );
        node->set_parent( this );
        _children.push_back( node );
        std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
            return ( ch1->sup() < ch2->sup() ); // Sup
        } );
    }

    /*!
     * \brief add_child
     * \param itemset
     * \param diffset
     */
    inline void add_child(Itemset && itemset, Diffset && diffset)
    {
        std::shared_ptr< Node > node( new Node( itemset, diffset, this ) );
        _children.push_back( node );
        std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
            return ( ch1->sup() < ch2->sup() ); // Sup
        } );

    }

    /*!
     * \brief children
     * \return
     */
    inline const std::vector< std::shared_ptr < Node > > & children() const
    {
        return _children;
    }

    /*!
     * \brief children_ref
     * \return
     */
    inline std::vector< std::shared_ptr < Node > > & children_ref()
    {
        return _children;
    }

    /*!
     * \brief parent
     * \return
     */
    inline const Node * parent() const
    {
        return _parent;
    }

    /*!
     * \brief set_parent
     * \param parent_ptr
     */
    inline void set_parent(const Node *parent_ptr)
    {
        //    assert( nullptr != parent_ptr );
        _parent = parent_ptr;
    }

    /*!
     * \brief itemset
     * \return
     */
    inline const Itemset & itemset() const
    {
        return _itemset;
    }

    /*!
     * \brief itemset
     * \return
     */
    inline Itemset & itemset()
    {
        return _itemset;
    }

    /*!
     * \brief is_erased
     * \return
     */
    inline bool is_erased() const
    {
        return _is_erased;
    }

    /*!
     * \brief set_erased
     */
    inline void set_erased()
    {
        _is_erased = true;
    }

    /*!
     * \brief sup
     * \return
     */
    unsigned int sup() const
    {
        return _sup;
    }

    /*!
     * \brief equal
     * \param r_node
     * \return
     */
    inline bool equal(const Node & r_node) const
    {
        const auto r_mist = r_node.mistakes( _diffset );
        const auto mist = mistakes( r_node.diffset() );
        if (  (r_mist == mist) && ( mist == 0 ) ) {
            return true;
        }
        else {
            return false;
        }
    }

    /*!
     * \brief is_superset_of
     * \param r_node
     * \return
     */
    inline bool is_superset_of(const Node r_node) const {
        const auto r_mist = r_node.mistakes( _diffset );
        const auto mist = mistakes( r_node.diffset() );
        return ( ( mist > 0 ) && ( r_mist == 0 ) );
    }

    /*!
     * \brief diffset
     * \return
     */
    inline const Diffset & diffset() const
    {
        return _diffset;
    }

    /*!
     * \brief mistakes
     * \param other
     * \return
     */
    inline unsigned int mistakes(const Diffset & other) const
    {
        unsigned int mistake_counter = 0;
        for ( const auto & tid : other ) {
            if ( !std::binary_search( _diffset.cbegin(), _diffset.cend(), tid ) ) {
                mistake_counter ++;
            }
        }
        return mistake_counter;
    }

    /*!
     * \brief hashkey
     * \return
     */
    inline int hashkey() const
    {
        return _hashkey;
    }

    /*!
     * \brief setHashkey
     * \param hashkey
     */
    inline void setHashkey(const int hashkey)
    {
        _hashkey = hashkey;
        _hash_key_setted = true;
    }

private:
    /*!
     * \brief calculate_support
     */
    inline void calculate_support()
    {
        _sup = _parent->sup() - _diffset.size();
    }

    /*!
     * \brief calculate_hashkey
     */
    inline void calculate_hashkey() {
        _hashkey = diffset_hash::hash( std::make_pair( _diffset, _parent->hashkey() ) );
        _hash_key_setted = true;
    }

private:
    Itemset _itemset;
    const Diffset _diffset;
    const Node * _parent;
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
