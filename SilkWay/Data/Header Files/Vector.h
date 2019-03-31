#pragma once

namespace silk_data {
	const int START_VECTOR_CAPACITY = 8;
	const int MAX_VECTOR_CAPACITY = 1024;

	template<typename T>
	class Vector {
	public:
		Vector(int _maxCapacity = MAX_VECTOR_CAPACITY) {
			capacity = START_VECTOR_CAPACITY;
			items = new T[capacity];
			maxCapacity = _maxCapacity;
			len = 0;
		}
		~Vector() {
			delete[] items;
			len = 0;
		}
		void Clear() {
			int length = Length();
			for (int i = 0; i < length; i++)
				delete items[i];
			len = 0;
		}
		void Insert(T& item, int pos) {
			if (!CheckInsert(pos))
				return;
			for (int i = pos; i < len - 1; i++)
				items[i + 1] = items[i];
			items[pos] = item;
			len++;
		}
		bool Erase(int pos) {
			for (int i = pos; i < len - 1; i++)
				items[i] = items[i + 1];
			items[len - 1] = 0;
			len--;
			return true;
		}
		bool Erase(T& item) {
			for (int i = 0; i < len; i++)
				if (item == items[i]) 
					return Erase(i);
			return false;
		}
		void PushBack(T& item) {
			Insert(item, len);
		}
		void PopBack() {
			Erase(len - 1);
		}
		void ToArray(T* array) {
			for (int i = 0; i < Length(); i++)
				array[i] = items[i];
		}
		T& GetItem(int pos) {
			return items[pos];
		}
		int Length() {
			return len;
		}
		int Capacity() {
			return capacity;
		}
		int MaxCapacity() {
			return maxCapacity;
		}
	private:
		bool CheckInsert(int pos) {
			if (pos >= maxCapacity)
				return false;
			if (pos < capacity)
				return true;
			int newCapacity = capacity * 2;
			while (pos > newCapacity)
				newCapacity *= 2;
			T* newItems = new T[newCapacity];
			for (int i = 0; i < capacity; i++)
				newItems[i] = items[i];
			delete items;
			items = newItems;
			capacity = newCapacity;
			return true;
		}
	private:
		T* items;
		int len;
		int capacity;
		int maxCapacity;
	};
}