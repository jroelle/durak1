#pragma once
#include <utility>
#include <unordered_set>
#include <memory>

namespace utility
{
	template<typename T>
	class list_element final
	{
	public:
		using holder = std::unique_ptr<T>;

		template<typename D, typename... Args>
		static holder make_holder(Args&&... args)
		{
			return std::make_unique<D>(std::forward<Args>(args)...);
		}

		template<typename D, typename... Args>
		list_element(Args&&... args)
			: _holder(make_holder<D>(std::forward<Args>(args)...))
		{}

		constexpr list_element(holder&& h)
			: _holder(std::move(h))
		{}

		constexpr T* value()
		{
			return _holder.get();
		}

		constexpr const T* value() const
		{
			return _holder.get();
		}

		constexpr list_element* next() const
		{
			return _next;
		}

	public:
		struct hash
		{
			size_t operator()(holder::pointer p) const { return reinterpret_cast<size_t>(p); }
			size_t operator()(const holder& h) const { return operator()(h.get()); }
		};
		struct equal
		{
			bool operator()(holder::pointer a, holder::pointer b) const { return a == b; }
			bool operator()(const holder& a, const holder& b) const { return operator()(a.get(), b.get()); }
			bool operator()(holder::pointer a, const holder& b) const { return operator()(a, b.get()); }
			bool operator()(const holder& a, holder::pointer b) const { return operator()(a.get(), b); }
		};

	private:
		holder _holder;
		list_element* _next = nullptr;

		friend class loop_list;
	};

	template<typename T>
	class loop_list final
	{
	public:
		using value = T;
		using element = list_element<T>;
		using storage = std::unordered_set<element, element::hash, element::equal>;

		size_t size() const noexcept
		{
			return _storage.size();
		}

		template<typename... Args>
		value* emplace(Args&&... args)
		{
			return push(element(std::forward<Args>(args)...));
		}

		value* push(element::holder&& h)
		{
			return push(element(std::move(h)));
		}

		value* push(element&& elem)
		{
			if (!elem.value())
				return nullptr;

			element* elem = _storage.insert(std::move(h)).first->get();
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
			return elem->value();
		}

		void erase(value* elem)
		{
			if (!elem)
				return;
			
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
		bool for_each(value* start, const F& callback) const
		{
			if (!start || !_root)
				return false;

			auto iter = _storage.find(start);
			if (iter == _storage.end())
				return false;

			element* elem = &*iter;
			do
			{
				if (callback(elem->value()))
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