#ifndef TETRIS_TYPEDWRAPPER_H_INCLUDED
#define TETRIS_TYPEDWRAPPER_H_INCLUDED


/**
 * Macro Tetris_TypedWrapper
 *
 * This macro generates simple class wrapper for a given type.
 * Accidental mix-ups will result in compile errors instead of
 * runtime-errors.
 *
 * Example:
 *
 *     Tetris_TypedWrapper(Width, int);
 *     Tetris_TypedWrapper(Height, int);
 *
 *     Width width = 800;
 *     Height height = 600;
 *
 *     // Signature is: 'SetSize(Width width, Height height);'
 *     SetSize(height, width); // => compiler error
 */
#define Tetris_TypedWrapper(ClassName, WrappedType)                           \
    class ClassName                                                           \
    {                                                                         \
    public:                                                                   \
        explicit ClassName(WrappedType inValue) : mValue(inValue) {}          \
        WrappedType get() const                                               \
        { return mValue; }                                                    \
        operator WrappedType() const                                          \
        { return mValue; }                                                    \
    private:                                                                  \
        WrappedType mValue;                                                   \
    } // semi-colon must be typed when calling the macro


#endif // TYPEDWRAPPER_H_INCLUDED
