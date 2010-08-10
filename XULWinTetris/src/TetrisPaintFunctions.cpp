#include "TetrisPaintFunctions.h"
#include "XULWin/Gdiplus.h"
#include "XULWin/Unicode.h"
#include <boost/lexical_cast.hpp>
#include <sstream>


namespace Tetris
{


    Gdiplus::RectF ToRectF(const Rect & inRect)
    {
        return Gdiplus::RectF(static_cast<float>(inRect.x()),
                              static_cast<float>(inRect.y()),
                              static_cast<float>(inRect.width()),
                              static_cast<float>(inRect.height()));
    }


    const Gdiplus::Color & GetColor(BlockType inBlockType)
    {
        if (inBlockType < BlockType_Nil || inBlockType >= BlockType_End)
        {
            throw std::logic_error("Invalid BlockType enum value.");
        }

        static const Gdiplus::Color fColors[] =
        {
            Gdiplus::Color(Gdiplus::Color::DarkGray),       // Background
            Gdiplus::Color(Gdiplus::Color::Cyan),           // I-Shape
            Gdiplus::Color(Gdiplus::Color::Blue),           // J-Shape
            Gdiplus::Color(Gdiplus::Color::Orange),         // L-Shape
            Gdiplus::Color(Gdiplus::Color::Yellow),         // O-Shape
            Gdiplus::Color(Gdiplus::Color::LightGreen),     // S-Shape
            Gdiplus::Color(Gdiplus::Color::Purple),         // T-Shape
            Gdiplus::Color(Gdiplus::Color::Red)             // Z-Shape
        };

        return fColors[static_cast<int>(inBlockType)];
    }


    void PaintUnit(Gdiplus::Graphics & g, int x, int y, BlockType inBlockType)
    {
        Gdiplus::SolidBrush solidBrush(GetColor(inBlockType));
        g.FillRectangle(
            &solidBrush,
            Gdiplus::RectF(
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(cUnitWidth),
                static_cast<float>(cUnitHeight)));
    }


    void PaintGrid(Gdiplus::Graphics & g, const Grid & inGrid, int x, int y)
    {
        for (size_t r = 0; r != inGrid.numRows(); ++r)
        {
            for (size_t c = 0; c != inGrid.numColumns(); ++c)
            {
                if (inGrid.get(r, c))
                {
                    PaintUnit(g, x + c * cUnitWidth, y + r * cUnitHeight, inGrid.get(r, c));
                }
            }
        }
    }
    
    
    //void PaintFutureBlocks(Gdiplus::Graphics & g)
    //{
    //    std::vector<BlockType> blocks;
    //    mGame->getFutureBlocks(5, blocks);

    //    for (size_t i = 1; i < cNumFutureBlocks; ++i)
    //    {
    //        const Grid & grid = GetGrid(GetBlockIdentifier(blocks[i], 0));
    //        paintGrid(g, grid, cNextBlocksX, cNextBlocksY + (i - 1) * 5 * cUnitHeight);
    //    }
    //}


    //void PaintText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Rect & inRect)
    //{
    //    Gdiplus::SolidBrush textBrush(Gdiplus::Color::Green);
    //    PaintText(inGraphics, inText, inRect, textBrush);
    //}


    //void PaintText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Rect & inRect, const Gdiplus::Brush & inBrush)
    //{
    //    Gdiplus::Font gdiFont(TEXT("Arial"), 9, Gdiplus::FontStyleRegular);

    //    Gdiplus::StringFormat stringFormat;
    //    inGraphics.DrawString(inText.c_str(), (int)inText.size(), &gdiFont, ToRectF(inRect), &stringFormat, &inBrush);
    //}


    //std::wstring GetScoreText(const std::string & inTitle, int inScore)
    //{
    //    std::stringstream ss;
    //    ss << inTitle << ": " << inScore;
    //    return XULWin::ToUTF16(ss.str());
    //}


    //void PaintScores(Gdiplus::Graphics & inGraphics, const GameState::Stats & inStats)
    //{
    //    PaintText(inGraphics,
    //              GetScoreText("Lines", inStats.numLines()),
    //              Rect(cScoresX, cScoresY_0, cScoresWidth, cScoresHeight));

    //    PaintText(inGraphics, GetScoreText("x1", inStats.numSingles()), Rect(cScoresX, cScoresY_1, cScoresWidth, cScoresHeight));
    //    PaintText(inGraphics, GetScoreText("x2", inStats.numDoubles()), Rect(cScoresX, cScoresY_2, cScoresWidth, cScoresHeight));
    //    PaintText(inGraphics, GetScoreText("x3", inStats.numTriples()), Rect(cScoresX, cScoresY_3, cScoresWidth, cScoresHeight));
    //    PaintText(inGraphics, GetScoreText("x4", inStats.numTetrises()), Rect(cScoresX, cScoresY_4, cScoresWidth, cScoresHeight));
    //}

    



} // namespace Tetris
