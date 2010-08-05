#include "Player.h"
#include "GameState.h"
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

        
        if (inNode.state().isGameOver())
        {
            // Game over state has no children.
            return;
        }


        // Generate the child nodes for inNode
        GenerateOffspring(inBlocks.front(), inNode, inNode.children());


        
        ChildNodes & children = inNode.children();
        ChildNodes::iterator it = children.begin();
        int selection = 0;
        while (selection < inSelectionCounts.front() && it != children.end())
        {
            ChildNodePtr childNode = *it;
            PopulateNode(*childNode, DropFirst(inBlocks), DropFirst(inSelectionCounts));
            selection++;
            ++it;
        }
    }


    void Player::move(const std::vector<int> & inSelectionCounts)
    {
        PopulateNode(mGame->currentNode(), mGame->getFutureBlocks(inSelectionCounts.size()), inSelectionCounts);
    }


} // namespace Tetris
