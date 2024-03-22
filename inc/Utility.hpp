#pragma once
#include <utility>

namespace utility
{
	template<typename T>
	class list_element final
	{
	public:
		template<typename... Args>
		list_element(Args&&... args)
			: _value(std::forward<Args>(args)...)
		{}

		constexpr list_element(const T& value)
			: _value(value)
		{}

		constexpr list_element(T&& value)
			: _value(std::move(value))
		{}

		constexpr T& value()
		{
			return _value;
		}

		constexpr const T& value() const
		{
			return _value;
		}

		constexpr list_element* next() const
		{
			return _next;
		}

	private:
		T _value;
		list_element* _next = nullptr;
	};

	template<typename T>
	class loop final
	{
	public:
		using element = list_element<T>;

		element* push(T&& value)
		{
			return push(element(std::forward<T>(value)...));
		}

		template<typename... Args>
		element* emplace(Args&&... args)
		{
			return push(element(std::forward<Args>(args)...));
		}

		element* erase(element* elem)
		{
			if (!elem)
				return nullptr;

			if (elem == _root)
				_root = elem->next();
			

		}

		template<typename F>
		bool for_each(element* start, const F& callback)
		{
			for (element* elem = start; elem != start && elem->next() != start; elem = elem->next())
			{

			}
		}

	private:
		element* push(element* elem)
		{

		}

	private:
		element* _root = nullptr;
	};
}