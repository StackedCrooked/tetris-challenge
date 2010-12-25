#include "PuzzleSolver.h"
#include "Parser.h"
#include <algorithm>
#include <sstream>

namespace Tetris
{

    PuzzleSolver::PuzzleSolver() :
        mRootNode(0, Block(' ', NO_BLOCK, 0), 0, 0)
    {
        Parser p;
        p.parse("inputs.txt", mBlocks);
    }


    const GameStateNode * PuzzleSolver::currentNode() const
    {
        if (!mNodes.empty())
        {
            return mNodes.back()->get();
        }
        return &mRootNode;
    }


    const GameStateNode * PuzzleSolver::rootNode() const
    {
        return &mRootNode;
    }


    void PuzzleSolver::populateNode(GameStateNode * inNode, const std::vector<Block> & inBlockTypes) const
    {
        if (!inBlockTypes.empty())
        {
            assert(inNode->children().empty());
            if (inNode->children().empty() && !inNode->isDeadEnd())
            {
                GameStateNode::Children futureGameStates;
                generateFutureGameStates(*inNode, inBlockTypes[0], 2, futureGameStates);
                if (!futureGameStates.empty())
                {
                    inNode->setChildren(futureGameStates);
                    if (inBlockTypes.size() > 1)
                    {
                        std::vector<Block> blockTypes;
                        for (size_t idx = 1; idx < inBlockTypes.size(); ++idx)
                        {
                            blockTypes.push_back(inBlockTypes[idx]);
                        }
                        GameStateNode::Children::const_iterator it = inNode->children().begin(), end = inNode->children().end();
                        for (; it != end; ++it)
                        {
                            populateNode(it->get(), blockTypes);
                        }
                    }
                }
                else
                {
                    inNode->markAsDeadEnd();
                }
            }
        }
    }


    bool PuzzleSolver::next()
    {
        if (mNodes.size() < mBlocks.size())
        {
            std::vector<Block> blockIds;
            {
                blockIds.push_back(mBlocks[mNodes.size()]);
            }
            GameStateNode * currentNode = mNodes.empty() ? &mRootNode : mNodes.back()->get();
            populateNode(currentNode, blockIds);
            if (!currentNode->isDeadEnd() && !currentNode->children().empty())
            {
                GameStateNode::Children::iterator newNodeIt = currentNode->children().begin();
                mNodes.push_back(newNodeIt);
                return true;
            }
            else
            {
                tryNextBranch();
                next();
            }
        }
        return false;
    }


    void PuzzleSolver::tryNextBranch()
    {
        if (!mNodes.empty())
        {
            // Since this branch didn't deliver any results we remove
            // its children to free up some memory
            mNodes.back()->get()->children().clear();

            // ..and mark as dead end, so that we remember not to
            // search this node again.
            mNodes.back()->get()->markAsDeadEnd();


            GameStateNode::Children::iterator & currentNodeIt = mNodes.back();
            GameStateNode * parent = (*currentNodeIt)->parent();
            if (parent)
            {
                assert(currentNodeIt != parent->children().end());

                // move to next branch
                ++currentNodeIt;

                // if there is no next branch, then try a level higher
                if (currentNodeIt == parent->children().end())
                {
                    Nodes::iterator it = mNodes.end();
                    --it;
                    mNodes.erase(it);
                    tryNextBranch();
                }
            }
        }
    }


    void PuzzleSolver::generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlock, size_t inLimitNumberOfGameStates, GameStateNode::Children & outGameGrids) const
    {
        const int numRotations = NumRotations(inBlock.type());
        for (size_t rotIdx = 0; rotIdx != numRotations; ++rotIdx)
        {
            int rotation = (inBlock.rotation() + rotIdx)%numRotations;
            Block block(Block(inBlock.charId(), inBlock.type(), rotation));

            size_t maxCol = inGameStateNode.state().grid().numColumns() - block.grid().numColumns();
            for (size_t colIdx = 0; colIdx <= maxCol; ++colIdx)
            {
                generateFutureGameStates(inGameStateNode, block, colIdx, inLimitNumberOfGameStates, outGameGrids);
            }
        }

        // The gamestates have been generated, and are already sorted.
        // Now we will continue to remove the last one until we are within the size limit.
        if (inLimitNumberOfGameStates != 0)
        {
            while (outGameGrids.size() > inLimitNumberOfGameStates)
            {
                outGameGrids.erase(--outGameGrids.end());
            }
        }
    }


    void PuzzleSolver::generateFutureGameStates(GameStateNode & inGameStateNode, const Block & inBlock, size_t inColIdx, size_t inLimitNumberOfGameStates, GameStateNode::Children & outGameGrids) const
    {
        size_t maxRow = inGameStateNode.state().grid().numRows() - inBlock.grid().numRows();
        for (size_t rowIdx = 0; rowIdx <= maxRow; ++rowIdx)
        {
            if (!inGameStateNode.state().checkPositionValid(inBlock, rowIdx, inColIdx))
            {
                if (rowIdx > 0)
                {
                    // we collided => solidify on position higher
                    ChildPtr child(new GameStateNode(&inGameStateNode, inBlock, rowIdx - 1, inColIdx));
                    child->setState(inGameStateNode.state().makeGameStateWithAddedBlock(inBlock, rowIdx - 1, inColIdx));
                    if (child->state().numHoles() > 0 || (rowIdx <= 2 && child->state().hasTopHoles()))
                    {
                        child->markAsDeadEnd();
                    }
                    outGameGrids.insert(child);
                }
                return;
            }
            else if (rowIdx == maxRow)
            {
                // we found the bottom of the grid => solidify
                ChildPtr child(new GameStateNode(&inGameStateNode, inBlock, rowIdx, inColIdx));
                child->setState(inGameStateNode.state().makeGameStateWithAddedBlock(inBlock, rowIdx, inColIdx));
                if (child->state().numHoles() > 0)
                {
                    child->markAsDeadEnd();
                }
                outGameGrids.insert(child);
                return;
            }
        }
        assert(!"We should not come here");
    }


    void PuzzleSolver::getAsciiFormat(GenericGrid<char> & grid) const
    {
        getAsciiFormat(&mRootNode, grid);
    }


    void PuzzleSolver::getAsciiFormat(const GameStateNode * inNode, GenericGrid<char> & grid) const
    {
        GameStateNode::Children::const_iterator it = inNode->children().begin(), end = inNode->children().end();
        for (; it != end; ++it)
        {
            const GameStateNode * childNode = it->get();
            if (!childNode->isDeadEnd())
            {
                char id = childNode->lastBlock().charId();
                size_t row, col;
                childNode->lastBlockPosition(row, col);
                for (size_t r = row; r != row + childNode->lastBlock().grid().numRows(); ++r)
                {
                    for (size_t c = col; c != col + childNode->lastBlock().grid().numColumns(); ++c)
                    {
                        if (childNode->lastBlock().grid().get(r - row, c - col) != NO_BLOCK)
                        {
                            grid.set(r, c, id);
                        }
                    }
                }
                getAsciiFormat(childNode, grid);
                return;
            }
        }
    }

    void PuzzleSolver::getListOfMoves(std::vector<std::string> & list) const
    {
        getListOfMoves(&mRootNode, list);
    }


    void PuzzleSolver::getListOfMoves(const GameStateNode * inNode, std::vector<std::string> & list) const
    {
        GameStateNode::Children::const_iterator it = inNode->children().begin(), end = inNode->children().end();
        for (; it != end; ++it)
        {
            const GameStateNode * childNode = it->get();
            if (!childNode->isDeadEnd())
            {
                char id = childNode->lastBlock().charId();
                size_t row, col;
                childNode->lastBlockPosition(row, col);
                int rot = childNode->lastBlock().rotation();
                size_t pos = (row * 15) + col;
                //Z 0 - Piece Z at location 0 with no orientation
                //K 60 - Piece K at location 60 with no orientation.
                //B 1
                //H 18 3 Piece H at location 18 (See below) orientation 3.
                std::stringstream ss;
                ss << id << " " << pos << " - " << "Piece " << id << " at location " << pos;
                if (rot == 0)
                {
                    ss << " with no orientation.";
                }
                else
                {
                    ss << " orientation " << rot << ".";
                }
                list.push_back(ss.str());
                getListOfMoves(childNode, list);
                return;
            }
        }
    }
}