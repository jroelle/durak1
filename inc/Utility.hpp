#pragma once
#include <utility>
#include <unordered_set>
#include <memory>

namespace utility
{
	template<typename T>
	class list_element final : public T
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

		friend class loop_list;
	};

	template<typename T>
	class loop_list final
	{
	public:
		using element = list_element<T>;
		using storage = std::unordered_set<holder>;

		loop_list() = default;

		~loop_list()
		{
			for (auto* elem : _storage)
				delete elem;
		}

		size_t size() const noexcept
		{
			return _storage.size();
		}

		element* push(std::unique_ptr<element>&& ptr)
		{
			return push(ptr.release());
		}

		template<typename... Args>
		element* emplace(Args&&... args)
		{
			return push(new element(std::forward<Args>(args)...));
		}

		template<typename B, typename E>
		void assign(B begin, E end)
		{
			*this = {};

			if (begin == end)
				return;

			insert_to_storage(begin, end);
			_root = *begin;
			for (auto iter = begin; iter != end; ++iter)
			{
				auto n = std::next(iter);
				if (n != end)
					iter->_next = *n;
				else
					iter->_next = _root;
			}
		}

		element* erase(element* elem)
		{
			if (!elem)
				return nullptr;
			
			if (elem == _root)
				_root = elem->next();

			auto* p = previous(elem);
			auto* n = elem->next();
			p->_next = n;

			erase_from_storage(elem);
		}

		template<typename F>
		bool for_each(element* start, const F& callback)
		{
			if (!start || !_root)
				return false;

			element* elem = start;
			do
			{
				if (callback(elem))
					return true;
				elem = elem->next();

			} while (elem != start);

			return false;
		}

		template<typename F>
		bool for_each(const F& callback)
		{
			return for_each(_root, callback);
		}

		element* previous(element* elem) const
		{
			if (!_root || !elem)
				return nullptr;

			element* iter = _root;
			while (iter->next() != elem)
				iter = iter->next();

			return iter;
		}

		element* root() const
		{
			return _root;
		}

	private:
		element* push(element* elem)
		{
			if (!elem)
				return nullptr;

			insert_to_storage(elem);
			if (_root)
			{
				auto* last = previous(_root);
				elem->_next = _root;
				last->_next = elem;
			}
			else
			{
				_root = elem;
				elem->_next = _root;
			}
		}

		void insert_to_storage(element* elem)
		{
			_storage.insert(elem);
		}

		void erase_from_storage(element* elem)
		{
			auto iter = _storage.find(elem);
			if (iter != _storage.end())
			{
				delete (*iter);
				_storage.erase(iter);
			}
		}

		template<typename B, typename E>
		void insert_to_storage(B begin, E end)
		{
			_storage.insert(begin, end);
		}

	private:
		element* _root = nullptr;
		storage _storage;
	};
}