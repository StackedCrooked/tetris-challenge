#include "Player.h"
#include "ErrorHandling.h"


namespace Tetris
{


    Player::Player(Game * inGame) :
        mGame(inGame)
    {
    }


    template<class T>
    std::vector<T> DropFirst(const std::vector<T> & inValues)
    {
        std::vector<T> result;
        for (size_t i = 1; i < inValues.size(); ++i)
        {
            result.push_back(inValues[i]);
        }
        return result;
    }


    void PopulateNode(GameStateNode & inNode, const std::vector<Block> & inBlocks, const std::vector<int> & inSelectionCounts)
    {
        CheckArgument(inBlocks.size() == inSelectionCounts.size(), "PopulateNode got inBlocks.size() != inSelectionCounts.size()");
        if (inBlocks.empty())
        {
            return;
        }

        inNode.populate(inBlocks.front());
        for (size_t i = 0; i != inSelectionCounts.size(); ++i)
        {
            Children & children = inNode.children();
            Children::iterator it = children.begin();
            const int selectionCount = inSelectionCounts[i];
            for (int j = 0; j < selectionCount && it != children.end(); ++j)
            {
                PopulateNode(*(it->get()), DropFirst(inBlocks), DropFirst(inSelectionCounts));
            }
        }
    }


    void Player::move(const std::vector<int> & inDepths)
    {
        PopulateNode(mGame->currentNode(), mGame->getFutureBlocks(inDepths.size()), inDepths);
    }


} // namespace Tetris
