#include <stm/fixed_array_buffer.hpp>
#include <stm/shared.hpp>
#include <stm/buffer_functions.hpp>

#include <string>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE( TxLocal )

void dummy_destroy(stm::backend::metadata*){}
void dummy_assign(const stm::backend::metadata*, void*){}

BOOST_AUTO_TEST_CASE( insert_read )
{
	using namespace stm;
	using namespace stm::backend;
	fixed_array_buffer<> buf;

	shared_base src;

	BOOST_CHECK_EQUAL(std::distance(buf.begin_r(), buf.end_r()), 0);
	// push_r
	BOOST_CHECK(buf.push_r(&src));

	BOOST_CHECK_EQUAL(*buf.begin_r(), &src);
	BOOST_CHECK_EQUAL(std::distance(buf.begin_r(), buf.end_r()), 1);

	shared_base src2;
	BOOST_CHECK(buf.push_r(&src2));

	BOOST_CHECK_EQUAL(*buf.begin_r(), &src2);
	BOOST_CHECK_EQUAL(*boost::next(buf.begin_r()), &src);
	BOOST_CHECK_EQUAL(std::distance(buf.begin_r(), buf.end_r()), 2);
}

BOOST_AUTO_TEST_CASE( insert_read_in_full)
{
	using namespace stm;
	using namespace stm::backend;
	fixed_array_buffer<1> buf;

	shared_base src;

	size_t capacity = 1024/sizeof(shared_base*);
	for (size_t i = 0; i < capacity; ++i){
		BOOST_CHECK(buf.push_r(&src));
	}
	BOOST_CHECK_EQUAL(std::distance(buf.begin_r(), buf.end_r()), static_cast<ptrdiff_t>(capacity));
	BOOST_CHECK(!buf.push_r(&src));
	BOOST_CHECK_EQUAL(std::distance(buf.begin_r(), buf.end_r()), static_cast<ptrdiff_t>(capacity));
}

template <typename T, typename iter_t>
T& value_from_iter(iter_t iter){
	using namespace stm;
	using namespace stm::backend;
	shared_base** ptr = &*iter;
	typedef buffer_entry<sizeof(T), stm::detail::alignment_of<T>::value> aligned;
	aligned* copy_ptr = stm::ptr_cast<aligned*>(ptr);
	T* obj_ptr = stm::ptr_cast<T*>(&copy_ptr->storage);
	return *obj_ptr;
}
template <typename T>
void insert_copy(T obj) {
	using namespace stm;
	using namespace stm::backend;
	fixed_array_buffer<> buf;

	shared_base src; 

	BOOST_CHECK_EQUAL(std::distance(buf.begin_rw(), buf.end_rw()), 0);
	BOOST_CHECK_NE(buf.push_rw(&src, NULL, obj, &dummy_destroy, &dummy_assign), static_cast<T*>(NULL));

	//// what does this return? Again, we don't need to know the value, I think, just the identity, so shared*?
	BOOST_CHECK_EQUAL(*buf.begin_rw(), &src);
	BOOST_CHECK_EQUAL(std::distance(buf.begin_rw(), buf.end_rw()), 1);

	BOOST_CHECK_EQUAL(value_from_iter<T>(buf.begin_rw()), obj);

	shared_base src2;
	int obj2 = 14;

	BOOST_CHECK_NE(buf.push_rw(&src2, NULL, obj2, &dummy_destroy, &dummy_assign), static_cast<int*>(NULL));

	BOOST_CHECK_EQUAL(*buf.begin_rw(), &src2);
	BOOST_CHECK_EQUAL(std::distance(buf.begin_rw(), buf.end_rw()), 2);
	BOOST_CHECK_EQUAL(*boost::next(buf.begin_rw()), &src);

	BOOST_CHECK_EQUAL(value_from_iter<int>(buf.begin_rw()), obj2);
	BOOST_CHECK_EQUAL(value_from_iter<T>(boost::next(buf.begin_rw())), obj);
}

BOOST_AUTO_TEST_CASE(insert_copy_test)
{
	insert_copy<char>('c');
	insert_copy<double>(42.0);
	insert_copy<std::string>("hello");
}

BOOST_AUTO_TEST_CASE( insert_copy_in_full)
{
	using namespace stm;
	using namespace stm::backend;
	fixed_array_buffer<1> buf;

	shared_base src;
	double obj = 42.0;
	size_t size = sizeof(buffer_entry<sizeof(double), stm::detail::alignment_of<double>::value>);
	size_t capacity = 1024/size;
	for (size_t i = 0; i < capacity; ++i){
		BOOST_CHECK_EQUAL(*buf.push_rw(&src, NULL, obj, &dummy_destroy, &dummy_assign), obj);
	}

	BOOST_CHECK_EQUAL(std::distance(buf.begin_rw(), buf.end_rw()), static_cast<ptrdiff_t>(capacity));
	BOOST_CHECK_EQUAL(buf.push_rw(&src, NULL, obj, dummy_destroy, &dummy_assign), static_cast<void*>(NULL));
	BOOST_CHECK_EQUAL(std::distance(buf.begin_rw(), buf.end_rw()), static_cast<ptrdiff_t>(capacity));
}

// and finally, insert into mixed, until full
BOOST_AUTO_TEST_CASE( insert_both)
{
	using namespace stm;
	using namespace stm::backend;
	fixed_array_buffer<1> buf;

	shared_base src;
	double obj = 42.0;
	size_t size = sizeof(buffer_entry<sizeof(double), stm::detail::alignment_of<double>::value>) + sizeof(shared_base*);
	size_t capacity = 1024/size;
	for (size_t i = 0; i < capacity; ++i){
		BOOST_CHECK_EQUAL(*buf.push_rw(&src, NULL, obj, dummy_destroy, &dummy_assign), obj);
		BOOST_CHECK(buf.push_r(&src));
	}

	BOOST_CHECK_EQUAL(buf.push_rw(&src, NULL, obj, dummy_destroy, &dummy_assign), static_cast<void*>(NULL));
	size_t read_count = (1024 - (capacity * size)) / sizeof(shared_base*);
	for (size_t i = 0; i < read_count; ++i){
		BOOST_CHECK(buf.push_r(&src));
	}
	BOOST_CHECK(!buf.push_r(&src));
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

BOOST_AUTO_TEST_CASE( dtor_called_on_buffer_entry )
{
	{
		using namespace stm;
		using namespace stm::backend;
		fixed_array_buffer<1> buf;

		shared_base src;
		consistency_checker obj;


		buf.push_rw(&src, NULL, obj, stm::frontend::destroy<consistency_checker>, &stm::frontend::assign<consistency_checker, stm::frontend::shared_internal<consistency_checker>, 1>);
	}

	BOOST_CHECK_EQUAL(consistency_checker::count, 0);

}

BOOST_AUTO_TEST_SUITE_END()
