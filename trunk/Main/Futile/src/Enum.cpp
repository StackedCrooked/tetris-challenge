#include "Futile/Enum.h"


namespace Futile {


//#define Futile_DefineEnumInfoSpecialization(Tag, Size, Enumerator, First, Last) \
//    EnumInfo<Tag>::Values EnumInfo<Tag>::values = \
//        { BOOST_PP_LIST_ENUM(BOOST_PP_TUPLE_TO_LIST(Size, Enumerator)) }; \
//    EnumInfo<Tag>::Names EnumInfo<Tag>::names = \
//        { BOOST_PP_LIST_ENUM(BOOST_PP_LIST_TRANSFORM(Futile_ToString, Size, BOOST_PP_TUPLE_TO_LIST(Size, Enumerator))) };


} // namespace Futile
