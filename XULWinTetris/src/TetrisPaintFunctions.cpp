#include "TetrisPaintFunctions.h"
#include "Tetris/Grid.h"
#include "Tetris/GenericGrid.h"
#include "XULWin/Gdiplus.h"
#include "XULWin/RGBColor.h"
#include "XULWin/HSVColor.h"
#include "XULWin/Unicode.h"
#include <boost/lexical_cast.hpp>
#include <sstream>


namespace Tetris
{
		
	using XULWin::RGBColor;
	using XULWin::HSVColor;
	using XULWin::RGB2HSV;
	using XULWin::HSV2RGB;


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


    Gdiplus::Color GetLightColor(const Gdiplus::Color & inColor)
    {
		RGBColor rgb(inColor.GetA(), inColor.GetR(), inColor.GetG(), inColor.GetB());
		HSVColor hsv(XULWin::RGB2HSV(rgb));
		double newValue = std::min<double>(1.5 * static_cast<double>(hsv.value()), 100.0);
		HSVColor lightHSV(hsv.hue(), hsv.saturation(), static_cast<int>(0.5 + newValue));
		RGBColor lightRGB(HSV2RGB(lightHSV));
		return Gdiplus::Color(inColor.GetA(), lightRGB.red(), lightRGB.green(), lightRGB.blue());
    }


    Gdiplus::Color GetDarkColor(const Gdiplus::Color & inColor)
    {
		RGBColor rgb(inColor.GetA(), inColor.GetR(), inColor.GetG(), inColor.GetB());
		HSVColor hsv(XULWin::RGB2HSV(rgb));
		double newValue = 0.5 * static_cast<double>(hsv.value());
		HSVColor darkHSV(hsv.hue(), hsv.saturation(), static_cast<int>(0.5 + newValue));
		RGBColor darkRGB(HSV2RGB(darkHSV));
		return Gdiplus::Color(inColor.GetA(), darkRGB.red(), darkRGB.green(), darkRGB.blue());
    }


    void PaintUnit(Gdiplus::Graphics & g, int inX, int inY, BlockType inBlockType)
    {
		float x = static_cast<float>(inX);
		float y = static_cast<float>(inY);
		float width = static_cast<float>(cUnitWidth);
		float height = static_cast<float>(cUnitHeight);

		const Gdiplus::Color & color = GetColor(inBlockType);

		Gdiplus::SolidBrush solidBrush(color);
		g.FillRectangle(&solidBrush, Gdiplus::RectF(x + 1, y + 1, width - 2, height - 2));

		Gdiplus::Pen lightPen(GetLightColor(color));
		g.DrawLine(&lightPen, x, y + height - 1, x, y);
		g.DrawLine(&lightPen, x, y, x + width - 1, y);

		Gdiplus::Pen darkPen(GetDarkColor(color));
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
