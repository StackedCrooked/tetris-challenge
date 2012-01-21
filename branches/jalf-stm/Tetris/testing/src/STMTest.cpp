#include "TetrisTest.h"
#include "Futile/WorkerPool.h"
#include "Futile/Worker.h"
#include "stm.hpp"
#include <atomic>
#include <string>


namespace testing {


using namespace Futile;


typedef stm::shared<int> SharedInt;


class STMTest : public TetrisTest
{
public:
    STMTest() :
        stop(false),
        a(0),
        b(0),
        c(0),
        sum_ab(0),
        sum_bc(0),
        sum_ac(0)
    {
    }

    std::atomic_bool stop;
    SharedInt a;
    SharedInt b;
    SharedInt c;
    SharedInt sum_ab;
    SharedInt sum_bc;
    SharedInt sum_ac;

    void increment_a(stm::transaction & tx)
    {
        int & a = this->a.open_rw(tx);
        a++;

        int & sum_ab = this->sum_ab.open_rw(tx);
        sum_ab = a + b.open_r(tx);

        int & sum_ac = this->sum_ac.open_rw(tx);
        sum_ac = a + c.open_r(tx);
    }

    void increment_b(stm::transaction & tx)
    {
        int & b = this->b.open_rw(tx);
        b++;

        int & sum_ab = this->sum_ab.open_rw(tx);
        sum_ab = a.open_r(tx) + b;

        int & sum_bc = this->sum_bc.open_rw(tx);
        sum_bc = b + c.open_r(tx);
    }

    void increment_c(stm::transaction & tx)
    {
        int & c = this->c.open_rw(tx);
        c++;

        int & sum_ac = this->sum_ac.open_rw(tx);
        sum_ac = a.open_r(tx) + c;

        int & sum_bc = this->sum_bc.open_rw(tx);
        sum_bc = b.open_r(tx) + c;
    }

    void check()
    {
        stm::atomic([&](stm::transaction & tx) {
            ASSERT_TRUE(a.open_r(tx) + b.open_r(tx) == sum_ab.open_r(tx));
            ASSERT_TRUE(a.open_r(tx) + c.open_r(tx) == sum_ac.open_r(tx));
            ASSERT_TRUE(b.open_r(tx) + c.open_r(tx) == sum_bc.open_r(tx));
        });
    }

    void test()
    {
        while (!stop)
        {
            stm::atomic([&](stm::transaction & tx) {
                increment_a(tx);
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_b(tx);
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_c(tx);
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_a(tx);
                increment_b(tx);
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_b(tx);
                increment_c(tx);
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_a(tx);
                increment_c(tx);
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_a(tx);
                increment_b(tx);
                increment_c(tx);
            });
            check();
        }
    }

    void printValues()
    {
        stm::atomic([&](stm::transaction & tx) {
            std::cout << "a: " << a.open_r(tx) << std::endl;
            std::cout << "b: " << b.open_r(tx) << std::endl;
            std::cout << "c: " << c.open_r(tx) << std::endl;
            std::cout << "sum_ab: " << sum_ab.open_r(tx) << std::endl;
            std::cout << "sum_ac: " << sum_ac.open_r(tx) << std::endl;
            std::cout << "sum_bc: " << sum_bc.open_r(tx) << std::endl;
        });
    }
};


TEST_F(STMTest, CoordinatedChanges)
{

    WorkerPool workers("Incrementers", 16);
    for (std::size_t idx = 0; idx < workers.size(); ++idx)
    {
        workers.schedule(boost::bind(&STMTest::test, this));
    }

    static const unsigned cDuration = 5000;

    std::cout << "Wait for " << cDuration << "ms..." << std::endl;
    Sleep(cDuration);

    std::cout << "Stopping the workers..." << std::endl;
    stop = true;
    workers.wait();

    std::cout << "Results:" << std::endl;
    printValues();

    std::cout << "Finished." << std::endl;
}


} // namespace testing
