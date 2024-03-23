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
		using holder = std::unique_ptr<element>;

		struct hash
		{
			size_t operator()(holder::pointer p) const { return reinterpret_cast<size_t>(p); }
			size_t operator()(const holder& h) const { return operator()(h.get()); }
		};
		struct equal
		{
			bool operator()(holder::pointer a, holder::pointer b) const { return a == b;  }
			bool operator()(const holder& a, const holder& b) const { return operator()(a.get(), b.get()); }
			bool operator()(holder::pointer a, const holder& b) const { return operator()(a, b.get()); }
			bool operator()(const holder& a, holder::pointer b) const { return operator()(a.get(), b); }
		};
		using storage = std::unordered_set<holder, hash, equal>;

		template<typename... Args>
		static holder make_holder(Args&&... args)
		{
			return std::make_unique<element>(std::forward<Args>(args)...);
		}

		size_t size() const noexcept
		{
			return _storage.size();
		}

		template<typename... Args>
		element* emplace(Args&&... args)
		{
			return push(make_holder(std::forward<Args>(args)...));
		}

		element* push(std::unique_ptr<element>&& ptr)
		{
			if (!ptr)
				return nullptr;

			element* elem = _storage.insert(std::move(ptr)).first->get();
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

		element* erase(element* elem)
		{
			if (!elem)
				return nullptr;
			
			if (elem == _root)
				_root = elem->next();

			auto* p = previous(elem);
			auto* n = elem->next();
			p->_next = n;

			auto iter = _storage.find(elem);
			if (iter != _storage.end())
				_storage.erase(iter); // _storage.erase(elem) in C++23
		}

		template<typename F>
		bool for_each(element* start, const F& callback) const
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
		bool for_each(const F& callback) const
		{
			return for_each(_root, callback);
		}

		element* previous(element* elem) const
		{
			if (!_root || !elem)
				return nullptr;

			element* prev = _root;
			while (prev->next() != elem)
				prev = prev->next();

			return prev;
		}

		element* root() const
		{
			return _root;
		}

	private:
		element* _root = nullptr;
		storage _storage;
	};
}