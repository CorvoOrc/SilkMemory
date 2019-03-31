#pragma once

namespace silk_data {
	const int START_QUEUE_CAPACITY = 8;
	const int DEFAULT_MAX_QUEUE_CAPACITY = 1024;

	template <typename T>
	class Queue {
	public:
		Queue(int _maxCapacity = DEFAULT_MAX_QUEUE_CAPACITY) {
			capacity = START_QUEUE_CAPACITY;
			items = new T[capacity];
			head = tail = 0;

			maxCapacity = _maxCapacity;
			maxCapacityReached = false;
		}
		~Queue() {
			delete[] items;
			head = tail = 0;
			capacity = 0;
			maxCapacity = 0;
			maxCapacityReached = false;
		}
		bool Enqueue(T& item) {
			if (!CheckEnqueue())
				return false;
			items[tail] = item;
			tail = (tail + 1) % capacity;
			maxCapacityReached = head == tail;
			return true;
		}
		T& Dequeue() {
			T& removedItem = items[head];
			head = (head + 1) % capacity;
			maxCapacityReached = false;
			return removedItem;
		}
		T& Peek() {
			return items[head];
		}
		void ToArray(T* array) {
			int len = Length();
			for (int i = 0; i < len; i++)
				array[i] = items[(head + i) % capacity];
		}
		int Length() {
			if (maxCapacityReached)
				return capacity;
			bool overflow = tail < head;
			return overflow ? capacity - (head - tail) : tail - head;
		}
		int Capacity() {
			return capacity;
		}
		int MaxCapacity() {
			return maxCapacity;
		}
	private:
		bool CheckEnqueue() {
			int len = Length();
			if (len == maxCapacity)
				return false;
			if (len < capacity)
				return true;

			T* newItems = new T[capacity * 2];
			for (int i = 0; i < len; i++)
				newItems[i] = items[(head + i) % capacity];

			delete[] items;
			items = newItems;
			head = 0;
			tail = len;
			capacity *= 2;
			return true;
		}
	private:
		T* items;
		int head;
		int tail;

		int capacity;
		int maxCapacity;
		bool maxCapacityReached;
	};

}