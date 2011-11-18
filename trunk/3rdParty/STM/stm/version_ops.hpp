#ifndef H_VERSION_OPS_63946EE58AA5C746BC6DB570FCC4ACDB
#define H_VERSION_OPS_63946EE58AA5C746BC6DB570FCC4ACDB

#include "utility.hpp"

namespace stm {
	namespace detail {
		template <typename V>
		struct basic_ver_ops {
			struct scoped_access_version {
				scoped_access_version(){}
				~scoped_access_version(){}
			};

			static V version_from_tx(V hdr) throw() {
				return hdr;	
			}

			static V zero() throw() { return V(); }

			static V version_from_shared(V hdr) throw() {
				return hdr >> 1;
			}
			static bool hdr_neq(V lhs, V rhs) throw() {
				return lhs != rhs;
			}
			static bool ver_neq(V hdr, V ver) throw() {
				return version_from_shared(hdr) != ver;
			}
			static V plus_one(V ver) throw() {
				return ver + 1;
			}
			static slot_offset_t active_offset(V hdr) throw() {
				return hdr & static_cast<V>(1);
			}
			static slot_offset_t inactive_offset(V hdr) throw() {
				return (~hdr) & static_cast<V>(1);
			}
			static V set_version_and_flip(V cur_hdr, V tx_ver) throw() {
				return (tx_ver << 1) | inactive_offset(cur_hdr);
			}
			static bool shared_valid_in_tx(V shared_hdr, V tx_ver) throw() {
				auto lhs = version_from_shared(shared_hdr);
				auto rhs = version_from_tx(tx_ver);
				return lhs <= rhs;
			}

			static slot_offset_t reader_registered_on(V shared_hdr, V tx_ver) throw() {			
				return (shared_valid_in_tx(shared_hdr, tx_ver)) ? active_offset(shared_hdr) : inactive_offset(shared_hdr);
			}

			static V to_version(unsigned long long ver){
				return static_cast<V>(ver);
			}
			static unsigned long long from_version(V ver) {
				return static_cast<unsigned long long>(ver);
			}
		};

#ifndef STM_X64
STM_BEGIN_SILENCE_MMX_WARNING
		template <>
		struct basic_ver_ops<__m64> {
			struct scoped_access_version {
				~scoped_access_version() {
					_mm_empty();
				}
			};

			static __m64 version_from_tx(__m64 hdr) throw() {
				return hdr;	
			}

			static __m64 zero() throw() { return _mm_setzero_si64(); }

			static __m64 version_from_shared(__m64 hdr) throw() {
				return _mm_srli_si64(hdr, 1);
			}
			static bool hdr_neq(__m64 lhs, __m64 rhs) throw() {
				return _mm_cvtsi64_si32(_mm_srli_si64(_mm_cmpeq_pi32(lhs, rhs), 16)) != 0xffffffff;
			}
			static bool ver_neq(__m64 hdr, __m64 ver) throw() {
				return hdr_neq(version_from_shared(hdr), ver);
			}
			static __m64 plus_one(__m64 ver) throw() {
				return _mm_add_si64(_mm_cvtsi32_si64(1), ver);
			}

			static slot_offset_t active_offset(__m64 hdr) throw() {
				return _mm_cvtsi64_si32(hdr) & static_cast<slot_offset_t>(1);
			}
			static slot_offset_t inactive_offset(__m64 hdr) throw() {
				return (~_mm_cvtsi64_si32(hdr)) & static_cast<slot_offset_t>(1);
			}
			static __m64 set_version_and_flip(__m64 cur_hdr, __m64 tx_ver) throw() {
				// todo: might some room for optimization here. Flipping flag via inactive_offset seems clumsy
				__m64 new_flag = _mm_cvtsi32_si64(inactive_offset(cur_hdr));
				//__m64 new_flag = _mm_srli_si64(_mm_slli_si64(plus_one(cur_hdr), 63), 63); // might be faster
				return _mm_or_si64(_mm_slli_si64(tx_ver, 1), new_flag);
			}

			// return shared version <= tx_ver
			static bool shared_valid_in_tx(__m64 shared_hdr, __m64 tx_ver) throw() {
				// mmx only has 32-bit > (we need 64-bit <=)
				// a <= b can be expressed as
				// !(a > b)
				// !(a[1] > b[1] || (a[1] == b[1] && a[0] > b[0]))

				// remove flag, and then offset numbers so signed < works correctly
				__m64 shared_ver = version_from_shared(shared_hdr);
				__m64 shared_signed = _mm_add_pi32(_mm_cvtsi32_si64(1 << 31), shared_ver);
				__m64 tx_signed = _mm_add_pi32(_mm_cvtsi32_si64(1 << 31), version_from_tx(tx_ver));

				__m64 gt = _mm_cmpgt_pi32(shared_signed, tx_signed);
				// lower half of eq contains upper half of a==b
				__m64 eq = _mm_srli_si64(_mm_cmpeq_pi32(shared_signed, tx_signed), 32);
				
				// lower half of lhs contains upper half of a>b
				__m64 lhs = _mm_srli_si64(gt, 32);
				__m64 rhs = _mm_and_si64(eq, gt);
				__m64 res = _mm_or_si64(lhs, rhs);				
				return _mm_cvtsi64_si32(res) == 0;
			}

			static slot_offset_t reader_registered_on(__m64 shared_hdr, __m64 tx_ver) throw() {
				return (shared_valid_in_tx(shared_hdr, tx_ver)) ? active_offset(shared_hdr) : inactive_offset(shared_hdr);
			}

			static __m64 to_version(unsigned long long ver){
				return *reinterpret_cast<__m64*>(&ver);

			}
			static unsigned long long from_version(__m64 ver) {
				return *reinterpret_cast<unsigned long long*>(&ver);
			}
		};

		STM_END_SILENCE_MMX_WARNING
#endif

			typedef basic_ver_ops<version_field_t>::scoped_access_version scoped_access_version;
	}
	typedef detail::basic_ver_ops<version_field_t> ver_ops;
}
#endif
