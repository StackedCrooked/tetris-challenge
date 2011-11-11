#include "TetrisTest.h"
#include "stm.hpp"
#include <string>


namespace testing {


class STMTest : public TetrisTest
{
};


TEST_F(STMTest, General)
{
    typedef stm::shared<std::string> SharedString;
    SharedString sharedString(std::string("There"));

    stm::atomic([&](stm::transaction & tx){
        std::string & str = sharedString.open_rw(tx);
        str.append("A");
    });
}


} // namespace testing
