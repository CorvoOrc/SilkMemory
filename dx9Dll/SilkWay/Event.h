#pragma once
#define VIRTUAL_EVENT(e) public: virtual IEvent* Get##e() = 0;
#define EVENT(e) private: IEvent* e; public: IEvent* Get##e() { return e; }	

const int MAX_EVENT_CALLBACKS = 1024;

class IEventArgs {
};

class ICallback {
public:
	virtual void Invoke(IEventArgs* args) = 0;
};

template<class A>
class Callback : public ICallback {
	typedef void(*f)(A*);
public:
	Callback(f _pFunc) {
		ptr = _pFunc;
	}
	~Callback() {
		delete ptr;
	}
	void Invoke(IEventArgs* args) {
		ptr((A*)args);
	}
private:
	f ptr = nullptr;
};

template<typename T, class A>
class MemberCallback : public ICallback {
	typedef void (T::*f)(A*);
public:
	MemberCallback(f _pFunc, T* _obj) {
		ptr = _pFunc;
		obj = _obj;
	}
	~MemberCallback() {
		delete ptr;
		obj = nullptr;
	}
	void Invoke(IEventArgs* args) {
		(obj->*(ptr))((A*)args);
	}
private:
	f ptr = nullptr;
	T* obj;
};

class IEvent {
public:
	virtual void Invoke(IEventArgs* args) = 0;
	virtual void Add(ICallback* callback) = 0;
	virtual bool Remove(ICallback* callback) = 0;
	virtual ~IEvent() {}
};

class Event : public IEvent {
public:
	Event() {
		index = 0;
		callbacks = new ICallback*[MAX_EVENT_CALLBACKS];
	}
	void Invoke(IEventArgs* args) {
		for (int i = 0; i < index; i++)
			callbacks[i]->Invoke(args);
	}
	void Add(ICallback* callback) {
		callbacks[index++] = callback;
	}
	bool Remove(ICallback* callback) {
		for (int i = 0; i < index; i++) {
			if (callback == callbacks[i]) {
				Shift(&i);
				index--;
				return true;
			}
		}
		return false;
	}
	~Event() {
		delete[] callbacks;
	}
private:
	void Shift(int* start) {
		for (int i = *start; i < index; ++i)
			callbacks[i] = callbacks[i + 1];
	}
private:
	ICallback** callbacks;
	int index;
};
