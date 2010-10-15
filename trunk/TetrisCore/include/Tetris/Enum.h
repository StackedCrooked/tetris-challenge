#ifndef TETRIS_ENUM_H_INCLUDED
#define TETRIS_ENUM_H_INCLUDED


/**
 * DefineEnum can be used to define a (fake) enum.
 * For example:
 *
 *     DefineEnum(Color)
 *     {
 *       Red,
 *       Green,
 *       Blue
 *     };
 *
 *
 * DeclareEnum can be used to create a forward declaration.
 * For example:
 *
 *    DeclareEnum(Color);
 */
#define DefineEnum(name) typedef int name; enum
#define DeclareEnum(name) typedef int name


#endif // TETRIS_ENUM_H_INCLUDED
