#ifndef NODE_HPP
#define NODE_HPP

#include "Itemset.hpp"
#include "Diffset.hpp"

#include <memory>

/*!
 * \brief The Node class
 */
class Node
{
public:
    Node();
    Node(const Itemset & itemset, const Diffset & diffset, const unsigned int sup, const unsigned int hash);
    Node(const Itemset & itemset, const Diffset & diffset, Node * parent_ptr);
    Node(const Node & r_node);
    Node(Node && m_node);

    Node & operator = ( const Node & r_node );

    void add_child(const Node & node_ref );
    const std::vector< std::shared_ptr < Node > > & children() const;
    std::vector< std::shared_ptr < Node > > & children_ref();

    Node * parent() const;
    void set_parent(Node *parent_ptr );

    const Itemset & itemset() const;
    Itemset & itemset();

    bool is_erased() const;
    void set_erased();

    unsigned int sup() const;

    bool equal( const Node & r_node) const;
    bool is_superset_of(const Node r_node) const;

    const Diffset & diffset() const;
    unsigned int mistakes(const Diffset & other) const;

    int hashkey() const;
    void setHashkey(const int hashkey);

private:
    void calculate_support();
    void calculate_hashkey();

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
