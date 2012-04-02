#include "TetrisTest.h"
#include "Futile/WorkerPool.h"
#include "Futile/Worker.h"
#include "Futile/Logger.h"
#include "Futile/Logging.h"
#include "stm.hpp"
#include <atomic>
#include <string>


namespace testing {


using namespace Futile;


typedef stm::shared<int> shared_int;


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
    shared_int a;
    shared_int b;
    shared_int c;
    shared_int sum_ab;
    shared_int sum_bc;
    shared_int sum_ac;

    void increment_a()
    {
        stm::atomic([this](stm::transaction& tx{
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
        stm::atomic([this](stm::transaction& tx{
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
        stm::atomic([this](stm::transaction& tx{
        int & c = this->c.open_rw(tx);
        c++;

        int & sum_ac = this->sum_ac.open_rw(tx);
        sum_ac = a.open_r(tx) + c;

        int & sum_bc = this->sum_bc.open_rw(tx);
        sum_bc = b.open_r(tx) + c;
        });
    }

    void check()
    {
        printValues();
        stm::atomic([&](stm::transaction & tx) {
            ASSERT_TRUE(a.open_r(tx) + b.open_r(tx) == sum_ab.open_r(tx));
            ASSERT_TRUE(a.open_r(tx) + c.open_r(tx) == sum_ac.open_r(tx));
            ASSERT_TRUE(b.open_r(tx) + c.open_r(tx) == sum_bc.open_r(tx));
        });
        FlushLogs();
    }

    void test()
    {
        while (!stop)
        {
            stm::atomic([&](stm::transaction & tx) {
                increment_a();
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_b();
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_c();
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_a();
                increment_b();
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_b();
                increment_c();
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_a();
                increment_c();
            });
            check();

            stm::atomic([&](stm::transaction & tx) {
                increment_a();
                increment_b();
                increment_c();
            });
            check();
        }
    }

    void printValues()
    {
        stm::atomic([&](stm::transaction & tx) {
            LogInfo(SS() << "a: " << a.open_r(tx));
            LogInfo(SS() << "b: " << b.open_r(tx));
            LogInfo(SS() << "c: " << c.open_r(tx));
            LogInfo(SS() << "sum_ab: " << sum_ab.open_r(tx));
            LogInfo(SS() << "sum_ac: " << sum_ac.open_r(tx));
            LogInfo(SS() << "sum_bc: " << sum_bc.open_r(tx));
        });
        FlushLogs();
    }
};


TEST_F(STMTest, CoordinatedChanges)
{

    WorkerPool workers("Incrementers", 16);
    for (std::size_t idx = 0; idx < workers.size(); ++idx)
    {
        workers.schedule(boost::bind(&STMTest::test, this));
    }

    static const unsigned cDuration = 1000;

    LogInfo(SS() << "Wait for " << cDuration << "ms...");
    Sleep(cDuration);

    LogInfo(SS() << "Stopping the workers...");
    stop = true;
    workers.wait();

    LogInfo(SS() << "Results:");
    printValues();

    LogInfo(SS() << "Finished.");
}


} // namespace testing
