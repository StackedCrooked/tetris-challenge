#ifndef FUTILE_NNODE_H_INCLUDED
#define FUTILE_NNODE_H_INCLUDED


namespace Futile {


template<typename T, unsigned N, unsigned H, unsigned D>
struct NNode;


namespace Private {


template<typename T, unsigned N, unsigned H, unsigned D>
struct NNodeWithParent
{
    typedef NNode<T, N, H, D - 1> Parent;

    NNodeWithParent() :
        mParent()
    {
    }

    const Parent * parent() const { return mParent; }

    Parent * parent() { return mParent; }

    Parent * mParent;
};


template<typename T, unsigned N, unsigned H, unsigned D>
struct NNodeWithChildren
{
    typedef NNode<T, N, H, D + 1> Child;

    enum
    {
        ChildCount = N
    };

    NNodeWithChildren() :
        mChildren()
    {
    }

    const Child & getChild(unsigned idx) const { return mChildren[idx]; }

    Child mChildren[ChildCount];
};


template<typename T, unsigned H, unsigned D>
struct NNodeCore
{
    enum
    {
        Height = H,
        Depth = D
    };

    NNodeCore() :
        mData()
    {
    }

    T mData;
};


} // namespace Private


/**
 * Root node:
 * -> node depth = 0
 * -> no parent
 */
template<typename T, unsigned N, unsigned H>
struct NNode<T, N, H, 0> : Private::NNodeCore<T, H, 0>,
                           Private::NNodeWithChildren<T, N, H, 0>
{
    NNode() :
        Private::NNodeCore<T, H, 0>(),
        Private::NNodeWithChildren<T, N, H, 0>()
    {
    }
};


/**
 * Leaf node:
 * -> node depth = tree height
 * -> no children
 */
template<typename T, unsigned N, unsigned H>
struct NNode<T, N, H, H> : Private::NNodeWithParent<T, N, H, H>,
                           Private::NNodeCore<T, H, H>
{
    NNode() :
        Private::NNodeWithParent<T, N, H, H>(),
        Private::NNodeCore<T, H, H>()
    {
    }
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
struct NNode : Private::NNodeWithParent<T, N, H, D>,
               Private::NNodeCore<T, H, D>,
               Private::NNodeWithChildren<T, N, H, D>
{
    NNode() :
        Private::NNodeWithParent<T, N, H, D>(),
        Private::NNodeCore<T, H, D>(),
        Private::NNodeWithChildren<T, N, H, D>()
    {
    }
};


} // namespace Futile


#endif // FUTILE_NNODE_H_INCLUDED
