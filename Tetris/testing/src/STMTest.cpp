#include "TetrisTest.h"
#include "stm.hpp"
#include <string>


namespace testing {


typedef stm::shared<int> SharedInt;


struct STMTest : public TetrisTest
{
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

    void check(stm::transaction & tx)
    {
        stm::atomic([&](stm::transaction & tx) { a.open_r(tx) + b.open_r(tx) == sum_ab.open_r(tx); });
        stm::atomic([&](stm::transaction & tx) { a.open_r(tx) + c.open_r(tx) == sum_ac.open_r(tx); });
        stm::atomic([&](stm::transaction & tx) { b.open_r(tx) + c.open_r(tx) == sum_bc.open_r(tx); });
    }

};


TEST_F(STMTest, CoordinatedChanges)
{

}


} // namespace testing
