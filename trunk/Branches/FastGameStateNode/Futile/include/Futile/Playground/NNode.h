#ifndef FUTILE_NNODE_H_INCLUDED
#define FUTILE_NNODE_H_INCLUDED


namespace Futile {


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
    NNodeWithChildren()
    {
        for (std::size_t idx = 0; idx < N; ++idx)
        {
            Child & child = mChildren[idx];
            child.mParent = static_cast< NNode<T, N, H, D>* >(this);
        }
    }

    const Child & getChild(unsigned idx) const { return mChildren[idx]; }

    Child mChildren[N];
};


template<unsigned D>
struct NNodeCore
{
    enum
    {
        Depth = D
    };
};


} // namespace Private



/**
 * NNodeBase
 */
template<typename T, unsigned N, unsigned H>
struct NNodeBase
{
    typedef T DataType;
    enum {
        ChildCount = N,
        TreeHeight = H
    };

    NNodeBase() :
        mData()
    {
    }

    DataType mData;
};


/**
 * Root node:
 * -> node depth = 0
 * -> no parent
 */
template<typename T, unsigned N, unsigned H>
struct NNode<T, N, H, 0> : NNodeBase<T, N, H>,
                           Private::NNodeCore<0>,
                           Private::NNodeWithChildren<T, N, H, 0>
{
    NNode() :
        Private::NNodeCore<0>(),
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
struct NNode<T, N, H, H> : NNodeBase<T, N, H>,
                           Private::NNodeWithParent<T, N, H, H>,
                           Private::NNodeCore<H>
{
    typedef NNode<T, N, H, H - 1> Parent;
    NNode() :
        Private::NNodeWithParent<T, N, H, H>(),
        Private::NNodeCore<H>()
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
struct NNode : NNodeBase<T, N, H>,
               Private::NNodeWithParent<T, N, H, D>,
               Private::NNodeCore<D>,
               Private::NNodeWithChildren<T, N, H, D>
{
    typedef NNode<T, N, H, D - 1> Parent;

    NNode() :
        Private::NNodeWithParent<T, N, H, D>(),
        Private::NNodeCore<D>(),
        Private::NNodeWithChildren<T, N, H, D>()
    {
    }
};


/**
 * For the root node you can either use:
 * - NNode<T, N, H, 0>
 * - RootNode<T, N, H>
 */
template<typename T, unsigned N, unsigned H>
struct RootNode : public NNode<T, N, H, 0>
{

};


} // namespace Futile


#endif // FUTILE_NNODE_H_INCLUDED
