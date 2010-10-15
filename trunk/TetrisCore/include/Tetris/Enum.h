#ifndef TETRIS_ENUM_H_INCLUDED
#define TETRIS_ENUM_H_INCLUDED


/**
 * DefineEnum can be used to define a fake enum. The purpose of this macro
 * is to enable enums that can be forward declared.
 *
 * Example:
 *
 *     DefineEnum(Color)
 *     {
 *       Color_Red,   // Color_* naming style compensates a bit for the loss of type-safety.
 *       Color_Green,
 *       Color_Blue
 *     };
 *
 *     Color c = Color_Blue;
 *
 */
#define DefineEnum(name) typedef int name; enum


/*
 * DeclareEnum can be used to create a forward declaration of a fake enum.
 *
 * Example:
 *
 *    DeclareEnum(Color);
 */
#define DeclareEnum(name) typedef int name


#endif // TETRIS_ENUM_H_INCLUDED
