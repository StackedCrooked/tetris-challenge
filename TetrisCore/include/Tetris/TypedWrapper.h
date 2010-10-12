#ifndef TETRIS_TYPEDWRAPPER_H_INCLUDED
#define TETRIS_TYPEDWRAPPER_H_INCLUDED


/**
 * Macro GENERATE_TYPESAFE_WRAPPER
 *
 * This macro can be used to create different typenames
 * for different values of the same primitive type. This
 * provides type-safety.
 *
 * See Block.h for a few examples.
 */
#define GENERATE_TYPESAFE_WRAPPER(PrimitiveType, ClassName) \
    class ClassName \
    { \
    public: \
        explicit ClassName(PrimitiveType inValue) : mValue(inValue) {}        \
        PrimitiveType get() const                                             \
        { return mValue; }                                                    \
        operator PrimitiveType() const                                        \
        { return mValue; }                                                    \
    private:                                                                  \
        PrimitiveType mValue;                                                 \
    };


#endif // TYPEDWRAPPER_H_INCLUDED
