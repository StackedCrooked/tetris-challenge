#ifndef TETRIS_TYPEDWRAPPER_H_INCLUDED
#define TETRIS_TYPEDWRAPPER_H_INCLUDED


/**
 * Macro TypedWrapper
 *
 * This macro generates simple class wrapper for a given type.
 * Accidental mix-ups will result in compile errors instead of 
 * runtime-errors.
 *
 * Example:
 *
 *     TypedWrapper(Width, int);
 *     TypedWrapper(Height, int);
 *
 *     Width w = 800;
 *     Width h = 600;
 *
 *     // Signature is: 'SetSize(Width inWidth, Height height);'
 *     SetSize(h, w); // => compiler error
 */
#define TypedWrapper(ClassName, WrappedType)                                  \
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
    } // semi-colon must be typed by the user.


#endif // TYPEDWRAPPER_H_INCLUDED
