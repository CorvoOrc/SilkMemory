#pragma once
#include <iostream>
#include "Core.h"
#include "Schema.h"

class IImplementator;

class SilkFactory {
public:
	SilkFactory() {
		items = new silk_data::RBTree<SILK_STRING*, IImplementator>(new StringCompareStrategy());
	}
	~SilkFactory() {
		items->Clear();
		delete items;
	}
	template <class ...Args> ISILK_WAY_OBJECT* Build(const char* type, Args...args) {
		auto key = new SILK_STRING(type);
		auto impl = items->Find(key)->payload;
		return impl->Build(args...);
	}
	void Register(const char* type, IImplementator* impl) {
		auto key = new SILK_STRING(type);
		items->Insert(*items->MakeNode(key, impl));
	}
protected:
	silk_data::RBTree<SILK_STRING*, IImplementator>* items;
};

class GigaFactory {
public:
	GigaFactory() {
		items = new silk_data::RBTree<SILK_STRING*, SilkFactory>(new StringCompareStrategy());
	}
	~GigaFactory() {
		items->Clear();
		delete items;
	}
	template <class T, class ...Args> T* Build(const char* concreteType, Args...args) {
		auto key = new SILK_STRING(typeid(T).raw_name());
		auto factory = items->Find(key)->payload;
		return (T*)factory->Build(concreteType, args...);
	}
	template <class T>
	void Register(SilkFactory* factory) {
		auto key = new SILK_STRING(typeid(T).raw_name());
		items->Insert(*items->MakeNode(key, factory));
	}
protected:
	silk_data::RBTree<SILK_STRING*, SilkFactory>* items;
};

class IImplementator {
public:
	virtual ISILK_WAY_OBJECT* Build(const char* type, const char* name, IContext* context) = 0;
	virtual ISILK_WAY_OBJECT* Build(IIdentity* id, ISchema* schema) = 0;
};

template<class T>
class SchemaImplementator : public IImplementator {
public:
	ISILK_WAY_OBJECT* Build(const char* type, const char* name, IContext* context) {
		return new T(type, name, context);
	}
	ISILK_WAY_OBJECT* Build(IIdentity* id, ISchema* schema) {
		return nullptr;
	}
};

template<class T>
class ConcreteImplementator : public IImplementator {
public:
	ISILK_WAY_OBJECT* Build(const char* type, const char* name, IContext* context) {
		return nullptr;
	}
	ISILK_WAY_OBJECT* Build(IIdentity* id, ISchema* schema) {
		return new T(id, schema);
	}
};