#include "TetrisPaintFunctions.h"
#include "Tetris/Grid.h"
#include "Futile/GenericGrid.h"
#include "XULWin/Gdiplus.h"
#include "XULWin/RGBColor.h"
#include "XULWin/HSVColor.h"
#include "XULWin/Unicode.h"
#include <boost/lexical_cast.hpp>
#include <sstream>


namespace Tetris {

		
using XULWin::RGBColor;
using XULWin::HSVColor;
using XULWin::RGB2HSV;
using XULWin::HSV2RGB;
using XULWin::Rect;


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


const Gdiplus::Color & GetLightColor(BlockType inBlockType)
{
    if (inBlockType < BlockType_Nil || inBlockType >= BlockType_End)
    {
        throw std::logic_error("Invalid BlockType enum value.");
    }

    static const Gdiplus::Color fColors[] =
    {
        Gdiplus::Color(Gdiplus::Color::White),          // Background
        Gdiplus::Color(127, 255, 255),                  // I-Shape
        Gdiplus::Color(127, 127, 255),                  // J-Shape
        Gdiplus::Color(255, 210, 127),                  // L-Shape
        Gdiplus::Color(255, 255, 127),                  // O-Shape
        Gdiplus::Color(127, 255, 127),                  // S-Shape
        Gdiplus::Color(211, 139, 255),                  // T-Shape
        Gdiplus::Color(255, 127, 127)                   // Z-Shape
    };

    return fColors[static_cast<int>(inBlockType)];
}


const Gdiplus::Color & GetDarkColor(BlockType inBlockType)
{
    if (inBlockType < BlockType_Nil || inBlockType >= BlockType_End)
    {
        throw std::logic_error("Invalid BlockType enum value.");
    }

    static const Gdiplus::Color fColors[] =
    {
        Gdiplus::Color(Gdiplus::Color::White),          // Background
        Gdiplus::Color(0, 127, 127),                    // I-Shape
        Gdiplus::Color(0, 0, 127),                      // J-Shape
        Gdiplus::Color(127, 82, 0),                     // L-Shape
        Gdiplus::Color(127, 127, 0),                    // O-Shape
        Gdiplus::Color(0, 127, 0),                      // S-Shape
        Gdiplus::Color(80, 16, 120),                    // T-Shape
        Gdiplus::Color(127, 0, 0)                       // Z-Shape
    };

    return fColors[static_cast<int>(inBlockType)];
}


void PaintUnit(Gdiplus::Graphics & g, int inX, int inY, BlockType inBlockType)
{
	float x = static_cast<float>(inX);
	float y = static_cast<float>(inY);
	float width = static_cast<float>(cUnitWidth);
	float height = static_cast<float>(cUnitHeight);
		
	Gdiplus::SolidBrush solidBrush(GetColor(inBlockType));
	g.FillRectangle(&solidBrush, Gdiplus::RectF(x + 1, y + 1, width - 2, height - 2));

	Gdiplus::Pen lightPen(GetLightColor(inBlockType));
	g.DrawLine(&lightPen, x, y + height - 1, x, y);
	g.DrawLine(&lightPen, x, y, x + width - 1, y);

	Gdiplus::Pen darkPen(GetDarkColor(inBlockType));
	g.DrawLine(&darkPen, x + 1, y + height - 1, x + width - 1, y + height - 1);
	g.DrawLine(&darkPen, x + width - 1, y + height - 1, x + width - 1, y + 1);
}


void PaintGrid(Gdiplus::Graphics & g, const Grid & inGrid, int inX, int inY, bool inPaintBackground)
{
	if (inPaintBackground)
	{
		Gdiplus::SolidBrush solidBrush(GetColor(BlockType_Nil));
		float x = static_cast<float>(inX);
		float y = static_cast<float>(inY);
		float w = static_cast<float>(inGrid.columnCount() * cUnitWidth);
		float h = static_cast<float>(inGrid.rowCount() * cUnitHeight);
		g.FillRectangle(&solidBrush, Gdiplus::RectF(x, y, w, h));
	}

    for (size_t r = 0; r != inGrid.rowCount(); ++r)
    {
        for (size_t c = 0; c != inGrid.columnCount(); ++c)
        {
            if (inGrid.get(r, c))
            {
                PaintUnit(g, inX + c * cUnitWidth, inY + r * cUnitHeight, inGrid.get(r, c));
            }
        }
    }
}


} // namespace Tetris
