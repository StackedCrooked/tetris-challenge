#ifndef TETRISPAINTFUNCTIONS_H_INCLUDED
#define TETRISPAINTFUNCTIONS_H_INCLUDED


#include "Tetris/GameState.h"
#include "Tetris/BlockType.h"
#include "XULWin/Rect.h"
#include <string>


namespace Gdiplus {
class Brush;
class Color;
class Graphics;
}


namespace Tetris {


enum Coords
{
    cUnitWidth    = 20,
    cUnitHeight   = 20
};
	
const Gdiplus::Color & GetColor(BlockType inType);
const Gdiplus::Color & GetLightColor(BlockType inType);
const Gdiplus::Color & GetDarkColor(BlockType inType);

void PaintUnit(Gdiplus::Graphics & g, int x, int y, BlockType inBlockType);

void PaintGrid(Gdiplus::Graphics & g, const Grid & inGrid, int x, int y, bool inPaintBackground);


} // namespace Tetris


#endif // TETRISPAINTFUNCTIONS_H_INCLUDED
