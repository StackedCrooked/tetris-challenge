#ifndef TETRISPAINTFUNCTIONS_H_INCLUDED
#define TETRISPAINTFUNCTIONS_H_INCLUDED


#include "GameState.h"
#include "BlockType.h"
#include "XULWin/Rect.h"
#include <string>


namespace Gdiplus
{
    class Brush;
    class Color;
    class Graphics;
}


namespace Tetris
{

    using XULWin::Rect;

    enum Coords
    {
        cUnitWidth    = 20,
        cUnitHeight   = 20
    };

    const Gdiplus::Color & GetColor(BlockType inType);

    void PaintUnit(Gdiplus::Graphics & g, int x, int y, BlockType inBlockType);

    void PaintGrid(Gdiplus::Graphics & g, const Grid & inGrid, int x, int y, bool inPaintBackground);

} // namespace Tetris


#endif // TETRISPAINTFUNCTIONS_H_INCLUDED
