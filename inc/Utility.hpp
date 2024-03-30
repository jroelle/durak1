#pragma once
#include <utility>
#include <unordered_set>
#include <memory>

namespace utility
{
	template<typename T, typename H = std::hash<T>>
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

			size_t operator()(size_t h) const { return h; }
			size_t operator()(const T& v) const { return H{}(v); }
			size_t operator()(const list_element& e) const { return H{}(*e.value()); }
		};
		struct transparent_equal
		{
			using is_transparent = void;

			bool operator()(size_t a, size_t b) const { return a == b; }
			bool operator()(size_t a, const T& b) const { return operator()(a, transparent_hash{}(b)); }
			bool operator()(size_t a, const list_element& b) const { return operator()(a, transparent_hash{}(b)); }
			bool operator()(const T& a, size_t b) const { return operator()(transparent_hash{}(a), b); }
			bool operator()(const T& a, const T& b) const { return operator()(transparent_hash{}(a), transparent_hash{}(b)); }
			bool operator()(const T& a, const list_element& b) const { return operator()(a, *b.value()); }
			bool operator()(const list_element& a, size_t b) const { return operator()(transparent_hash{}(a), b); }
			bool operator()(const list_element& a, const T& b) const { return operator()(*a.value(), b); }
			bool operator()(const list_element& a, const list_element& b) const { return operator()(*a.value(), *b.value()); }
		};

	private:
		holder _holder = {};
		list_element* _next = nullptr;

		template<typename T, typename H> friend class loop_list;
	};

	template<typename T, typename H = std::hash<T>>
	class loop_list final
	{
	public:
		using element = list_element<T, H>;
		using storage = std::unordered_set<element, typename element::transparent_hash, typename element::transparent_equal>;

		size_t size() const noexcept
		{
			return _storage.size();
		}

		template<typename... Args>
		T* emplace_back(Args&&... args)
		{
			return push(element(std::forward<Args>(args)...), false);
		}

		template<typename... Args>
		T* emplace_front(Args&&... args)
		{
			return push(element(std::forward<Args>(args)...), true);
		}

		T* push_back(element::holder&& h)
		{
			return push(element(std::move(h)), false);
		}

		T* push_front(element::holder&& h)
		{
			return push(element(std::move(h)), true);
		}

		element::holder remove(const T* v)
		{
			element* elem = remove_element(v);
			typename element::holder holder;
			std::swap(elem->_holder, holder);
			_storage.erase(*elem);
			return holder;
		}

		void erase(const T* v)
		{
			element* elem = remove_element(v);
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
			auto iter = _storage.find(H{}(key));
			return iter != _storage.end() ? const_cast<T*>(iter->value()) : nullptr;
		}

		bool contains(const T* v) const
		{
			return v && _storage.contains(*v);
		}

		template<typename K>
		bool contains(const K& key) const
		{
			return _storage.contains(H{}(key));
		}

	private:
		T* push(element&& tmp, bool front)
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

				if (front)
					_root = elem;
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

		element* remove_element(const T* v)
		{
			if (!_root)
				return nullptr;

			element* elem = find(v);
			if (!elem)
				return nullptr;

			if (H{}(*elem->value()) == H{}(*_root->value()))
				_root = elem->next();

			previous(elem)->_next = elem->next();
			return elem;
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