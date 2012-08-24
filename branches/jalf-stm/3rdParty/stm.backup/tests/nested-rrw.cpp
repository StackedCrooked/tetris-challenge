#include <stm.hpp>
#include <catch.hpp>
#include <boost/thread.hpp>


stm::shared<int> i(0);

void deadly() {
    stm::atomic([&](stm::transaction& tx) {
        stm::atomic([&](stm::transaction& tx) {
            i.open_r(tx);
        });
        stm::atomic([&](stm::transaction& tx) {
            i.open_rw(tx);
        });

    });
}



TEST_CASE("boo", "blah blah")
{
    deadly();
    REQUIRE(true);
}

TEST_CASE("threaded_boo", "blah")
{
    boost::thread_group grp;
    bool stop = false;

    for (int x = 0; x != 2; ++x) {
        grp.create_thread([&]() {
            while (!stop) {
                deadly();
            }
        });
    }

    boost::this_thread::sleep(boost::posix_time::seconds(1));

    stop = true;

    grp.join_all();
    REQUIRE(true);
}
