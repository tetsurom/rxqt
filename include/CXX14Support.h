#ifndef CXX14SUPPORT_h__
#define CXX14SUPPORT_h__
//Idea from https://github.com/taocpp/sequences The Art of C++ / Sequences 
#include <tuple>
#include <utility>

namespace std
{
	template<class _Ty,
		_Ty... _Vals>
	struct integer_sequence
	{	
		static_assert(is_integral<_Ty>::value,
			"integer_sequence<T, I...> requires T to be an integral type.");

		typedef _Ty value_type;

		static size_t size() _NOEXCEPT
		{	
			return (sizeof...(_Vals));
		}
	};

	namespace impl
	{
		template< bool V, bool E >
		struct generate_sequence;

		template<>
		struct generate_sequence< false, true >
		{
			template< typename T, T M, T N, std::size_t S, T... Ns >
			using f = integer_sequence< T, Ns... >;
		};

		template<>
		struct generate_sequence< true, true >
		{
			template< typename T, T M, T N, std::size_t S, T... Ns >
			using f = integer_sequence< T, Ns..., S >;
		};

		template<>
		struct generate_sequence< false, false >
		{
			template< typename T, T M, T N, std::size_t S, T... Ns >
			using f = typename generate_sequence< (N & (M / 2)) != 0, (M / 2) == 0 >::template f< T, M / 2, N, 2 * S, Ns..., (Ns + S)... >;
		};

		template<>
		struct generate_sequence< true, false >
		{
			template< typename T, T M, T N, std::size_t S, T... Ns >
			using f = typename generate_sequence< (N & (M / 2)) != 0, (M / 2) == 0 >::template f< T, M / 2, N, 2 * S + 1, Ns..., (Ns + S)..., 2 * S >;
		};

		template< typename T, T N >
		struct memoize_sequence
		{
			static_assert(N < T(1 << 20), "N too large");
			using type = typename generate_sequence< false, false >::template f < T, (N < T(1 << 1)) ? T(1 << 1) : (N < T(1 << 2)) ? T(1 << 2) : (N < T(1 << 3)) ? T(1 << 3) : (N < T(1 << 4)) ? T(1 << 4) : (N < T(1 << 5)) ? T(1 << 5) : (N < T(1 << 6)) ? T(1 << 6) : (N < T(1 << 7)) ? T(1 << 7) : (N < T(1 << 8)) ? T(1 << 8) : (N < T(1 << 9)) ? T(1 << 9) : (N < T(1 << 10)) ? T(1 << 10) : T(1 << 20), N, 0 >;
		};
	}

	template< typename T, T N >
	using make_integer_sequence = typename impl::memoize_sequence< T, N >::type;

	template<size_t... _Vals>
	using index_sequence = integer_sequence<size_t, _Vals...>;

	template<size_t _Size>
	using make_index_sequence = make_integer_sequence<size_t, _Size>;

	template<size_t _Index,
	class _Tuple>
		using tuple_element_t = typename tuple_element<_Index, _Tuple>::type;
}
#endif // CXX14SUPPORT_h__