
#pragma once

#include <assert.h>

template<typename T>
struct vec {
	T* data 	 = nullptr;
	int size 	 = 0;
	int capacity = 0;

	static vec<T> make(int capacity = 8) {
		return {new T[capacity], 0, capacity};
	}
	static vec<T> copy(vec<T> source) {
		vec<T> ret = {new T[source.capacity], source.size, source.capacity};
		memcpy(ret.data,source.data,sizeof(T)*source.size);
		return ret;
	}
	static vec<T> take(vec<T>& source) {
		vec<T> ret = source;
		source = {nullptr, 0, 0};
		return ret;
	}

	void destroy() {
		delete[] data;
		data = nullptr;
		size = capacity = 0;
	}
	
	void grow() {
		int new_capacity = capacity ? 2 * capacity : 8;
		T* new_data = new T[new_capacity];
		memcpy(new_data,data,sizeof(T)*capacity);
		capacity = new_capacity;
		data = new_data;
	}
	void clear() {
		size = 0;
	}

	bool empty() const {
		return size == 0;
	}
	bool full() const {
		return size == capacity;
	}

	T* push(T value) {
		if(full()) grow();
		assert(size < capacity);
		data[size] = value;
		return &data[size++];
	}
	T pop() {
		assert(size > 0);
		return data[size--];
	}

	T& operator[](int idx) {
		assert(idx >= 0 && idx < size);
		return data[idx];
	}
	T operator[](int idx) const {
		assert(idx >= 0 && idx < size);
		return data[idx];
	}
	T* at(int idx) const {
		assert(idx >= 0 && idx < size);
		return data + idx;
	}
	T* begin() const {
		return data;
	}
	T* end() const {
		return data + size;
	}

	struct split {
		const vec<T> l, r;
	};

	split halves() const {
		assert(size > 1);
		i32 r_s = size / 2;
		i32 l_s = r_s + size % 2;
		return {{data, l_s, l_s}, {data + l_s, r_s, r_s}};
	}
};
