#include <stm/transaction.hpp>
#include <stm/fixed_array_buffer.hpp>
#include <stm/shared.hpp>
#include <stm/buffer_functions.hpp>

#include <string>
#include <catch.hpp>

namespace {
	void dummy_destroy(stm::backend::metadata*){}
	void dummy_assign(const stm::backend::metadata*, void*){}

	template <typename T, typename iter_t>
	T& value_from_iter(iter_t iter){
		using namespace stm;
		using namespace stm::backend;
		shared_base** ptr = &*iter;
		typedef buffer_entry<sizeof(T), stm::detail::alignment_of<T>::value> aligned;
		aligned* copy_ptr = reinterpret_cast<aligned*>(ptr);
		T* obj_ptr = reinterpret_cast<T*>(&copy_ptr->storage);
		return *obj_ptr;
	}
	template <typename T>
	void insert_copy(T obj) {
		using namespace stm;
		using namespace stm::backend;
		fixed_array_buffer buf;

		shared_base src; 

		REQUIRE(std::distance(buf.begin_rw(), buf.end_rw()) == 0);
		REQUIRE(push_rw(buf, &src, NULL, obj, &dummy_destroy, &dummy_assign) != NULL);

		//// what does this return? Again, we don't need to know the value, I think, just the identity, so shared*?
		REQUIRE(*buf.begin_rw() == &src);
		REQUIRE(std::distance(buf.begin_rw(), buf.end_rw()) == 1);

		REQUIRE(value_from_iter<T>(buf.begin_rw()) == obj);

		shared_base src2;
		int obj2 = 14;

		REQUIRE(push_rw(buf, &src2, NULL, obj2, &dummy_destroy, &dummy_assign) != NULL); 

		REQUIRE(*buf.begin_rw() == &src2);
		REQUIRE(std::distance(buf.begin_rw(), buf.end_rw()) == 2);
		REQUIRE(*boost::next(buf.begin_rw()) == &src);

		REQUIRE(value_from_iter<int>(buf.begin_rw()) == obj2);
		REQUIRE(value_from_iter<T>(boost::next(buf.begin_rw())) == obj);
	}

	struct consistency_checker {
		consistency_checker() {
			++count;
		}
		consistency_checker(const consistency_checker&) {
			++count;
		}
		~consistency_checker(){
			--count;
		}
		static int count;
	};

	int consistency_checker::count;
}

TEST_CASE("transaction-local", "Testing the transaction-local buffer")
{
	SECTION("insert-readonly", "Opening an object for reading should insert a pointer into the buffer")
	{
		using namespace stm;
		using namespace stm::backend;
		fixed_array_buffer buf;

		shared_base src;

		REQUIRE(std::distance(buf.begin_r(), buf.end_r()) == 0);
		// push_r
		REQUIRE(buf.push_r(&src)); 
		REQUIRE(*buf.begin_r() == &src);
		REQUIRE(std::distance(buf.begin_r(), buf.end_r()) == 1);

		shared_base src2;
		REQUIRE(buf.push_r(&src2)); 
		
		REQUIRE(*buf.begin_r() == &src2);
		REQUIRE(*boost::next(buf.begin_r()) == &src);
		REQUIRE(std::distance(buf.begin_r(), buf.end_r()) == 2);
	}
	
	// todo: need variable size buffer for this one
	//SECTION("insert-readonly-in-full", "opening object for reading when buffer is full should insert pointer into backing store")
	//{
	//	using namespace stm;
	//	using namespace stm::backend;
	//	fixed_array_buffer<1kb> buf;

	//	shared_base src;

	//	size_t capacity = 1024/sizeof(shared_base*);
	//	for (size_t i = 0; i < capacity; ++i){
	//		REQUIRE(buf.push_r(&src)); // failed on 129th
	//	}
	//	REQUIRE(std::distance(buf.begin_r(), buf.end_r()) == static_cast<ptrdiff_t>(capacity));
	//	REQUIRE(!buf.push_r(&src));
	//	REQUIRE(std::distance(buf.begin_r(), buf.end_r()) == static_cast<ptrdiff_t>(capacity));
	//}

	SECTION("insert-rw", "opening an object for writing should place a full copy in the buffer")
	{
		insert_copy<char>('c');
		insert_copy<double>(42.0);
		insert_copy<std::string>("hello");
	}

	// todo: need variable size buffer for this one
	//SECTION("insert-rw-in-full", "opening an object for writing when buffer is full should place copy in backing store")
	//{
	//	using namespace stm;
	//	using namespace stm::backend;
	//	fixed_array_buffer<1> buf;

	//	shared_base src;
	//	double obj = 42.0;
	//	size_t size = sizeof(buffer_entry<sizeof(double), stm::detail::alignment_of<double>::value>);
	//	size_t capacity = 1024/size;
	//	for (size_t i = 0; i < capacity; ++i){
	//		REQUIRE(*buf.push_rw(&src, NULL, obj, &dummy_destroy, &dummy_assign) == obj);
	//	}

	//	REQUIRE(std::distance(buf.begin_rw(), buf.end_rw()) == static_cast<ptrdiff_t>(capacity));
	//	REQUIRE(buf.push_rw(&src, NULL, obj, dummy_destroy, &dummy_assign) == static_cast<void*>(NULL));
	//	REQUIRE(std::distance(buf.begin_rw(), buf.end_rw()) == static_cast<ptrdiff_t>(capacity));
	//}

	// todo: need variable size buffer for this one
	//SECTION("overflow-with-mixed", "insert r and rw until buffer is full")
	//{
	//	using namespace stm;
	//	using namespace stm::backend;
	//	fixed_array_buffer<1> buf;

	//	shared_base src;
	//	double obj = 42.0;
	//	size_t size = sizeof(buffer_entry<sizeof(double), stm::detail::alignment_of<double>::value>) + sizeof(shared_base*);
	//	size_t capacity = 1024/size;
	//	for (size_t i = 0; i < capacity; ++i){
	//		REQUIRE(*buf.push_rw(&src, NULL, obj, dummy_destroy, &dummy_assign) == obj);
	//		REQUIRE(buf.push_r(&src));
	//	}

	//	REQUIRE(buf.push_rw(&src, NULL, obj, dummy_destroy, &dummy_assign) == static_cast<void*>(NULL));
	//	size_t read_count = (1024 - (capacity * size)) / sizeof(shared_base*);
	//	for (size_t i = 0; i < read_count; ++i){
	//		REQUIRE(buf.push_r(&src));
	//	}
	//	REQUIRE(!buf.push_r(&src));
	//}

	// todo: need variable size buffer for this one
	//SECTION("dtor-called-on-buffered-object", "when a copy is placed in the bufer, it should be properly destroyed afterwards")
	//{
	//	{
	//		using namespace stm;
	//		using namespace stm::backend;
	//		fixed_array_buffer<1> buf;

	//		shared_base src;
	//		consistency_checker obj;

	//		buf.push_rw(&src, NULL, obj, stm::frontend::destroy<consistency_checker>, &stm::frontend::assign<consistency_checker, stm::frontend::shared_internal<consistency_checker>, 1>);
	//	}

	//	REQUIRE(consistency_checker::count == 0);
	//}
}
