#pragma once
#include "Core.h"
#include "Factory.h"
#include "Schema.h"
#include "MemoryLogger.h"
#include "Chrono.h"
#include <climits>
#include <d3dx9.h>

using namespace silk_data;

class IGraphicDeviceHolder {
	VIRTUAL_ACCESSOR(void, Value)
};

class D9Device : public IGraphicDeviceHolder {
	ACCESSOR(void, Value)
public:
	D9Device(void* device) {
		Value = device;
	}
};

class ViewContext : public SilkContext {
	ACCESSOR(ILogger, Logger)
	ACCESSOR(IChrono, Clock)
	ACCESSOR(GigaFactory, Factory)
public:
	ViewContext(SILK_GUID* guid, ILogger* logger, IChrono* clock, GigaFactory* factory) : SilkContext(guid) {
		Logger = logger;
		Clock = clock;
		Factory = factory;
	}
	~ViewContext() {
		Logger = nullptr;
		Clock = nullptr;
		Factory = nullptr;
	}
};

class IViewSchema : public BaseSchema {
	ACCESSOR(ViewContext, Context)
public:
	IViewSchema(const char* type, const char* name, IContext* context) : BaseSchema(type, name) {
		Context = dynamic_cast<ViewContext*>(context);
	}
	~IViewSchema() {
		Context = nullptr;
	}
};
class IViewCollectionSchema : public silk_data::Vector<IViewSchema*>, public IViewSchema {
protected:
	IViewCollectionSchema(const char* type, const char* name, IContext* context) : Vector(), IViewSchema(type, name, context) { }
public:
	~IViewCollectionSchema() {
		Clear();
	}
};
class ElementCollectionSchema : public IViewCollectionSchema {
public:
	ElementCollectionSchema(const char* type, const char* name, IContext* context) : IViewCollectionSchema(type, name, context) { }
};
class ILabelSchema : public IViewSchema {
	VIRTUAL_ACCESSOR(SILK_INT, RecRight)
	VIRTUAL_ACCESSOR(SILK_INT, RecLeft)
	VIRTUAL_ACCESSOR(SILK_INT, RecTop)
	VIRTUAL_ACCESSOR(SILK_INT, RecDown)
	VIRTUAL_ACCESSOR(SILK_FLOAT, ColorR)
	VIRTUAL_ACCESSOR(SILK_FLOAT, ColorG)
	VIRTUAL_ACCESSOR(SILK_FLOAT, ColorB)
	VIRTUAL_ACCESSOR(SILK_FLOAT, ColorA)
	VIRTUAL_ACCESSOR(SILK_STRING, Text)
protected:
	ILabelSchema(const char* type, const char* name, IContext* context) : IViewSchema(type, name, context) { }
};
class LabelD9Schema : public ILabelSchema {
	ACCESSOR(SILK_INT, RecRight)
	ACCESSOR(SILK_INT, RecLeft)
	ACCESSOR(SILK_INT, RecTop)
	ACCESSOR(SILK_INT, RecDown)
	ACCESSOR(SILK_FLOAT, ColorR)
	ACCESSOR(SILK_FLOAT, ColorG)
	ACCESSOR(SILK_FLOAT, ColorB)
	ACCESSOR(SILK_FLOAT, ColorA)
	ACCESSOR(SILK_STRING, Text)
public:
	LabelD9Schema(const char* type, const char* name, IContext* context) : ILabelSchema(type, name, context) { }
	~LabelD9Schema() {
		delete RecRight;
		delete RecLeft;
		delete RecTop;
		delete RecDown;
		delete ColorR;
		delete ColorG;
		delete ColorB;
		delete ColorA;
		delete Text;
	}
};
class IButtonSchema : public IViewSchema {
	VIRTUAL_ACCESSOR(ILabelSchema, Label)
	VIRTUAL_ACCESSOR(SILK_INT, BorderColorR)
	VIRTUAL_ACCESSOR(SILK_INT, BorderColorG)
	VIRTUAL_ACCESSOR(SILK_INT, BorderColorB)
	VIRTUAL_ACCESSOR(SILK_INT, BorderColorA)
	VIRTUAL_ACCESSOR(SILK_INT, FillColorR)
	VIRTUAL_ACCESSOR(SILK_INT, FillColorG)
	VIRTUAL_ACCESSOR(SILK_INT, FillColorB)
	VIRTUAL_ACCESSOR(SILK_INT, FillColorA)
	VIRTUAL_ACCESSOR(SILK_INT, PushColorR)
	VIRTUAL_ACCESSOR(SILK_INT, PushColorG)
	VIRTUAL_ACCESSOR(SILK_INT, PushColorB)
	VIRTUAL_ACCESSOR(SILK_INT, PushColorA)
	VIRTUAL_ACCESSOR(SILK_FLOAT, Border)
protected:
	IButtonSchema(const char* type, const char* name, IContext* context) : IViewSchema(type, name, context) { }
};
class ButtonD9Schema : public IButtonSchema {
	ACCESSOR(ILabelSchema, Label)
	ACCESSOR(SILK_INT, BorderColorR)
	ACCESSOR(SILK_INT, BorderColorG)
	ACCESSOR(SILK_INT, BorderColorB)
	ACCESSOR(SILK_INT, BorderColorA)
	ACCESSOR(SILK_INT, FillColorR)
	ACCESSOR(SILK_INT, FillColorG)
	ACCESSOR(SILK_INT, FillColorB)
	ACCESSOR(SILK_INT, FillColorA)
	ACCESSOR(SILK_INT, PushColorR)
	ACCESSOR(SILK_INT, PushColorG)
	ACCESSOR(SILK_INT, PushColorB)
	ACCESSOR(SILK_INT, PushColorA)
	ACCESSOR(SILK_FLOAT, Border)
public:
	ButtonD9Schema(const char* type, const char* name, IContext* context) : IButtonSchema(type, name, context) { }
	~ButtonD9Schema() {
		delete Label;
		delete BorderColorR;
		delete BorderColorG;
		delete BorderColorB;
		delete BorderColorA;
		delete FillColorR;
		delete FillColorG;
		delete FillColorB;
		delete FillColorA;
		delete PushColorR;
		delete PushColorG;
		delete PushColorB;
		delete PushColorA;
		delete Border;
	}
};
class ICanvasSchema : public IViewSchema {
	VIRTUAL_ACCESSOR(IViewCollectionSchema, Elements)
public:
	ICanvasSchema(const char* type, const char* name, IContext* context) : IViewSchema(type, name, context) {}
};
class CanvasD9Schema : public ICanvasSchema {
	ACCESSOR(IViewCollectionSchema, Elements)
public:
	CanvasD9Schema(const char* type, const char* name, IContext* context) : ICanvasSchema(type, name, context) { }
	~CanvasD9Schema() {
		delete Elements;
	}
};

template<class T>
SILK_OBJ(IView) {
	ACCESSOR(IIdentity, Id)
	ACCESSOR(T, Schema)
	ACCESSOR(SILK_BOOL, Enable)
protected:
	IView(IIdentity* id, ISchema* schema) {
		Id = id;
		Schema = dynamic_cast<T*>(schema);
		Enable = new SILK_BOOL(true);
	}
public:
	~IView() {
		delete Id;
		delete Enable;
		Schema = nullptr;
		device = nullptr;
	}
	virtual void HandleInput() = 0;
	virtual void Draw() = 0;
	virtual RECT* GetRect() = 0;
	virtual void SetDevice(IGraphicDeviceHolder* device) = 0;
	virtual bool ContainPoint(float x, float y) = 0;
	virtual void Release() = 0;

	bool CheckMouseOver() {
		POINT cursorPos;
		D3DDEVICE_CREATION_PARAMETERS params;
		device->GetCreationParameters(&params);
		GetCursorPos(&cursorPos);
		ScreenToClient(params.hFocusWindow, &cursorPos);
		return ContainPoint(cursorPos.x, cursorPos.y);
	}
protected:
	IDirect3DDevice9* device;
};
class ILabel : public IView<ILabelSchema> {
protected:
	ILabel(IIdentity* id, ISchema* schema) : IView(id, schema) { }
public:
	virtual void SetRawText(const char* value) = 0;
};
class LabelD9 : public ILabel {
public:
	LabelD9(IIdentity* id, ISchema* schema) : ILabel(id, schema) {
		rect = new RECT();
		SetRect(rect, *Schema->GetRecLeft()->GetValue(), *Schema->GetRecTop()->GetValue(),
			*Schema->GetRecRight()->GetValue(), *Schema->GetRecDown()->GetValue());
		color = new D3DXCOLOR(*Schema->GetColorR()->GetValue(), *Schema->GetColorG()->GetValue(), 
			*Schema->GetColorB()->GetValue(), *Schema->GetColorA()->GetValue());
		text = new SILK_STRING(Schema->GetText()->GetValue());
	}
	~LabelD9() {
		delete text;
		delete rect;
		delete color;
		//DeleteObject(font);
	}
	RECT* GetRect() {
		return rect;
	}
	virtual void HandleInput() { 
		if (!*Enable->GetValue())
			return;
	}
	void SetDevice(IGraphicDeviceHolder* value) {
		device = (IDirect3DDevice9*)value->GetValue();
	}
	virtual void SetText(SILK_STRING* value) {
		text = value;
	}
	virtual void SetRawText(const char* value) {
		text->SetRaw(value);
	}
	virtual void Draw() {
		HRESULT r = D3DXCreateFont(device, 15, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			"Arial", &font);
		int res = font->DrawText(0, text->GetValue(), strlen(text->GetValue()), rect, DT_NOCLIP, *color);
	}
	virtual bool ContainPoint(float x, float y) {
		return x >= rect->left && x <= rect->right && y <= rect->bottom && y >= rect->top;
	}
	virtual void Release() {
		if (font)
			font->Release();
	}
private:
	ID3DXFont* font;
	SILK_STRING* text;
	RECT* rect;
	D3DXCOLOR* color;
};
class IButton : public IView<IButtonSchema> {
	VIRTUAL_EVENT(ClickedEvent)
	VIRTUAL_ACCESSOR(ILabel, Label)
protected:
	IButton(IIdentity* id, ISchema* schema) : IView(id, schema) { }
};
class ButtonD9 : public IButton {
	EVENT(ClickedEvent)
	ACCESSOR(ILabel, Label)
public:
	ButtonD9(IIdentity* id, ISchema* schema) : IButton(id, schema) {
		ClickedEvent = new Event();

		auto factory = Schema->GetContext()->GetFactory();
		auto guid = Schema->GetContext()->GetGuid();
		Label = factory->Build<ILabel>(Schema->GetLabel()->GetType()->GetValue(), guid->Get(), Schema->GetLabel());
		borderColor = new D3DCOLOR(D3DCOLOR_ARGB(*Schema->GetBorderColorA()->GetValue(), *Schema->GetBorderColorR()->GetValue(), 
			*Schema->GetBorderColorG()->GetValue(), *Schema->GetBorderColorB()->GetValue()));
		fillColor = new D3DCOLOR(D3DCOLOR_ARGB(*Schema->GetFillColorA()->GetValue(), *Schema->GetFillColorR()->GetValue(),
			*Schema->GetFillColorG()->GetValue(), *Schema->GetFillColorB()->GetValue()));
		pushColor = new D3DCOLOR(D3DCOLOR_ARGB(*Schema->GetPushColorA()->GetValue(), *Schema->GetPushColorR()->GetValue(),
			*Schema->GetPushColorG()->GetValue(), *Schema->GetPushColorB()->GetValue()));
		borderWidth = *Schema->GetBorder()->GetValue();
	}
	~ButtonD9() {
		delete ClickedEvent;
		delete Label;
		delete borderColor;
		delete fillColor;
		delete pushColor;
		borderWidth = 0;
	}
	virtual void HandleInput() {
		mousePressed = GetAsyncKeyState(VK_LBUTTON);
		if (!*Enable->GetValue())
			return;
		clicked = false;
		if (!pressed && mousePressed) {
			if (CheckMouseOver())
				pressed = true;
		}
		else if (pressed && !mousePressed) {
			pressed = false;
			if (CheckMouseOver()) {
				clicked = true;
				ClickedEvent->Invoke(nullptr);
			}
		}
	}
	RECT* GetRect() {
		return Label->GetRect();
	}
	void SetDevice(IGraphicDeviceHolder* value) {
		device = (IDirect3DDevice9*)value->GetValue();
		Label->SetDevice(value);
	}
	virtual void SetRawText(const char* value) {
		Label->SetRawText(value);
	}
	virtual void Draw() {
		DrawRect();
		Label->Draw();
	}
	virtual void Release() {
		Label->Release();
	}
	virtual bool ContainPoint(float x, float y) {
		return Label->ContainPoint(x, y);
	}
private:
	void DrawLine(float x1, float y1, float x2, float y2, float width, D3DCOLOR color) {
		ID3DXLine* line;
		D3DXCreateLine(device, &line);
		D3DXVECTOR2* pVertexList = new D3DXVECTOR2[2];
		pVertexList[0] = D3DXVECTOR2(x1, y1);
		pVertexList[1] = D3DXVECTOR2(x2, y2);
		line->SetWidth(width);
		line->Draw(pVertexList, 2, color);
		line->Release();
	}
	void DrawRect() {
		auto rect = Label->GetRect();
		DrawLine(rect->left, rect->top, rect->right, rect->top, borderWidth, *borderColor);
		DrawLine(rect->right, rect->top, rect->right, rect->bottom, borderWidth, *borderColor);
		DrawLine(rect->right, rect->bottom, rect->left, rect->bottom, borderWidth, *borderColor);
		DrawLine(rect->left, rect->bottom, rect->left, rect->top, borderWidth, *borderColor);
		float fillY = rect->top + (rect->bottom - rect->top) / 2;

		if (pressed)
			DrawLine(rect->left, fillY, rect->right, fillY, rect->bottom - rect->top, *pushColor);
		else
			DrawLine(rect->left, fillY, rect->right, fillY, rect->bottom - rect->top, *fillColor);
	}
private:
	D3DCOLOR* borderColor;
	D3DCOLOR* fillColor;
	D3DCOLOR* pushColor;
	float borderWidth;

	bool mousePressed;
	bool pressed;
	bool clicked;
};

template<class T, class S>
class IViewCollection : public silk_data::Vector<IView<T>*>, public IView<S> {
protected:
	IViewCollection(IIdentity* id, ISchema* schema) : Vector(), IView(id, schema) { }
public:
	~IViewCollection() {
		Clear();
	}
};
class IElementCollection : public IViewCollection<IViewSchema, IViewCollectionSchema> {
protected:
	IElementCollection(IIdentity* id, ISchema* schema) : IViewCollection(id, schema) { }
public:
	virtual IView<IViewSchema>* GetByName(const char* name) = 0;
};
class ElementCollection : public IElementCollection {
public:
	ElementCollection(IIdentity* id, ISchema* schema) : IElementCollection(id, schema) {
		rect = new RECT();
		auto factory = Schema->GetContext()->GetFactory();
		auto guid = Schema->GetContext()->GetGuid();
		foreach(Schema->Length()) {
			auto elementSchema = Schema->GetItem(i);
			auto element = factory->Build<IView<IViewSchema>>(elementSchema->GetType()->GetValue(), guid->Get(), elementSchema);
			PushBack(element);
		}
	}
	~ElementCollection() {
		delete rect;
	}
	virtual void HandleInput() {
		foreach(Length())
			GetItem(i)->HandleInput();
	}
	RECT* GetRect() {
		return rect;
	}
	virtual void Draw() {
		foreach(Length())
			GetItem(i)->Draw();
	}
	virtual void SetDevice(IGraphicDeviceHolder* device) {
		foreach(Length())
			GetItem(i)->SetDevice(device);
	}
	virtual bool ContainPoint(float x, float y) {
		foreach(Length())
			if (GetItem(i)->ContainPoint(x, y))
				return true;
		return false;
	}
	virtual IView<IViewSchema>* GetByName(const char* name) {
		foreach(Length())
			if (GetItem(i)->GetSchema()->CheckName(name))
				return GetItem(i);
		return nullptr;
	}
	void Release() {
		foreach(Length())
			GetItem(i)->Release();
	}
protected:
	virtual void Insert(IView<IViewSchema>*& item, int pos) {
		Vector::Insert(item, pos);
		UpdateRect();
	}
	virtual void PushBack(IView<IViewSchema>*& item) {
		Vector::PushBack(item);
		UpdateRect();
	}
	void UpdateRect() {
		if (Length() == 0)
			return;
		int left = INT_MAX;
		int right = INT_MIN;
		int top = INT_MAX;
		int bottom = INT_MIN;
		foreach(Length()) {
			auto r = GetItem(i)->GetRect();
			if (left > r->left)
				left = r->left;
			if (right < r->right)
				right = r->right;
			if (top > r->top)
				top = r->top;
			if (bottom < r->bottom)
				bottom = r->bottom;
		}
		SetRect(rect, left, top, right, bottom);
	}
private:
	RECT* rect;
};

class ICanvas : public IView<ICanvasSchema> {
protected:
	ICanvas(IIdentity* id, ISchema* schema) : IView(id, schema) { }
public:
	virtual IView<IViewSchema>* Get(const char* name) = 0;
};
class CanvasD9 : public ICanvas {
public:
	CanvasD9(IIdentity* id, ISchema* schema) : ICanvas(id, schema) {
		auto factory = Schema->GetContext()->GetFactory();
		auto guid = Schema->GetContext()->GetGuid();
		auto elemsSchema = Schema->GetElements();
		elements = factory->Build<IElementCollection>(elemsSchema->GetType()->GetValue(), guid->Get(), elemsSchema);
	}
	~CanvasD9() {
		delete elements;
	}
	void HandleInput() {
		elements->HandleInput();
	}
	RECT* GetRect() {
		return elements->GetRect();
	}
	virtual void Draw() {
		elements->Draw();
	}
	virtual void SetDevice(IGraphicDeviceHolder* device) {
		elements->SetDevice(device);
	}
	virtual bool ContainPoint(float x, float y) {
		return elements->ContainPoint(x, y);
	}
	IView<IViewSchema>* Get(const char* name) {
		return elements->GetByName(name);
	}
	void Release() {
		elements->Release();
	}
private:
	IElementCollection* elements;
};

class ViewRegistrator {
public:
	void Register(GigaFactory* factories);
};