#include <stm.hpp>
#include <catch.hpp>

TEST_CASE("rollback-nested", "the bug StackedCrooked found")
{
    stm::shared<int> sh(0);
    stm::atomic([&](stm::transaction& tx) {
        stm::atomic([&](stm::transaction& tx) {
            sh.open_r(tx);
        });
        stm::atomic([&](stm::transaction& tx) {
            tx.abort();
        });
    });
}

