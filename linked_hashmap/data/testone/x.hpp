/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
	/**
	 * In linked_hashmap, iteration ordering is differ from map,
	 * which is the order in which keys were inserted into the map.
	 * You should maintain a doubly-linked list running through all
	 * of its entries to keep the correct iteration order.
	 *
	 * Note that insertion order is not affected if a key is re-inserted
	 * into the map.
	 */

	template<
		class Key,
		class T,
		class Hash = std::hash<Key>,
		class Equal = std::equal_to<Key>
	> class linked_hashmap {
	public:
		/**
		 * the internal type of data.
		 * it should have a default constructor, a copy constructor.
		 * You can use sjtu::linked_hashmap as value_type by typedef.
		 */
		typedef pair<const Key, T> value_type;
	private:
		struct node {
			value_type* data;
			node* next, * before, * after;//next指向映射到相同位置的下一元素 before after是指向插入顺序前后的元素用于迭代
			node() :data(nullptr), next(nullptr), before(nullptr), after(nullptr) {}
			// 引用就不需要拷贝一份数据过来 const防止修改原值
			node(const value_type& x, node* n = nullptr, node* b = nullptr, node* a = nullptr) :next(n), before(b), after(a) {
				data = (value_type*)malloc(sizeof(value_type));
				new(data) value_type(x.first, x.second);
			}
			~node() {
				if (data) {
					//对储存内容的对象手动调用析构函数
					data->first.~Key();
					data->second.~T();
					free(data);
				}
			}
		};

		class BucketList {
		public:
			node* head;

			BucketList() {
				head = nullptr;
			}
			~BucketList() = default;
			void insert(node* n) {
				n->next = head;
				head = n;
			}
			node* erase(const Key& key) {
				if (!head) return nullptr;
				node* p = head;
				if (Equal()(head->data->first, key)) {
					head = p->next;
					return p;
				}
				while (p->next && !Equal()(p->next->data->first, key)) {
					p = p->next;
				}
				if (p->next) {
					node* tmp = p->next;
					p->next = tmp->next;
					return tmp;
				}
				return nullptr;
			}
			//const函数 不修改成员状态或者调用非常函数
			node* find(const Key& k) const {
				node* p = head;
				while (p && !Equal()(k, p->data->first))p = p->next;
				return p;
			}

		};
		BucketList* cont;
		static constexpr float LOAD_FACTOR = 0.75;
		size_t capacity;
		size_t len;
		int capp;
		node* head, * tail;//迭代用的将元素按照插入顺序储存的双链表 的两个哨兵

		//五个素数作为哈希表的容量 选素数碰撞概率小
		static constexpr size_t mod[5] = { 1009,10007,100003,1000003,10000019 };
		void resize() {
			if (capp == 4)return;
			capp++;
			capacity = mod[capp];
			//删除给cont分配的一对bucketlist的内存，node没有删除
			delete[]cont;
			cont = new BucketList[capacity];
			for (node* p = head->after; p != tail; p = p->after) {
				cont[Hash()(p->data->first) % capacity].insert(p);
			}

		}

	public:
		/**
		 * see BidirectionalIterator at CppReference for help.
		 *
		 * if there is anything wrong throw invalid_iterator.
		 *     like it = linked_hashmap.begin(); --it;
		 *       or it = linked_hashmap.end(); ++end();
		 */


		class const_iterator;
		class iterator {
			friend class linked_hashmap;
		private:
			/**
			 * TODO add data members
			 *   just add whatever you want.
			 */
			linked_hashmap* f;
			node* ptr;
		public:
			// The following code is written for the C++ type_traits library.
			// Type traits is a C++ feature for describing certain properties of a type.
			// For instance, for an iterator, iterator::value_type is the type that the 
			// iterator points to. 
			// STL algorithms and containers may use these type_traits (e.g. the following 
			// typedef) to work properly. 
			// See these websites for more information:
			// https://en.cppreference.com/w/cpp/header/type_traits
			// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
			// About iterator_category: https://en.cppreference.com/w/cpp/iterator
			using difference_type = std::ptrdiff_t;
			using value_type = typename linked_hashmap::value_type;
			using pointer = value_type*;
			using reference = value_type&;
			using iterator_category = std::output_iterator_tag;


			iterator(linked_hashmap* ff = nullptr, node* pp = nullptr) {
				// TODO
				f = ff;
				ptr = pp;
			}
			iterator(const iterator& other) {
				// TODO
				f = other.f;
				ptr = other.ptr;
			}
			/**
			 * TODO iter++
			 */
			 //返回值是自增之前
			iterator operator++(int) {
				if (ptr == f->tail)throw invalid_iterator();
				node* pre = ptr;
				ptr = ptr->after;
				return iterator(f, pre);
			}
			/**
			 * TODO ++iter
			 */
			 //返回自增后
			iterator& operator++() {
				if (ptr == f->tail)throw invalid_iterator();
				ptr = ptr->after;
				//这好像是解引用
				return *this;
			}
			/**
			 * TODO iter--
			 */
			iterator operator--(int) {
				if (ptr == f->head->after)throw invalid_iterator();
				node* pre = ptr;
				ptr = ptr->before;
				return iterator(f, ptr);
			}
			/**
			 * TODO --iter
			 */
			iterator& operator--() {
				if (ptr == f->head->after)throw invalid_iterator();
				ptr = ptr->before;
				return *this;
			}
			/**
			 * a operator to check whether two iterators are same (pointing to the same memory).
			 */
			value_type& operator*() const {
				if (!ptr->data)throw invalid_iterator();
				//记得解引用
				return *(this->ptr->data);
			}
			bool operator==(const iterator& rhs) const {
				return f == rhs.f && ptr == rhs.ptr;
			}
			bool operator==(const const_iterator& rhs) const {
				return f == rhs.f && ptr == rhs.ptr;
			}
			/**
			 * some other operator for iterator.
			 */
			bool operator!=(const iterator& rhs) const {
				return f != rhs.f || ptr != rhs.ptr;
			}
			bool operator!=(const const_iterator& rhs) const {
				return f != rhs.f || ptr != rhs.ptr;
			}

			/**
			 * for the support of it->first.
			 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
			 */
			value_type* operator->() const noexcept {
				return ptr->data;
			}
		};

		class const_iterator {
			// it should has similar member method as iterator.
			//  and it should be able to construct from an iterator.
			friend class linked_hashmap;
		private:
			// data members.
			const linked_hashmap* f;
			node* ptr;
		public:
			const_iterator(const linked_hashmap* _ptr = nullptr, node* _pos = nullptr) :f(_ptr), ptr(_pos) {
				// TODO
			}
			const_iterator(const const_iterator& other) :f(other.f), ptr(other.ptr) {
				// TODO
			}
			const_iterator(const iterator& other) :f(other.f), ptr(other.ptr) {
				// TODO
			}
			/**
			 * TODO iter++
			 */
			const_iterator operator++(int) {
				if (ptr == f->tail) throw invalid_iterator();
				node* p = ptr;
				ptr = ptr->after;
				return const_iterator(f, p);
			}
			/**
			 * TODO ++iter
			 */
			const_iterator& operator++() {
				if (ptr == f->tail) throw invalid_iterator();
				ptr = ptr->after;
				return *this;
			}
			/**
			 * TODO iter--
			 */
			const_iterator operator--(int) {
				if (ptr == f->head->after) throw invalid_iterator();
				node* p = ptr;
				ptr = ptr->before;
				return const_iterator(f, p);
			}
			/**
			 * TODO --iter
			 */
			const_iterator& operator--() {
				if (ptr == f->head->after) throw invalid_iterator();
				ptr = ptr->before;
				return *this;
			}
			/**
			 * a operator to check whether two iterators are same (pointing to the same memory).
			 */
			 //第一个const表示返回值不可被修改 体现const iter 第二个const表示这个函数不会修改自己
			const value_type& operator*() const {
				if (!ptr->data) throw invalid_iterator();
				return *(ptr->data);
			}
			bool operator==(const iterator& rhs) const {
				return ptr == rhs.ptr && f == rhs.f;
			}
			bool operator==(const const_iterator& rhs) const {
				return ptr == rhs.ptr && f == rhs.f;
			}
			/**
			 * some other operator for iterator.
			 */
			bool operator!=(const iterator& rhs) const {
				return ptr != rhs.ptr || f != rhs.f;
			}
			bool operator!=(const const_iterator& rhs) const {
				return ptr != rhs.ptr || f != rhs.f;
			}

			/**
			 * for the support of it->first.
			 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
			 */
			const value_type* operator->() const noexcept {
				return ptr->data;
			}
		};

		/**
		 * TODO two constructors
		 */
		linked_hashmap() {
			len = 0;
			capp = 0;
			capacity = mod[capp];
			head = new node();
			tail = new node();
			head->after = tail;
			tail->before = head;
			cont = new BucketList[capacity];
		}
		linked_hashmap(const linked_hashmap& other) {
			capp = other.capp;
			capacity = other.capacity;
			len = other.len;
			cont = new BucketList[capacity];
			head = new node();
			tail = new node();
			node* cur;
			node* p;
			for (p = other.head->after, cur = this->head; p != other.tail; p = p->after, cur = cur->after) {
				size_t hashcode = Hash()(p->data->first) % capacity;
				cur->after = new node(*(p->data), nullptr, cur, nullptr);
				cont[hashcode].insert(cur->after);
			}
			cur->after = tail;
			tail->before = cur;
		}

		/**
		 * TODO assignment operator
		 */
		linked_hashmap& operator=(const linked_hashmap& other) {
			if (&other == this)return *this;
			clear();
			delete[]cont;
			capacity = other.capacity;
			capp = other.capp;
			len = other.len;
			cont = new BucketList[capacity];
			node* p;
			node* q;
			p = other.head->after, q = head;
			while (p != other.tail) {
	
				size_t hash = Hash()(p->data->first) % capacity;
				q->after = new node(*(p->data), cont[hash].head, q);
				cont[hash].head = q = q->after;
				p = p->after;
			}
			q->after = tail;
			tail->before = q;
			return *this;
		}

		/**
		 * TODO Destructors
		 */
		~linked_hashmap() {
			delete[]cont;
			node* p = head->after;
			node* q;
			while (p != tail) {
				q = p;
				p = p->after;
				delete q;
			}
			delete head;
			delete tail;
		}

		/**
		 * TODO
		 * access specified element with bounds checking
		 * Returns a reference to the mapped value of the element with key equivalent to key.
		 * If no such element exists, an exception of type `index_out_of_bound'
		 */
		T& at(const Key& key) {
			size_t hashcode = Hash()(key) % capacity;
			node* p = cont[hashcode].find(key);
			if (p) return p->data->second;
			throw index_out_of_bound();
		}
		const T& at(const Key& key) const {
			size_t hashcode = Hash()(key) % capacity;
			node* p = cont[hashcode].find(key);
			if (p) return p->data->second;
			throw index_out_of_bound();
		}

		/**
		 * TODO
		 * access specified element
		 * Returns a reference to the value that is mapped to a key equivalent to key,
		 *   performing an insertion if such key does not already exist.
		 */
		T& operator[](const Key& key) {
			node* p = cont[Hash()(key) % capacity].find(key);
			if (p) {
				return p->data->second;
			}
			else {
				//这里使用了iterator重载的-> 插入了一个默认值
				return insert(value_type(key, T())).first->second;
			}
		}

		/**
		 * behave like at() throw index_out_of_bound if such key does not exist.
		 */
		const T& operator[](const Key& key) const {
			node* p = cont[Hash()(key) % capacity].find(key);
			if (p) {
				return p->data->second;
			}
			else {
				//这里使用了iterator重载的-> 插入了一个默认值
				throw index_out_of_bound();
			}
		}

		/**
		 * return a iterator to the beginning
		 */
		iterator begin() {
			return iterator(this, head->after);
		}
		const_iterator cbegin() const {
			return const_iterator(this, head->after);
		}

		/**
		 * return a iterator to the end
		 * in fact, it returns past-the-end.
		 */
		iterator end() {
			return iterator(this, tail);
		}
		const_iterator cend() const {
			return const_iterator(this, tail);
		}

		/**
		 * checks whether the container is empty
		 * return true if empty, otherwise false.
		 */
		bool empty() const {
			return head->after == tail;
		}

		/**
		 * returns the number of elements.
		 */
		size_t size() const {
			return len;
		}

		/**
		 * clears the contents
		 */
		void clear() {
			
			node* p = head->after;
			node* q;
			while (p != tail) {
				q = p;
				p = p->after;
				delete q;
			}
			delete []cont;

			head->after = tail;
			tail->before = head;
			len = 0;
			capp=0;
            capacity=mod[capp];
            cont=new BucketList[capacity];
		}

		/**
		 * insert an element.
		 * return a pair, the first of the pair is
		 *   the iterator to the new element (or the element that prevented the insertion),
		 *   the second one is true if insert successfully, or false.
		 */
		pair<iterator, bool> insert(const value_type& value) {
			Key key = value.first;
			T val = value.second;
			size_t hashcode = Hash()(key);
			node* p = cont[hashcode% capacity].find(key);
			if (p) {
				return pair<iterator, bool>(iterator(this, p), false);
			}
				len++;
				if (len > capacity * LOAD_FACTOR && capp < 4) resize();
				p = new node(value);
				cont[hashcode % capacity].insert(p);
				p->before = tail->before;
				p->after = tail;
				tail->before->after = p;
				tail->before = p;
				return pair<iterator, bool>(iterator(this, p), true);
			
		}

		/**
		 * erase the element at pos.
		 *
		 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
		 */
		 //用.来访问迭代器的正常成员 用->来访问迭代器代表的值
		void erase(iterator pos) {
			if (pos.f != this || pos == end()) { throw invalid_iterator(); }
			pos.ptr->before->after = pos.ptr->after;
			pos.ptr->after->before = pos.ptr->before;
			delete cont[Hash()(pos->first) % capacity].erase(pos->first);
			len--;
		}

		/**
		 * Returns the number of elements with key
		 *   that compares equivalent to the specified argument,
		 *   which is either 1 or 0
		 *     since this container does not allow duplicates.
		 */
		size_t count(const Key& key) const {
			node* p = cont[Hash()(key) % capacity].find(key);
			if (p) {
				return 1;
			}
			else {
				return 0;
			}
		}

		/**
		 * Finds an element with key equivalent to key.
		 * key value of the element to search for.
		 * Iterator to an element with key equivalent to key.
		 *   If no such element is found, past-the-end (see end()) iterator is returned.
		 */
		iterator find(const Key& key) {
			node* p = cont[Hash()(key) % capacity].find(key);
			if (p)return iterator(this, p);
			else return end();
		}
		const_iterator find(const Key& key) const {
			node* p = cont[Hash()(key) % capacity].find(key);
			if (p)return const_iterator(this, p);
			else return cend();
		}
	};

}

#endif
