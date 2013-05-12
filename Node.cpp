#include "Node.hpp"

Node::Node() :
    _itemset( Itemset() ),
    _tidset( Tidset() )
{

}

Node::Node(const Itemset &itemset, const Tidset &tidset) :
    _itemset( itemset ),
    _tidset( tidset )
{

}

Node::Node(const Itemset &itemset, const Tidset &tidset, Node * parent_ptr) :
    _itemset( itemset ),
    _tidset( tidset ),
    _parent( parent_ptr )
{

}

Node::Node(const Node &r_node) :
    _itemset( r_node.itemset() ),
    _tidset( r_node.tidset() ),
    _parent( r_node.parent() ),
    _children( r_node.children() )
{

}

Node &Node::operator =(const Node &r_node)
{
    if ( this != &r_node ) {
        _itemset = r_node.itemset();
        _tidset = r_node.tidset();
        _children = r_node.children();
    }
    return *this;
}

void Node::add_child(const Node &node_ref)
{
    std::shared_ptr< Node > node( new Node( node_ref ) );

    node->set_parent( this );
    _children.push_back( node );

    /* Lexicographic orded
    std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
        return ( ch1->itemset() < ch2->itemset() );
    } );
    */

    //    /* Increasing order of sup
    std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
        return ( ch1->tidset().size() < ch2->tidset().size() ); // Sup
    } );
    //    */

    /* Increasing order of sup and lexicograph
    std::sort( _children.begin(), _children.end(), [] ( std::shared_ptr< Node > ch1, std::shared_ptr< Node > ch2 ) {
        if ( ch1->tidset().size() != ch2->tidset().size() ) {
            return ( ch1->tidset().size() < ch2->tidset().size() ); // Sup
        }
        else {
            return ( ch1->itemset() < ch2->itemset() ); // Lexicograph
        }
    } );
    */

}

Node *Node::parent() const
{
    return _parent;
}

void Node::set_parent(Node * parent_ptr)
{
    assert( nullptr != parent_ptr );
    _parent = parent_ptr;
}

const std::vector<std::shared_ptr<Node> > &Node::children() const
{
    return _children;
}

std::vector<std::shared_ptr<Node> > &Node::children_ref()
{
    return _children;
}

const Itemset &Node::itemset() const
{
    return _itemset;
}

Itemset &Node::itemset()
{
    return _itemset;
}

const Tidset &Node::tidset() const
{
    return _tidset;
}
