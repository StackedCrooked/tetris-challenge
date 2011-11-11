#ifndef TYPEDWRAPPER_H
#define TYPEDWRAPPER_H


/**
 * Macro FUTILE_BOX_TYPE
 *
 * This macro generates simple class wrapper for a given type.
 * Accidental mix-ups will result in compile errors instead of
 * runtime-errors.
 *
 * Example:
 *
 *     FUTILE_BOX_TYPE(Width, int);
 *     FUTILE_BOX_TYPE(Height, int);
 *
 *     Width width = 800;
 *     Height height = 600;
 *
 *     // Signature is: 'SetSize(Width width, Height height);'
 *     SetSize(height, width); // => compiler error
 */
#define FUTILE_BOX_TYPE(ClassName, Type) \
    struct ClassName {                           \
        explicit ClassName(Type inValue) :       \
            mValue(inValue) {}                   \
        operator Type() const { return mValue; } \
        Type mValue;                             \
    } // semi-colon must be typed when calling the macro


#endif // TYPEDWRAPPER_H
