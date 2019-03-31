#pragma once
#include <cstdio>
#include <cstring>
#include "Event.h"
#include "Vector.h"
#include "Queue.h"
#include "RBTree.h"

#define foreach(finish) for (int i = 0; i < finish; i++)

#define GET(T, m) T* Get##m() { return m; };
#define SET(T, m) void Set##m(T* p) { m = p; };
#define VIRTUAL_GET(T, m) virtual T* Get##m() = 0;
#define VIRTUAL_SET(T, m) virtual void Set##m(T* p) = 0;
#define ACCESSOR(T, m) protected: T*##m; public: GET(T, m) SET(T, m)
#define IMMUTABLE_ACCESSOR(T, m) protected: T*##m; public: T* Get##m() const { return m; };
#define REACTIVE_ACCESSOR(T, m) protected: T*##m; public: GET(T, m) void Set##m(T* p) { m = p; m##Changed->Invoke(nullptr); }; \
	EVENT(m##Changed)
#define ARRAY_ACCESSOR(T, m, size) protected: T* ##m = new T[size]; public: GET(T, m) SET(T, m)
#define VIRTUAL_ACCESSOR(T, m) public: VIRTUAL_GET(T, m) VIRTUAL_SET(T, m)
#define VIRTUAL_COMPONENT(T, m) VIRTUAL_ACCESSOR(T, m)
#define VIRTUAL_REACTIVE_COMPONENT(T, m) VIRTUAL_ACCESSOR(T, m) VIRTUAL_EVENT(m##Changed)
#define COMPONENT(T, m) ACCESSOR(T, m)
#define REACTIVE_COMPONENT(T, m) REACTIVE_ACCESSOR(T, m)
#define ARRAY_COMPONENT(T, m, size) ARRAY_ACCESSOR(T, m, size)

#define ISILK_WAY_OBJECT ISilkWayObject
#define ISILK_PRIMITIVE ISilkPrimitive
#define ISILK_PRIMITIVE_ARRAY ISilkPrimitiveArray
#define SILK_OBJ(child) class child: public ISILK_WAY_OBJECT
#define SILK_PRIMITIVE(child, T) class child: public ISILK_PRIMITIVE <T> { public: child(const T value) : ISILK_PRIMITIVE(value) {}};
#define SILK_COLLECTION_BEGIN(child, T, size) SILK_OBJ(child) { \
		IMMUTABLE_ACCESSOR(T, Value) \
	public:child(const T* value) { Value = new T[size](); SetRaw(value); } \
		~child() { delete[] Value; } 
#define SILK_COLLECTION_END };

const int MAX_COLLECTION_SIZE = 1024;
const int MAX_STRING_SIZE = 64;

class ISILK_WAY_OBJECT {
public:
	virtual ~ISILK_WAY_OBJECT() { };
};

template <typename T>
SILK_OBJ(ISILK_PRIMITIVE) {
	IMMUTABLE_ACCESSOR(T, Value)
public:
	ISILK_PRIMITIVE(const T value) {
		Value = new T();
		SetRaw(value);
	}
	~ISILK_PRIMITIVE() {
		delete Value;
	}
	virtual void SetRaw(const T value) {
		*Value = value;
	}
};

SILK_PRIMITIVE(SILK_INT, int)
SILK_PRIMITIVE(SILK_BOOL, bool)
SILK_PRIMITIVE(SILK_LL, long long)
SILK_PRIMITIVE(SILK_FLOAT, float)

SILK_COLLECTION_BEGIN(SILK_STRING, char, MAX_STRING_SIZE)
public:
	int Length() const {
		return strlen(Value);
	}
	void SetRaw(const char* value) {
		memcpy(Value, value, strlen(value));
		Value[strlen(value)] = '\0';
	}
	bool operator<(const SILK_STRING& a) const {
		auto minIndex = Length() < a.Length() ? Length() : a.Length();
		auto aVal = GetValue();
		auto bVal = a.GetValue();
		foreach(minIndex) {
			if (aVal[i] < bVal[i])
				return true;
			else if (aVal[i] > bVal[i])
				return false;
		}
		return Length() < a.Length();
	}
SILK_COLLECTION_END

struct SILK_STRING_COMPARATOR {
	bool operator()(const SILK_STRING* a, const SILK_STRING* b) const {
		auto minIndex = a->Length() < b->Length() ? a->Length() : b->Length();
		auto aVal = a->GetValue();
		auto bVal = b->GetValue();
		foreach(minIndex) {
			if (aVal[i] < bVal[i])
				return true;
			else if (aVal[i] > bVal[i])
				return false;
		}
		return a->Length() < b->Length();
	}
};

class StringCompareStrategy : public silk_data::ICompareStrategy<SILK_STRING*> {
public:
	bool Equals(SILK_STRING*& a, SILK_STRING*& b) {
		if (a->Length() != b->Length())
			return false;
		auto aVal = a->GetValue();
		auto bVal = b->GetValue();
		foreach(a->Length())
			if (aVal[i] != bVal[i])
				return false;
		return true;
	}
	bool Less(SILK_STRING*& a, SILK_STRING*& b) {
		auto minIndex = a->Length() < b->Length() ? a->Length() : b->Length();
		auto aVal = a->GetValue();
		auto bVal = b->GetValue();
		foreach(minIndex) {
			if (aVal[i] < bVal[i])
				return true;
			else if (aVal[i] > bVal[i])
				return false;
		}
		return a->Length() < b->Length();
	}
};

SILK_OBJ(IIdentity) {
	VIRTUAL_ACCESSOR(SILK_INT, Value)
};
class Identity : public IIdentity {
	ACCESSOR(SILK_INT, Value)
public:
	Identity(int value) {
		Value = new SILK_INT(value);
	}
	~Identity() {
		delete Value;
	}
};

SILK_OBJ(SILK_GUID) {
public:
	IIdentity* Get() {
		return new Identity(value++);
	}

	~SILK_GUID() { }
private:
	int value = 0;
};

SILK_OBJ(IContext) {
	VIRTUAL_ACCESSOR(SILK_GUID, Guid)
};
class SilkContext : public IContext {
	ACCESSOR(SILK_GUID, Guid)
public:
	SilkContext(SILK_GUID* guid) {
		Guid = guid;
	}
	~SilkContext() {
		Guid = nullptr;
	}
};