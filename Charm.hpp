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
    static void charm_extend(Node &p_tree, CSet &c_set, const unsigned int min_sup);
//    static void charm_property(Tree & p_i_tree, Tree & p_tree, const Node &test_node, Node & Xi, Node & Xj, const unsigned int min_sup);
public:
    static bool is_subsumed(const CSet &c_set, const Node &node);
private:
    static void itemset_union(Itemset & X, const Node &Xj);
    static void diffset_difference(Diffset &Y, const Node & Xj);

    static void replace_item(Node& node_ref, const Itemset &itemset, const Itemset &itemset_to);
};

#endif // CHARM_HPP
