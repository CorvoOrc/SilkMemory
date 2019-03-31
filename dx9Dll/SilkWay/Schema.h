#pragma once
#include "Core.h"

SILK_OBJ(ISchema) {
	VIRTUAL_ACCESSOR(SILK_STRING, Type)
	VIRTUAL_ACCESSOR(SILK_STRING, Name)
};

class BaseSchema : public ISchema {
	ACCESSOR(SILK_STRING, Type)
	ACCESSOR(SILK_STRING, Name)
public:
	BaseSchema(const char* type, const char* name) {
		Name = new SILK_STRING(name);
		Type = new SILK_STRING(type);
	}
	~BaseSchema() {
		delete Type;
		delete Name;
	}
	bool CheckName(const char* name) {
		return strcmp(name, Name->GetValue()) == 0;
	}
};

