#include "stm.hpp"
#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>


std::atomic<bool>& stop_flag()
{
    static std::atomic<bool> fStop;
    return fStop;
}


typedef stm::shared<int> shared_int;


struct Globals
{
    shared_int a;
    shared_int b;
    shared_int c;
    shared_int sum_ab;
    shared_int sum_bc;
    shared_int sum_ac;
};


Globals& globals()
{
    static Globals fGlobals;
    return fGlobals;
}


void increment_a()
{
    stm::atomic([&](stm::transaction& tx) {
    int& a = globals().a.open_rw(tx);
    a++;

    int& sum_ab = globals().sum_ab.open_rw(tx);
    sum_ab = a + globals().b.open_r(tx);

    int& sum_ac = globals().sum_ac.open_rw(tx);
    sum_ac = a + globals().c.open_r(tx);
    });
}


void increment_b()
{
    stm::atomic([&](stm::transaction& tx) {
    int& b = globals().b.open_rw(tx);
    b++;

    int& sum_ab = globals().sum_ab.open_rw(tx);
    sum_ab = globals().a.open_r(tx) + b;

    int& sum_bc = globals().sum_bc.open_rw(tx);
    sum_bc = b + globals().c.open_r(tx);
    });
}


void increment_c()
{
    stm::atomic([&](stm::transaction& tx) {
    int& c = globals().c.open_rw(tx);
    c++;

    int& sum_ac = globals().sum_ac.open_rw(tx);
    sum_ac = globals().a.open_r(tx) + c;

    int& sum_bc = globals().sum_bc.open_rw(tx);
    sum_bc = globals().b.open_r(tx) + c;
    });
}


void assertTrue(bool b)
{
    if (!b)
    {
        throw std::logic_error("Assertion failed.");
    }
}


void check()
{
    stm::atomic([&](stm::transaction& tx) {
        assertTrue(globals().a.open_r(tx) + globals().b.open_r(tx) == globals().sum_ab.open_r(tx));
        assertTrue(globals().a.open_r(tx) + globals().c.open_r(tx) == globals().sum_ac.open_r(tx));
        assertTrue(globals().b.open_r(tx) + globals().c.open_r(tx) == globals().sum_bc.open_r(tx));
    });
}


struct STMTest : std::thread
{
    STMTest() : std::thread(std::bind(&STMTest::test)) { }

    static void test()
    {
        while (!stop_flag())
        {
            stm::atomic([&](stm::transaction& ) {
                increment_a();
            });
            check();

            stm::atomic([&](stm::transaction& ) {
                increment_b();
            });
            check();

            stm::atomic([&](stm::transaction& ) {
                increment_c();
            });
            check();

            stm::atomic([&](stm::transaction& ) {
                increment_a();
                increment_b();
            });
            check();

            stm::atomic([&](stm::transaction& ) {
                increment_b();
                increment_c();
            });
            check();

            stm::atomic([&](stm::transaction& ) {
                increment_a();
                increment_c();
            });
            check();

            stm::atomic([&](stm::transaction& ) {
                increment_a();
                increment_b();
                increment_c();
            });
            check();
        }
    }
};


typedef std::vector<int> Values;


Values getValues(stm::transaction& tx)
{
    Values results;
    results.push_back(globals().a.open_r(tx));
    results.push_back(globals().b.open_r(tx));
    results.push_back(globals().c.open_r(tx));
    results.push_back(globals().sum_ab.open_r(tx));
    results.push_back(globals().sum_ac.open_r(tx));
    results.push_back(globals().sum_bc.open_r(tx));
    return results;
}


void printValues()
{
    Values values = stm::atomic<Values>([&](stm::transaction& tx) { return getValues(tx); });
    for (Values::size_type idx = 0; idx < values.size(); ++idx)
    {
        if (idx != 0)
        {
            std::cout << ", ";
        }

        if (idx % 6 == 0)
        {
            std::cout << std::endl;
        }

        std::cout << values[idx];
    }
}


void test()
{
    static const unsigned cDuration = 5;
    std::cout << "Run 4 workers for " << cDuration << "s..." << std::endl;

    // Creates 4 workers.
    STMTest worker_1;
    STMTest worker_2;
    STMTest worker_3;
    STMTest worker_4;

    usleep(cDuration * 1000 * 1000);

    std::cout << "Setting the stop flag." << std::endl;
    stop_flag() = true;

    std::cout << "Joining threads." << std::endl;
    worker_1.join();
    worker_2.join();
    worker_3.join();
    worker_4.join();

    std::cout << "Results:" << std::endl;
    printValues();

    std::cout << "\nFinished." << std::endl;
}


int main()
{
    std::cout << "Starting STMTest" << std::endl;
    test();
}
