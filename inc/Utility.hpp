#pragma once
#include <vector>

namespace utility
{
	template<typename T>
	class loop_vector
	{
	public:
		using vector_type = std::vector<T>;

		using iterator = vector_type::iterator;
		using const_iterator = vector_type::const_iterator;
		using reverse_iterator = vector_type::reverse_iterator;
		using const_reverse_iterator = vector_type::const_reverse_iterator;

		constexpr iterator begin() { return _vector.begin(); }
		constexpr const_iterator begin() const { return _vector.begin(); }
		constexpr reverse_iterator rbegin() { return _vector.rbegin(); }
		constexpr const_reverse_iterator rbegin() const { return _vector.rbegin(); }

		constexpr iterator end() { return _vector.end(); }
		constexpr const_iterator end() const { return _vector.end(); }
		constexpr reverse_iterator rend() { return _vector.rend(); }
		constexpr const_reverse_iterator rend() const { return _vector.rend(); }

		constexpr iterator next(iterator i) const { return next<iterator>(i); }
		constexpr const_iterator next(const_iterator i) const { return next<const_iterator>(i); }

		constexpr iterator prev(iterator i) const { return prev<iterator>(i); }
		constexpr const_iterator prev(const_iterator i) const { return prev<const_iterator>(i); }

		constexpr vector_type& vector() { return _vector; }
		constexpr const vector_type& vector() const { return _vector; }

	private:
		template<typename I>
		constexpr I next(I i) const
		{
			if (i == _vector.end())
				return _vector.begin();
			else
				return ++i;
		}

		template<typename I>
		constexpr I prev(I i) const
		{
			if (i == _vector.rend())
				return _vector.rbegin();
			else
				return --i;
		}

	private:
		vector_type _vector;
	};
}