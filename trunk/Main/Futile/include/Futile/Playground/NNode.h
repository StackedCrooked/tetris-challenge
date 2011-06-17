#ifndef FUTILE_NNODE_H_INCLUDED
#define FUTILE_NNODE_H_INCLUDED

namespace Futile {


template<typename T, unsigned N, unsigned H, unsigned D>
struct NNode;


template<typename T, unsigned N, unsigned H, unsigned D>
struct NNodeWithParent
{
    typedef NNode<T, N, H, D - 1> Parent;
    Parent * mParent;
};


template<typename T, unsigned N, unsigned H, unsigned D>
struct NNodeWithChildren
{
    typedef NNode<T, N, H, D + 1> Child;
    Child mChildren[N];
};


/**
 * Partial specialization for D == 0 represents the root node.
 * The root node has no parent node.
 */
template<typename T, unsigned N, unsigned H>
struct NNode<T, N, H, 0> : NNodeWithChildren<T, N, H, 1>
{
    NNode() :
        NNodeWithChildren<T, N, H, 1>(),
        mData()
    {
    }

    T mData;
};


/**
 * Partial specialization for H == D represents the leaf node.
 * Leaf nodes have no children.
 */
template<typename T, unsigned N, unsigned H>
struct NNode<T, N, H, H> : NNodeWithParent<T, N, H, H>
{
    NNode() :
        NNodeWithParent<T, N, H, H>(),
        mData()
    {
    }

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
struct NNode : NNodeWithParent<T, N, H, D>,
               NNodeWithChildren<T, N, H, D>
{
    NNode() :
        NNodeWithParent<T, N, H, D>(),
        NNodeWithChildren<T, N, H, D>(),
        mData()
    {
    }

    T mData;
};


} // namespace Futile


#endif // FUTILE_NNODE_H_INCLUDED
