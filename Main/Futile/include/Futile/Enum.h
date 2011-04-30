#ifndef FUTILE_ENUM_H
#define FUTILE_ENUM_H


#include <boost/lexical_cast.hpp>
#include <boost/preprocessor.hpp>
#include <stdexcept>
#include <iostream>


namespace Futile {


/**
 * EnumInfo<EnumType> provides useful meta data beloning to an enum type.
 *
 * Details:
 *   - name() returns the enumeration's tag name.
 *   - values() returns an array containing all enumerator values.
 *   - names() returns an array containing all enumerator names.
 *   - size() returns the number of enumerators (this is the same as the length of the values and names arrays)
 *   - first() returns the first enumerator.
 *   - last() returns the last enumerator.
 *   - FromString(const std::string &) returns the enumerator value for the given string value.
 *       An exception is thrown if the string was not found in the list of enumerator names.
 *   - ToString(const std::string &) returns the enumerator name for a given enumerator value.
 *       A std::runtime_error is thrown if the enumerator value was not found.
 */
template<typename T>
struct EnumInfo {};


/**
 * EnumeratorInfo<EnumType, Enumerator> provides useful metadata for the requested enumerator value.
 *
 * Details:
 *   - name() returns the Enumerator name
 *   - value() returns the Enumerator value
 */
template<class TEnum, TEnum TEnumerator>
struct EnumeratorInfo {};


/**
 * The ENUM macro allows you to generate a new enumeration and automatically generate
 * the accompanying code for parsing and serializing the enumerator values.
 *
 * Usage example:
 *
 *    // Define the enum like this:
 *    Futile_Enum(HTTPRequestMethod, 9, (HEAD, GET, POST, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH))
 *
 *    // Example for getting meta data
 *    EnumInfo<HTTPRequestMethod>::name() // returns "HTTPRequestMethod"
 *    EnumumeratorInfo<HTTPRequestMethod, HEAD>::name() // returns "HEAD"
 *
 * LIMITATIONS:
 *    - The enumerators always start at value zero and increment in units of 1.
 *      Negative enumerator values or interval gaps are not supported.
 *
 *    - The string representation are as specified in the macro call.
 *      Specifying a differernt string representation is not supported.
 *
 * See the documentation for the EnumInfo and EnumeratorInfo classes for more information.
 *
 */
#define Futile_Enum(Tag, Size, Values)                                                    \
    Futile_DefineEnum(Tag, Size, Values)                                                  \
    Futile_DefineEnumInfoSpecialization(                                                  \
        Tag,                                                                              \
        Size,                                                                             \
        Values,                                                                           \
        BOOST_PP_LIST_FIRST(BOOST_PP_TUPLE_TO_LIST(Size, Values)),                        \
        BOOST_PP_LIST_FIRST(BOOST_PP_LIST_REVERSE(BOOST_PP_TUPLE_TO_LIST(Size, Values)))) \
    BOOST_PP_LIST_FOR_EACH(                                                               \
        Futile_DefineEnumeratorInfoSpecialization,                                        \
        Tag,                                                                              \
        BOOST_PP_TUPLE_TO_LIST(Size, Values))


//
// Macros for internal use
//
#define Futile_DefineEnum(Tag, Size, Enumerators) \
    enum Tag { BOOST_PP_LIST_ENUM(BOOST_PP_TUPLE_TO_LIST(Size, Enumerators)) };


#define Futile_ToString(Dummy0, Dummy1, Element) \
    BOOST_PP_STRINGIZE(Element)


#define Futile_DefineEnumInfoSpecialization(Tag, Size, Enumerator, First, Last)                   \
    template<> struct EnumInfo<Tag>                                                               \
    {                                                                                             \
        static const char * name() { return #Tag; }                                               \
        typedef const Tag Values[Size];                                                           \
        static const Values & values();                                                           \
        typedef const char * Names[Size];                                                         \
        static const Names & names();                                                             \
        static int size() { return Size; }                                                        \
        static Tag first() { return First; }                                                      \
        static Tag last() { return Last; }                                                        \
        static Tag FromString(const std::string & inName) {                                       \
            for (std::size_t idx = 0; idx < size(); ++idx) {                                      \
                const Names & theNames = names();                                                 \
                const Values & theValues = values();                                              \
                if (theNames[idx] == inName) return theValues[idx];                               \
            }                                                                                     \
            throw std::runtime_error("Invalid enumerator name: " + inName);                       \
        }                                                                                         \
        static const char * ToString(Tag inValue) {                                               \
            for (std::size_t idx = 0; idx < size(); ++idx) {                                      \
                const Names & theNames = names();                                                 \
                const Values & theValues = values();                                              \
                if (theValues[idx] == inValue) return theNames[idx];                              \
            }                                                                                     \
            throw std::runtime_error(                                                             \
                    "Invalid enumerator value: " +                                                \
                    boost::lexical_cast<std::string>(inValue));                                   \
        }                                                                                         \
    };                                                                                            \
    inline const EnumInfo<Tag>::Values & EnumInfo<Tag>::values() {                                \
        static Values fValues = { BOOST_PP_LIST_ENUM(BOOST_PP_TUPLE_TO_LIST(Size, Enumerator)) }; \
        return fValues;                                                                           \
    };                                                                                            \
    inline const EnumInfo<Tag>::Names & EnumInfo<Tag>::names() {                                  \
        static Names fNames = {                                                                   \
            BOOST_PP_LIST_ENUM(                                                                   \
                BOOST_PP_LIST_TRANSFORM(                                                          \
                    Futile_ToString,                                                              \
                    Size,                                                                         \
                    BOOST_PP_TUPLE_TO_LIST(Size, Enumerator))) };                                 \
        return fNames;                                                                            \
    };


#define Futile_DefineEnumeratorInfoSpecialization(Dummy, Enum, Enumerator) \
    template<> struct EnumeratorInfo<Enum, Enumerator> {                   \
        typedef Enum EnumType;                                             \
        static const char * name() { return #Enumerator; }                 \
        static Enum value() { return Enumerator; }                         \
    };


} // namespace Futile


#endif // FUTILE_ENUM_H
