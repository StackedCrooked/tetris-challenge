#ifndef FUTILE_ENUM_H
#define FUTILE_ENUM_H


#include <boost/lexical_cast.hpp>
#include <boost/preprocessor.hpp>
#include <stdexcept>
#include <iostream>


/**
 * EnumInfo<EnumType> provides metadata associated with an enum type. All methods are static.
 *
 * Overview:
 *
 * const char * name();
 * Returns the string representation of the enum name.
 *
 * Values values();
 * Returns an array that stores a copy of all enum value.
 * This enables you to iterate over the enum values using a loop.
 *
 * Names names();
 * Returns an array that contains a string representations of the enum values.
 *
 * int size();
 * Returns the size of the arrays returned by the values() and names() methods.
 *
 * EnumType first();
 * Returns the first enum value in the enum.
 *
 * EnumType last();
 * Returns the last value in the enum.
 *
 * EnumType FromString(const std::string &);
 * Returns the enum value that matches the given string.
 *
 * std::string ToString(EnumType);
 * Returns the string representation of the provided enum value.
 * Throws a std::invalid_argument if the enum value is invalid.
 */
#define Futile_EnumInfo \
    template<typename Enum_> struct EnumInfo;


/**
 * EnumValueInfo<EnumType, EnumValue> provides useful metadata for the requested enumerator value.
 *
 * Overview:
 * - name() returns the EnumValue name
 * - value() returns the EnumValue value
 */
#define Futile_EnumValueInfo \
    template<class Enum_, Enum_ EnumValue_> struct EnumValueInfo;


/**
 * The ENUM macro allows you to generate a new enumeration and automatically generate
 * the accompanying code for parsing and serializing the enumerator values.
 *
 * Use the "Futile_Enum" macro to generate the code:
 *   Futile_Enum(HTTPRequestMethod, 9, (HEAD, GET, POST, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH))
 *
 * The enum will be genereated like this:
 *   enum HTTPRequestMethod { HEAD, GET, POST, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH };
 *
 * The macro also generates a specialization of the EnumInfo and EnumValueInfo template
 * classes. Documentation for these classes can be found at the Futile_EnumInfo and
 * Futile_EnumValueInfo macros.
 *
 *
 *   // - EnumValueInfo<HTTPRequestMethod> { ... };
 *   // These classes provide metadata that can be accessed through static methods.

 *   EnumInfo<HTTPRequestMethod>::name() // returns "HTTPRequestMethod"
 *   EnumValueInfo<HTTPRequestMethod, HEAD>::name() // returns "HEAD"
 *
 * Limitations:
 * - The enumerators always start at value zero and increment in units of 1.
 *   Negative enumerator values or interval gaps are not supported.
 *
 * - The string representation are as specified in the macro call.
 *   Specifying a differernt string representation is not supported.
 *
 */
#define Futile_Enum(EnumType, Size, Values)                                                       \
    Futile_EnumInfo                                                                               \
    Futile_EnumValueInfo                                                                          \
    Futile_DefineEnum(EnumType, Size, Values)                                                     \
    Futile_DefineEnumInfoSpecialization(                                                          \
        EnumType,                                                                                 \
        Size,                                                                                     \
        Values,                                                                                   \
        BOOST_PP_LIST_FIRST(BOOST_PP_TUPLE_TO_LIST(Size, Values)),                                \
        BOOST_PP_LIST_FIRST(BOOST_PP_LIST_REVERSE(BOOST_PP_TUPLE_TO_LIST(Size, Values))))         \
    BOOST_PP_LIST_FOR_EACH(                                                                       \
        Futile_DefineEnumValueInfoSpecialization,                                                 \
        EnumType,                                                                                 \
        BOOST_PP_TUPLE_TO_LIST(Size, Values))


//
// Helper macros. Private.
//
#define Futile_DefineEnum(EnumType, Size, EnumValues)                                             \
    enum EnumType                                                                                 \
    {                                                                                             \
        BOOST_PP_LIST_ENUM(BOOST_PP_TUPLE_TO_LIST(Size, EnumValues))                              \
    };


#define Futile_ToString(Dummy0, Dummy1, Element)                                                  \
    BOOST_PP_STRINGIZE(Element)


#define Futile_DefineEnumInfoSpecialization(EnumType_, Size_, EnumValues_, First_, Last_)         \
    template<> struct EnumInfo<EnumType_>                                                         \
    {                                                                                             \
        enum                                                                                      \
        {                                                                                         \
            Size  = Size_,                                                                        \
            First = First_,                                                                       \
            Last  = Last_                                                                         \
        };                                                                                        \
                                                                                                  \
        typedef EnumType_ EnumType;                                                               \
        static const EnumType cFirst = First_;                                                    \
        static const EnumType cLast = Last_;                                                      \
                                                                                                  \
        static const char * name() { return #EnumType_; }                                         \
                                                                                                  \
        typedef const EnumType Values[Size];                                                      \
        static const Values & values();                                                           \
                                                                                                  \
        typedef const char * Names[Size];                                                         \
        static const Names & names();                                                             \
                                                                                                  \
        static int size() { return Size; }                                                        \
                                                                                                  \
        static EnumType first() { return cFirst; }                                                \
                                                                                                  \
        static EnumType last() { return cLast; }                                                  \
                                                                                                  \
        static EnumType FromString(const std::string & inName)                                    \
        {                                                                                         \
            for (int idx = 0; idx < size(); ++idx)                                                \
            {                                                                                     \
                const Names & theNames = names();                                                 \
                const Values & theValues = values();                                              \
                if (theNames[idx] == inName) return theValues[idx];                               \
            }                                                                                     \
            throw std::runtime_error("Invalid enumerator name: " + inName);                       \
        }                                                                                         \
                                                                                                  \
        static const char * ToString(EnumType inValue)                                            \
        {                                                                                         \
            for (int idx = 0; idx < size(); ++idx)                                                \
            {                                                                                     \
                const Names & theNames = names();                                                 \
                const Values & theValues = values();                                              \
                if (theValues[idx] == inValue) return theNames[idx];                              \
            }                                                                                     \
            throw std::invalid_argument(boost::lexical_cast<std::string>(inValue));               \
        }                                                                                         \
    };                                                                                            \
    inline const EnumInfo<EnumType_>::Values & EnumInfo<EnumType_>::values()                      \
    {                                                                                             \
        static Values fValues = {                                                                 \
            BOOST_PP_LIST_ENUM(                                                                   \
                    BOOST_PP_TUPLE_TO_LIST(Size_, EnumValues_))                                   \
        };                                                                                        \
        return fValues;                                                                           \
    }                                                                                             \
    inline const EnumInfo<EnumType_>::Names & EnumInfo<EnumType_>::names()                        \
    {                                                                                             \
        static Names fNames =                                                                     \
        {                                                                                         \
            BOOST_PP_LIST_ENUM(                                                                   \
                BOOST_PP_LIST_TRANSFORM(                                                          \
                    Futile_ToString,                                                              \
                    Size,                                                                         \
                    BOOST_PP_TUPLE_TO_LIST(Size_, EnumValues_)))                                  \
        };                                                                                        \
        return fNames;                                                                            \
    }


#define Futile_DefineEnumValueInfoSpecialization(Dummy, Enum, EnumValue)                          \
    template<> struct EnumValueInfo<Enum, EnumValue>                                              \
    {                                                                                             \
        typedef Enum EnumType;                                                                    \
                                                                                                  \
        static const char * name() { return #EnumValue; }                                         \
                                                                                                  \
        static Enum value() { return EnumValue; }                                                 \
    };


#endif // FUTILE_ENUM_H
