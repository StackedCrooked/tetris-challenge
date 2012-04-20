#include "stm.hpp"
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>


typedef stm::shared<int> shared_int;


// Global stop flag.
std::atomic_bool gStop(false);


struct STMTest : std::thread
{
    STMTest() :
        std::thread(std::bind(&STMTest::test, this)),
        a(0),
        b(0),
        c(0),
        sum_ab(0),
        sum_bc(0),
        sum_ac(0)
    {
    }

    shared_int a;
    shared_int b;
    shared_int c;
    shared_int sum_ab;
    shared_int sum_bc;
    shared_int sum_ac;

    void increment_a()
    {
        stm::atomic([this](stm::transaction& tx) {
        int & a = this->a.open_rw(tx);
        a++;

        int & sum_ab = this->sum_ab.open_rw(tx);
        sum_ab = a + b.open_r(tx);

        int & sum_ac = this->sum_ac.open_rw(tx);
        sum_ac = a + c.open_r(tx);
        });
    }

    void increment_b()
    {
        stm::atomic([this](stm::transaction& tx) {
        int & b = this->b.open_rw(tx);
        b++;

        int & sum_ab = this->sum_ab.open_rw(tx);
        sum_ab = a.open_r(tx) + b;

        int & sum_bc = this->sum_bc.open_rw(tx);
        sum_bc = b + c.open_r(tx);
        });
    }

    void increment_c()
    {
        stm::atomic([this](stm::transaction& tx) {
        int & c = this->c.open_rw(tx);
        c++;

        int & sum_ac = this->sum_ac.open_rw(tx);
        sum_ac = a.open_r(tx) + c;

        int & sum_bc = this->sum_bc.open_rw(tx);
        sum_bc = b.open_r(tx) + c;
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
        stm::atomic([&](stm::transaction & tx) {
            assertTrue(a.open_r(tx) + b.open_r(tx) == sum_ab.open_r(tx));
            assertTrue(a.open_r(tx) + c.open_r(tx) == sum_ac.open_r(tx));
            assertTrue(b.open_r(tx) + c.open_r(tx) == sum_bc.open_r(tx));
        });
    }

    void test()
    {
        while (!gStop)
        {
            stm::atomic([&](stm::transaction & ) {
                increment_a();
            });
            check();

            stm::atomic([&](stm::transaction & ) {
                increment_b();
            });
            check();

            stm::atomic([&](stm::transaction & ) {
                increment_c();
            });
            check();

            stm::atomic([&](stm::transaction & ) {
                increment_a();
                increment_b();
            });
            check();

            stm::atomic([&](stm::transaction & ) {
                increment_b();
                increment_c();
            });
            check();

            stm::atomic([&](stm::transaction & ) {
                increment_a();
                increment_c();
            });
            check();

            stm::atomic([&](stm::transaction & ) {
                increment_a();
                increment_b();
                increment_c();
            });
            check();
        }
    }

    typedef std::vector<int> Values;
    Values getValues(stm::transaction & tx)
    {
        Values results;
        results.push_back(a.open_r(tx));
        results.push_back(b.open_r(tx));
        results.push_back(c.open_r(tx));
        results.push_back(sum_ab.open_r(tx));
        results.push_back(sum_ac.open_r(tx));
        results.push_back(sum_bc.open_r(tx));
        return results;
    }

    void printValues()
    {
        Values values = stm::atomic<Values>([&](stm::transaction & tx) { return this->getValues(tx); });
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
};


void test()
{
    std::vector<STMTest> workers(16);

    static const unsigned cDuration = 5;

    std::cout << "Wait for " << cDuration << "s..." << std::endl;
    usleep(cDuration * 1000 * 1000);

    std::cout << "Stopping the workers..." << std::endl;
    gStop = true;

    std::cout << "Results:" << std::endl;
    for (auto & worker : workers)
    {
        worker.join();
        worker.printValues();
    }
    std::cout << std::endl;

    std::cout << "Finished." << std::endl;
}


int main()
{
    test();
}
