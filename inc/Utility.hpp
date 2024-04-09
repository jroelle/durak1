#pragma once
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <list>
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

	template<typename KeyT, typename ValueT, typename H = std::hash<KeyT>>
	class mapped_list final
	{
	public:
		using storage = std::list<ValueT>;
		using keys = std::unordered_map<KeyT, typename storage::iterator, H>;

		size_t size() const
		{
			return _storage.size();
		}

		bool empty() const
		{
			return _storage.empty();
		}

		ValueT& at(const KeyT& key)
		{
			return *_keys.at(key);
		}

		const ValueT& at(const KeyT& key) const
		{
			return *_keys.at(key);
		}

		template<typename... Args>
		ValueT& emplace_back(KeyT&& key, Args&&... args)
		{
			ValueT& elem = _storage.emplace_back(std::forward<Args>(args)...);
			add_key(std::move(key), std::prev(_storage.end()));
			return elem;
		}

		template<typename... Args>
		ValueT& emplace_front(KeyT&& key, Args&&... args)
		{
			ValueT& elem = _storage.emplace_front(std::forward<Args>(args)...);
			add_key(std::move(key), _storage.begin());
			return elem;
		}

		void push_back(const KeyT& key, const ValueT& value)
		{
			_storage.push_back(value);
			add_key(key, std::prev(_storage.end()));
		}

		void push_back(const KeyT& key, ValueT&& value)
		{
			_storage.push_back(std::move(value));
			add_key(key, std::prev(_storage.end()));
		}

		void push_front(const KeyT& key, const ValueT& value)
		{
			_storage.push_front(value);
			add_key(key, _storage.begin());
		}

		void push_front(const KeyT& key, ValueT&& value)
		{
			_storage.push_front(std::move(value));
			add_key(key, _storage.begin());
		}

		ValueT remove(const KeyT& key)
		{
			const auto key_iter = _keys.find(key);
			const auto storage_iter = key_iter->second;
			ValueT value = std::move(*storage_iter);
			_storage.erase(storage_iter);
			_keys.erase(key_iter);
			return value;
		}

		void erase(const KeyT& key)
		{
			const auto key_iter = _keys.find(key);
			const auto storage_iter = key_iter->second;
			_storage.erase(storage_iter);
			_keys.erase(key_iter);
		}

		template<typename F>
		bool for_each(const F& callback)
		{
			for (auto& elem : _storage)
			{
				if (callback(elem))
					return true;
			}
			return false;
		}

		template<typename F>
		bool for_each(const F& callback) const
		{
			for (const auto& elem : _storage)
			{
				if (callback(elem))
					return true;
			}
			return false;
		}

		storage::iterator begin()
		{
			return _storage.begin();
		}

		storage::iterator end()
		{
			return _storage.begin();
		}

		storage::const_iterator begin() const
		{
			return _storage.begin();
		}

		storage::const_iterator end() const
		{
			return _storage.begin();
		}

	private:
		void add_key(const KeyT& key, storage::iterator iter)
		{
			_keys.emplace(key, iter);
		}

		void add_key(KeyT&& key, storage::iterator iter)
		{
			_keys.emplace(std::move(key), iter);
		}

	private:
		storage _storage;
		keys _keys;
	};
}