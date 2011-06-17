#ifndef FUTILE_NNODE_H_INCLUDED
#define FUTILE_NNODE_H_INCLUDED

namespace Futile {


template<typename T, unsigned N, unsigned H, unsigned D>
struct NNode;


/**
 * Partial specialization for D == 0 represents the root node.
 * The root node has no parent node.
 */
template<typename T, unsigned N, unsigned H>
struct NNode<T, N, H, 0>
{
    NNode() :
        mData(),
        mChildren()
    {
    }

    T mData;

    typedef NNode<T, N, H, 1> Child;
    Child mChildren[N];
};


/**
 * Partial specialization for H == D represents the leaf node.
 * Leaf nodes have no children.
 */
template<typename T, unsigned N, unsigned H>
struct NNode<T, N, H, H>
{
    NNode() :
        mParent(NULL),
        mData()
    {
    }

    typedef NNode<T, N, H, H - 1> Parent;
    Parent * mParent;

    T mData;
};


/**
 * NNode represents a node in an n-ary tree.
 * T = data type
 * N = number of children per node
 * H = tree height
 * D = node depth (distance from the root node)
 *
 * The entire tree is contained in a single segement of contiguous memory.
 */
template<typename T, unsigned N, unsigned H, unsigned D>
struct NNode
{
    NNode() :
        mParent(NULL),
        mData(),
        mChildren()
    {
    }


    typedef NNode<T, N, H, D - 1> Parent;
    Parent * mParent;

    T mData;

    typedef NNode<T, N, H, D + 1> Child;
    Child mChildren[N];
};


} // namespace Futile


#endif // FUTILE_NNODE_H_INCLUDED
