#pragma once
#include "Factory.h"
#include "Schema.h"
#include "MemoryLogger.h"
#include "Chrono.h"
#include "Model.h"
#include "View.h"

class UIStateContext : public SilkContext {
	ACCESSOR(ILogger, Logger)
	ACCESSOR(IChrono, Clock)
	ACCESSOR(GigaFactory, Factory)
	ACCESSOR(IGameModel, Model)
public:
	UIStateContext(SILK_GUID* guid, ILogger* logger, IChrono* clock, GigaFactory* factory, IGameModel* model) : SilkContext(guid) {
		Logger = logger;
		Clock = clock;
		Factory = factory;
		Model = model;
	}
	~UIStateContext() {
		Logger = nullptr;
		Clock = nullptr;
		Factory = nullptr;
		Model = nullptr;
	}
};

class UIStateSchema : public BaseSchema {
	ACCESSOR(UIStateContext, Context)
		ACCESSOR(IRequirement, Requirement)
public:
	UIStateSchema(const char* type, const char* name, IContext* context) : BaseSchema(type, name) {
		Context = dynamic_cast<UIStateContext*>(context);
	}
	~UIStateSchema() {
		Context = nullptr;
		delete Requirement;
	}
};

class EmptyUIStateSchema : public UIStateSchema {
public:
	EmptyUIStateSchema(const char* type, const char* name, IContext* context) : UIStateSchema(type, name, context) { }
};

class LabelStateSchema : public UIStateSchema {
public:
	LabelStateSchema(const char* type, const char* name, IContext* context) : UIStateSchema(type, name, context) { }
};
class LabelAliveStateSchema : public LabelStateSchema {
public:
	LabelAliveStateSchema(const char* type, const char* name, IContext* context) : LabelStateSchema(type, name, context) { }
};
class LabelRessurectStateSchema : public LabelStateSchema {
public:
	LabelRessurectStateSchema(const char* type, const char* name, IContext* context) : LabelStateSchema(type, name, context) { }
};

class ButtonStateSchema : public UIStateSchema {
public:
	ButtonStateSchema(const char* type, const char* name, IContext* context) : UIStateSchema(type, name, context) { }
};
class ButtonAliveStateSchema : public ButtonStateSchema {
public:
	ButtonAliveStateSchema(const char* type, const char* name, IContext* context) : ButtonStateSchema(type, name, context) { }
};
class ButtonRessurectBaseStateSchema : public ButtonStateSchema {
public:
	ButtonRessurectBaseStateSchema(const char* type, const char* name, IContext* context) : ButtonStateSchema(type, name, context) { }
};
class ButtonRessurectExtraStateSchema : public ButtonStateSchema {
public:
	ButtonRessurectExtraStateSchema(const char* type, const char* name, IContext* context) : ButtonStateSchema(type, name, context) { }
};

class IUIStatesSchema : public silk_data::Vector<UIStateSchema*>, public UIStateSchema {
protected:
	IUIStatesSchema(const char* type, const char* name, IContext* context) : Vector(), UIStateSchema(type, name, context) { }
};
class UIStatesSchema : public IUIStatesSchema {
public:
	UIStatesSchema(const char* type, const char* name, IContext* context) : IUIStatesSchema(type, name, context) { }
	~UIStatesSchema() {
		Clear();
	}
};

template<class S=UIStateSchema, class U=IView<IViewSchema>>
SILK_OBJ(UIState) {
	ACCESSOR(IIdentity, Id)
	ACCESSOR(S, Schema)
	ACCESSOR(U, UIElement)
protected:
	UIState(IIdentity* id, ISchema* schema) {
		Id = id;
		Schema = dynamic_cast<S*>(schema);
	}
public:
	~UIState() {
		delete Id;
		Schema = nullptr;
		UIElement = nullptr;
	}
	virtual void Activate() = 0;
	virtual void Deactivate() = 0;
};

class EmptyUIState : public UIState<EmptyUIStateSchema> {
public:
	EmptyUIState(IIdentity* id, ISchema* schema) : UIState(id, schema) { }
	void Activate() { }
	void Deactivate() { }
};

class LabelState : public UIState<LabelStateSchema, ILabel> {
protected:
	LabelState(IIdentity* id, ISchema* schema) : UIState(id, schema) { }
	IChrono* GetClock() {
		return Schema->GetContext()->GetClock();
	}
};

class LabelAliveState : public LabelState {
public:
	LabelAliveState(IIdentity* id, ISchema* schema) : LabelState(id, schema) { }
	void Activate() {
		UIElement->SetRawText("Roshan states: alive");
	}
	void Deactivate() { }
};

class LabelRessurectState : public LabelState {
public:
	LabelRessurectState(IIdentity* id, ISchema* schema) : LabelState(id, schema) { 
		clockStrikingCallback = new MemberCallback<LabelRessurectState, IEventArgs>(&LabelRessurectState::OnClockStriking, this);
	}
	~LabelRessurectState() {
		delete clockStrikingCallback;
	}
	void Activate() {
		GetClock()->GetStruck()->Add(clockStrikingCallback);
	}
	void Deactivate() {
		GetClock()->GetStruck()->Remove(clockStrikingCallback);
	}
	void OnClockStriking(IEventArgs* args) {
		char title[MAX_STRING_SIZE]; title[0] = '\0';
		GetRestTime(title);
		UIElement->SetRawText(title);
	}
	void GetRestTime(char* out) {
		auto model = Schema->GetContext()->GetModel();
		auto currentState = model->GetRoshanStatus()->GetStates()->GetCurrent();
		auto dest = currentState->GetSchema();
		long long deathTs = *model->GetResources()->GetByName("roshan_death_ts")->GetValue()->GetValue();
		auto stamp = GetClock()->GetStamp();
		long long diff = GetReviveTime() - GetClock()->GetDiffS(deathTs);
		long long minutes = diff / 60;
		long long secs = diff < 60 ? diff : diff % 60;
		sprintf_s(out, 1024, "Roshan status: %s(%lldm:%llds)", dest->GetName()->GetValue(), minutes, secs);
	}
	long long GetReviveTime() {
		auto model = Schema->GetContext()->GetModel();
		auto currentState = model->GetRoshanStatus()->GetStates()->GetCurrent();
		auto generalSchema = model->GetSchema();
		auto ressurectBaseTime = *generalSchema->GetRessurectBaseTime()->GetValue();
		auto ressurectExtraTime = *generalSchema->GetRessurectExtraTime()->GetValue();
		return currentState->GetSchema()->CheckName("ressurect_base") ? ressurectBaseTime : ressurectExtraTime;
	}
private:
	ICallback* clockStrikingCallback;
};

class ButtonState : public UIState<ButtonStateSchema, IButton> {
protected:
	ButtonState(IIdentity* id, ISchema* schema) : UIState(id, schema) { }
};

class ButtonAliveState : public ButtonState {
public:
	ButtonAliveState(IIdentity* id, ISchema* schema) : ButtonState(id, schema) { }
	void Activate() {
		UIElement->GetEnable()->SetRaw(true);
		UIElement->GetLabel()->SetRawText("Kill Roshan");
	}
	void Deactivate() { }
};

class ButtonRessurectBaseState : public ButtonState {
public:
	ButtonRessurectBaseState(IIdentity* id, ISchema* schema) : ButtonState(id, schema) { }
	void Activate() {
		UIElement->GetEnable()->SetRaw(false);
	}
	void Deactivate() { }
};

class ButtonRessurectExtraState : public ButtonState {
public:
	ButtonRessurectExtraState(IIdentity* id, ISchema* schema) : ButtonState(id, schema) { }
	void Activate() {
		UIElement->GetEnable()->SetRaw(true);
		UIElement->GetLabel()->SetRawText("Revive Roshan");
	}
	void Deactivate() { }
};

class IUIStates : public silk_data::Vector<UIState<>*>, public UIState<UIStatesSchema> {
protected:
	IUIStates(IIdentity* id, ISchema* schema) : Vector(), UIState(id, schema) { }
public:
	virtual void Update() = 0;
	virtual void SetElement(IView<IViewSchema>* element) = 0;
};
class UIStates : public IUIStates {
public:
	UIStates(IIdentity* id, ISchema* schema) : IUIStates(id, schema) {
		current = nullptr;
		auto factory = Schema->GetContext()->GetFactory();
		auto guid = Schema->GetContext()->GetGuid();
		foreach(Schema->Length()) {
			auto stateSchema = Schema->GetItem(i);
			auto uiState = factory->Build<UIState<>>(stateSchema->GetType()->GetValue(), guid->Get(), stateSchema);
			PushBack(uiState);
		}
	}
	~UIStates() {
		Clear();
		current = nullptr;
	}
	void Update() {
		foreach(Length()) {
			if (GetItem(i)->GetSchema()->GetRequirement()->Check()) {
				if (current != nullptr)
					current->Deactivate();
				current = GetItem(i);
				current->Activate();
			}
		}
	}
	void Activate() { }
	void Deactivate() { }
	void SetElement(IView<IViewSchema>* element) {
		SetUIElement(element);
		foreach(Length())
			GetItem(i)->SetUIElement(element);
	}
private:
	UIState<>* current;
};

class UIStateRegistrator {
public:
	void Register(GigaFactory* factories);
};
