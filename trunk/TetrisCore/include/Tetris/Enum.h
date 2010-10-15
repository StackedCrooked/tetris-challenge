#ifndef TETRIS_ENUM_H_INCLUDED
#define TETRIS_ENUM_H_INCLUDED


/**
 * Tetris_DefineEnum can be used to define a fake enum. The purpose of this macro
 * is to enable enums that can be forward declared.
 *
 * Example:
 *
 *     Tetris_DefineEnum(Color)
 *     {
 *       Color_Red,   // Color_* naming style compensates a bit for the loss of type-safety.
 *       Color_Green,
 *       Color_Blue
 *     };
 *
 *     Color c = Color_Blue;
 *
 */
#define Tetris_DefineEnum(name) typedef int name; enum


/*
 * Tetris_DeclareEnum can be used to create a forward declaration of a fake enum.
 *
 * Example:
 *
 *    Tetris_DeclareEnum(Color);
 */
#define Tetris_DeclareEnum(name) typedef int name


#endif // TETRIS_ENUM_H_INCLUDED
