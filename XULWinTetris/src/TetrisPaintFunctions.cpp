#include "TetrisPaintFunctions.h"
#include "Tetris/Grid.h"
#include "Tetris/GenericGrid.h"
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
            Gdiplus::Color(Gdiplus::Color::White),          // Background
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


    void PaintGrid(Gdiplus::Graphics & g, const Grid & inGrid, int x, int y, bool inPaintBackground)
    {
        for (size_t r = 0; r != inGrid.numRows(); ++r)
        {
            for (size_t c = 0; c != inGrid.numColumns(); ++c)
            {
                if (inPaintBackground || inGrid.get(r, c))
                {
                    PaintUnit(g, x + c * cUnitWidth, y + r * cUnitHeight, inGrid.get(r, c));
                }
            }
        }
    }

} // namespace Tetris
