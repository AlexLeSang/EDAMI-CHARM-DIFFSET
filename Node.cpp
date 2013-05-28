#include "Node.hpp"

#include <cassert>

#include "Typedefs.hpp"

/*!
 * \brief Node::Node
 */
Node::Node() :
    _itemset( Itemset() ),
    _diffset( Diffset() ),
    _parent( nullptr ),
    _is_erased( false ),
    _sup( 0 ),
    _hash_key_setted( false ),
    _hashkey( 0 )
{}

/*!
 * \brief Node::Node
 * \param itemset
 * \param diffset
 * \param sup
 */
Node::Node(const Itemset &itemset, const Diffset &diffset, const unsigned int sup, const unsigned int hash) :
    _itemset( itemset ),
    _diffset( diffset ),
    _parent( nullptr ),
    _is_erased( false ),
    _sup( sup ),
    _hash_key_setted( true ),
    _hashkey( hash )
{
}

/*!
 * \brief Node::Node
 * \param itemset
 * \param tidset
 */
Node::Node(const Itemset &itemset, const Diffset &diffset, Node *parent_ptr) :
    _itemset( itemset ),
    _diffset( diffset ),
    _parent( parent_ptr ),
    _is_erased( false ),
    _sup( 0 ),
    _hash_key_setted( false ),
    _hashkey( 0 )
{
    calculate_support();
    calculate_hashkey();
}

/*!
 * \brief Node::Node
 * \param r_node
 */
Node::Node(const Node &r_node) :
    _itemset( r_node.itemset() ),
    _diffset( r_node.diffset() ),
    _parent( r_node.parent() ),
    _children( r_node.children() ),
    _is_erased( r_node.is_erased() ),
    _sup( r_node.sup() ),
    _hash_key_setted( true ),
    _hashkey( r_node.hashkey() )
{}

/*!
 * \brief Node::Node
 * \param m_node
 */
Node::Node(Node && m_node) :
    _itemset( std::move( m_node.itemset() ) ),
    _diffset( std::move( m_node.diffset() ) ),
    _parent( m_node.parent() ),
    _children( std::move( m_node.children() ) ),
    _is_erased( m_node.is_erased() ),
    _sup( m_node.sup() )
{
    std::cerr << "O_o, the move constructor had been called" << std::endl;
}

/*!
 * \brief Node::operator =
 * \param r_node
 * \return
 */
Node &Node::operator =(const Node &r_node)
{
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

/*!
 * \brief Node::is_erased
 * \return
 */
bool Node::is_erased() const
{
    return _is_erased;
}

/*!
 * \brief Node::set_erased
 */
void Node::set_erased()
{
    _is_erased = true;
}

/*!
 * \brief Node::equal
 * \param r_node
 * \return
 */
bool Node::equal(const Node &r_node) const
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
 * \brief Node::is_subset
 * \param r_node
 * \return
 */
bool Node::is_superset_of( const Node r_node ) const
{
    const auto r_mist = r_node.mistakes( _diffset );
    const auto mist = mistakes( r_node.diffset() );

    if ( (mist > 0) && ( r_mist == 0 ) ) {
        // is superset
        return true;
    }
    else {
        // isn't superset
        return false;
    }
}

/*!
 * \brief Node::sup
 * \return
 */
unsigned int Node::sup() const
{
//    assert( _sup != 0 );
    return _sup;
}

/*!
 * \brief Node::add_child
 * \param node_ref
 */
void Node::add_child(const Node &node_ref)
{
    std::shared_ptr< Node > node( new Node( node_ref ) );
    node->set_parent( this );
    _children.push_back( node );
    /* Lexicographic orded
    std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
        return ( sup( ch1 ) < sup( ch2 ) );
    } );
    */
    //    /* Increasing order of sup
    std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
        return ( ch1->sup() < ch2->sup() ); // Sup
    } );
    //    */
    /* Increasing order of sup and lexicograph
    std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
        if ( sup( ch1 ) != sup( ch2 ) ) {
            return ( sup( ch1 ) < sup( ch2 ) ); // Sup
        }
        else {
            return ( ch1->itemset() < ch2->itemset() ); // Lexicograph
        }
    } );
    */
}

/*!
 * \brief Node::parent
 * \return
 */
Node *Node::parent() const
{
    return _parent;
}

/*!
 * \brief Node::set_parent
 * \param parent_ptr
 */
void Node::set_parent(Node * parent_ptr)
{
    assert( nullptr != parent_ptr );
    _parent = parent_ptr;
}

/*!
 * \brief Node::children
 * \return
 */
const std::vector<std::shared_ptr<Node> > &Node::children() const
{
    return _children;
}

/*!
 * \brief Node::children_ref
 * \return
 */
std::vector<std::shared_ptr<Node> > &Node::children_ref()
{
    return _children;
}

/*!
 * \brief Node::itemset
 * \return
 */
const Itemset &Node::itemset() const
{
    return _itemset;
}

/*!
 * \brief Node::itemset
 * \return
 */
Itemset &Node::itemset()
{
    return _itemset;
}

/*!
 * \brief Node::diffset
 * \return
 */
const Diffset &Node::diffset() const
{
    return _diffset;
}

/*!
 * \brief Node::calculate_support
 */
void Node::calculate_support()
{
    assert( _sup == 0 ); // A default value of the supp, support recalculation is allowed only once
    assert( _parent != nullptr ); // Parent should be avaible
    _sup = _parent->sup() - _diffset.size();
//    assert( _sup >= 0 ); // Support must be positive
}

/*!
 * \brief Node::calculate_hashkey
 */
void Node::calculate_hashkey()
{
    assert( _parent != nullptr );
    assert( !_hash_key_setted );
    _hashkey = diffset_hash::hash( std::make_pair( _diffset, _parent->hashkey() ) );
    _hash_key_setted = true;
}

/*!
 * \brief Node::mistakes A number of tid which appears at other Diffset but doesn't appear in current
 * \param other
 */
unsigned int Node::mistakes(const Diffset &other) const
{
    unsigned int mistake_counter = 0;
    for ( const auto & tid : other ) {
        if ( _diffset.cend() == std::find( _diffset.cbegin(), _diffset.cend(), tid ) ) {
            mistake_counter ++;
        }
    }
    return mistake_counter;
}

/*!
 * \brief Node::hashkey
 * \return
 */
int Node::hashkey() const
{
    assert( _hash_key_setted );
    return _hashkey;
}

/*!
 * \brief Node::setHashkey
 * \param hashkey
 */
void Node::setHashkey(const int hashkey)
{
    assert( ! _hash_key_setted );
    _hashkey = hashkey;
    _hash_key_setted = true;
}
