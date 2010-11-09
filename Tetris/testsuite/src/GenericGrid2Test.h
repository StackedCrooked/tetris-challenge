#ifndef GENERICGRID2TEST_H_INCLUDED
#define GENERICGRID2TEST_H_INCLUDED


#include "CppUnit/TestCase.h"
#include "Poco/MemoryPool.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


namespace Allocators {


    template<class T>
    class Allocator_Malloc
    {
    public:
        Allocator_Malloc(size_t inSize);
        T * alloc();
        void free(T * inBuffer);

    private:
        size_t mSize;
    };


    template<class T>
    class Allocator_New
    {
    public:
        Allocator_New(size_t inSize);
        T * alloc();
        void free(T * inBuffer);

    private:
        size_t mSize;
    };


    template<class T>
    class Allocator_Pool
    {
    public:
        Allocator_Pool(size_t inSize);
        T * alloc();
        void free(T * inBuffer);

    private:
        Poco::MemoryPool mMemoryPool;
    };


} // namespace Allocators   


class GenericGrid2Test: public CppUnit::TestCase
{
public:
	GenericGrid2Test(const std::string & name);

	~GenericGrid2Test();
    
    void testAllocator_PocoMemoryPool();
    void testAllocator_New();
    void testAllocator_Malloc();
    void testAllocator_PocoMemoryPool_WithInitialValue();
    void testAllocator_New_WithInitialValue();
    void testAllocator_Malloc_WithInitialValue();
	
	void setUp();

	void tearDown();

	static CppUnit::Test* suite();
};


#endif // GENERICGRID2TEST_H_INCLUDED
