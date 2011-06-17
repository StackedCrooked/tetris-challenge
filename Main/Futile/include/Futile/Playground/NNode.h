#ifndef FUTILE_NNODE_H_INCLUDED
#define FUTILE_NNODE_H_INCLUDED

namespace Futile {


/**
 * Base class can be used to refer to the parent.
 */
struct NNodeBase
{
};


/**
 * NNode represents a node in an n-ary tree.
 * T = data type
 * N = maximum number of children per node
 * H = tree height
 * D = node depth (distance from the root node)
 *
 * The entire tree is contained in a single segement of contiguous memory.
 */
template<typename T, unsigned N, unsigned H, unsigned D = 0>
struct NNode : public NNodeBase
{
    NNode() :
        mParent(NULL),
        mData(),
        mChildren()
    {
    }

    NNodeBase * mParent;
    T mData;
    typedef NNode<T, N, H, D + 1> Child;
    Child mChildren[N];
};


/**
 * Partial specialization for H == D represents the leaf node.
 */
template<typename T, unsigned N, unsigned H>
struct NNode<T, N, H, H> : public NNodeBase
{
    NNode() :
        mParent(NULL),
        mData()
    {
    }

    NNodeBase * mParent;
    T mData;
};


} // namespace Futile


#endif // FUTILE_NNODE_H_INCLUDED
