#ifndef TREE_HPP
#define TREE_HPP

#include "Typedefs.hpp"

#include <memory>
#include <cassert>

#include "Node.hpp"

/*!
 * \brief The Tree class
 */
class Tree
{
public:
    Tree();
    void add(const Itemset &itemset, const Diffset & diffset , const unsigned int sup, const unsigned int hash);
    void add(const Node & node);
    std::weak_ptr < Node > root_node();
    void print_tree() const;
    void replace(const Itemset & itemset, const Itemset & itemset_to);

private:
    static void replace_item(Node &node_ref, const Itemset &itemset, const Itemset &itemset_to);
    static void print_node(const std::weak_ptr<Node> &node_ptr);

private:
    std::shared_ptr < Node > _root_node;
};


#endif // TREE_HPP
