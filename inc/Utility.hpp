#pragma once
#include <utility>
#include <unordered_set>
#include <memory>

namespace utility
{
	template<typename T, typename H = std::hash<T>, typename E = std::equal_to<T>>
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
		struct transparent_hash
		{
			using is_transparent = void;

			size_t operator()(const T& p) const { return H{}(p); }
			size_t operator()(const list_element& e) const { return operator()(*e.value()); }
		};
		struct transparent_equal
		{
			using is_transparent = void;

			bool operator()(const T& a, const T& b) const { return E{}(a, b); }
			bool operator()(const list_element& a, const list_element& b) const { return operator()(*a.value(), *b.value()); }
			bool operator()(const T& a, const list_element& b) const { return operator()(a, *b.value()); }
			bool operator()(const list_element& a, const T& b) const { return operator()(*a.value(), b); }
		};

	private:
		holder _holder = {};
		list_element* _next = nullptr;

		template<typename T, typename H, typename E> friend class loop_list;
	};

	template<typename T, typename H = std::hash<T>, typename E = std::equal_to<T>>
	class loop_list final
	{
	public:
		using element = list_element<T, H, E>;
		using storage = std::unordered_set<element, typename element::transparent_hash, typename element::transparent_equal>;

		size_t size() const noexcept
		{
			return _storage.size();
		}

		template<typename... Args>
		T* emplace(Args&&... args)
		{
			return push(element(std::forward<Args>(args)...));
		}

		T* push(element::holder&& h)
		{
			return push(element(std::move(h)));
		}

		void erase(const T* v)
		{
			if (!_root)
				return;

			const element* elem = find(v);
			if (!elem)
				return;
			
			if (H{}(*elem->value()) == H{}(*_root->value()))
				_root = elem->next();

			previous(elem)->_next = elem->next();
			_storage.erase(*elem); // _storage.erase(*v) in C++23
		}

		template<typename F>
		bool for_each(const T* start, const F& callback) const
		{
			return for_each(find(start), callback);
		}

		template<typename F>
		bool for_each(const F& callback) const
		{
			return for_each(_root, callback);
		}

		T* next(const T* v) const
		{
			element* elem = find(v);
			return elem ? elem->next()->value() : nullptr;
		}

		T* previous(const T* v) const
		{
			element* elem = previous(find(v));
			return elem ? elem->value() : nullptr;
		}

		T* root() const
		{
			return _root ? _root->value() : nullptr;
		}

		template<typename K>
		T* find(const K& key) const
		{
			auto iter = _storage.find(key);
			return iter != _storage.end() ? iter->value() : nullptr;
		}

	private:
		T* push(element&& tmp)
		{
			if (!tmp.value())
				return nullptr;

			auto iter = _storage.insert(std::move(tmp)).first;
			element* elem = const_cast<element*>(&*iter);
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

		element* find(const T* v) const
		{
			const auto iter = v ? _storage.find(*v) : _storage.end();
			return iter != _storage.end() ? const_cast<element*>(&*iter) : nullptr;
		}

		element* next(const element* elem) const
		{
			return elem ? elem->next() : nullptr;
		}

		element* previous(const element* elem) const
		{
			if (!_root || !elem)
				return nullptr;

			element* prev = _root;
			while (prev->next() != elem)
				prev = prev->next();

			return prev;
		}

		template<typename F>
		bool for_each(element* start, const F& callback) const
		{
			if (!start || !_root)
				return false;

			element* elem = start;
			do
			{
				if (callback(elem->value()))
					return true;
				elem = elem->next();

			} while (elem != start);

			return false;
		}

	private:
		element* _root = nullptr;
		storage _storage;
	};
}