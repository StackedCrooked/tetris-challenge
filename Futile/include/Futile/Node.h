#ifndef NODE_H
#define NODE_H


#include <boost/shared_ptr.hpp>
#include <set>
#include <vector>


template<class ValueType>
struct ContainerPolicy_Set
{
    typedef std::set<ValueType> Container;
    static void insert(Container& ioContainer, const ValueType& inValue)
    {
        ioContainer.insert(inValue);
    }

    static size_t size(const Container& ioContainer)
    {
        return ioContainer.size();
    }

    static bool empty(const Container& ioContainer)
    {
        return ioContainer.empty();
    }
};


template<class ValueType>
struct ContainerPolicy_Vector
{
    typedef std::vector<ValueType> Container;
    static void insert(Container& ioContainer, const ValueType& inValue)
    {
        ioContainer.push_back(inValue);
    }

    static size_t size(const Container& ioContainer)
    {
        return ioContainer.size();
    }

    static bool empty(const Container& ioContainer)
    {
        return ioContainer.empty();
    }
};


template <class PointeeType>
struct PointerPolicy_Normal_NoOwnership
{
    typedef PointeeType * PointerType;

    static PointeeType * getRaw(PointerType p)
    {
        return p;
    }

    static void destroy(PointerType p) { }
};


template <class PointeeType>
struct PointerPolicy_Normal_WithOwnership
{
    typedef PointeeType * PointerType;

    static PointeeType * getRaw(PointerType p)
    {
        return p;
    }

    static void destroy(PointerType p)
    {
        delete p;
    }
};


template <class PointeeType>
struct PointerPolicy_Shared
{
    typedef boost::shared_ptr<PointeeType> PointerType;

    static PointeeType * getRaw(PointerType p)
    {
        return p.get();
    }

    static void destroy(PointerType p) { }
};


template <class TDataType,
          template <class> class ContainerPolicy = ContainerPolicy_Vector,
          template <class> class PointerPolicy = PointerPolicy_Normal_WithOwnership>
class GenericNode
{
public:
    typedef TDataType DataType;

    GenericNode() { }

    GenericNode(const DataType& inData) : mData(inData) { }

    typedef GenericNode<DataType, ContainerPolicy, PointerPolicy> This;
    typedef typename PointerPolicy<This>::PointerType ChildPtr;
    typedef typename ContainerPolicy<ChildPtr>::Container Container;

    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;

    ~GenericNode()
    {
        const_iterator it = this->begin(), endIt = this->end();
        for (; it != endIt; ++it)
        {
            PointerPolicy<This>::destroy(*it);
        }
    }

    iterator begin()
    {
        return mChildren.begin();
    }

    iterator end()
    {
        return mChildren.end();
    }

    const_iterator begin() const
    {
        return mChildren.begin();
    }

    const_iterator end() const
    {
        return mChildren.end();
    }

    void insert(This * inItem)
    {
        ChildPtr item(inItem);
        ContainerPolicy<ChildPtr>::insert(mChildren, item);
    }

    size_t size() const
    {
        return ContainerPolicy<ChildPtr>::size(mChildren);
    }

    bool empty() const
    {
        return ContainerPolicy<ChildPtr>::empty(mChildren);
    }

    DataType& data()
    {
        return mData;
    }

    const DataType& data() const
    {
        return mData;
    }

    void setData(const DataType& inData)
    {
        mData = inData;
    }

private:
    DataType mData;
    Container mChildren;
};


template<class T,
         template<class> class C,
         template<class> class P>
bool HasCycles(const GenericNode<T, C, P>& inNode,
               std::vector<const T*> inPreviousNodes)
{
    typedef GenericNode<T, C, P> Node;
    typedef typename Node::Container Container;

    if (!inNode.empty())
    {
        for (typename Container::const_iterator it = inNode.begin(),
                                                end = inNode.end();
             it != end;
             ++it)
        {
            GenericNode<T, C, P>& child = **it;
            if (std::find(inPreviousNodes.begin(), inPreviousNodes.end(), &child.data()) != inPreviousNodes.end())
            {
                return true;
            }
            else
            {
                inPreviousNodes.push_back(&inNode.data());

                // Previous nodes object is passed by value!
                // This is an important aspect of the algorithm!
                if (HasCycles(child, inPreviousNodes))
                {
                    return true;
                }
            }
        }
    }
    return false;
}


template<class T,
         template<class> class C,
         template<class> class P>
bool HasCycles(const GenericNode<T, C, P>& inNode)
{
    return HasCycles(inNode, std::vector<const T*>());
}


#endif // NODE_H
