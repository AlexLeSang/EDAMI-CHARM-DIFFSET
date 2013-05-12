#ifndef NODE_HPP
#define NODE_HPP

#include "Typedefs.hpp"

#include <memory>

class Node
{
public:
    Node();
    Node(const Itemset & itemset, const Tidset & tidset);
    Node(const Itemset & itemset, const Tidset & tidset, Node * parent_ptr);

    void add_child(const Node & node_ref );
    const std::vector< std::shared_ptr < Node > > & children() const;
    std::vector< std::shared_ptr < Node > > & children_ref();

    Node * parent() const;
    void set_parent(Node *parent_ptr );

    const Itemset & itemset() const;
    Itemset & itemset();
    const Tidset & tidset() const;

    Node( const Node & r_node );
    Node & operator = ( const Node & r_node );

private:
    Itemset _itemset;
    Tidset _tidset;

    Node * _parent;
    std::vector < std::shared_ptr < Node > > _children;
};

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

    os << " Tidset: ";
    index = node.tidset().size();
    if ( index ) {
        os << '<';
        std::for_each( node.tidset().cbegin(), node.tidset().cend(), [&]( const TID & tid ) {
            os << tid << ( --index ? ',' : '>' );
        } );
    }
    return os;
}

#endif // NODE_HPP
