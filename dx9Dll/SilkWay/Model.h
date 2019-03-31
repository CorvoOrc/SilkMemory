#pragma once
#include "Schema.h"
#include "Factory.h"
#include "Event.h"
#include "MemoryLogger.h"
#include "Chrono.h"

class IGameModel;
class GameModel;

class ModelContext : public SilkContext {
	ACCESSOR(ILogger, Logger)
	ACCESSOR(IChrono, Clock)
	ACCESSOR(GigaFactory, Factory)
	ACCESSOR(IGameModel*, Model)
public:
	ModelContext(SILK_GUID* guid, ILogger* logger, IChrono* clock, GigaFactory* factory, IGameModel** model) : SilkContext(guid) {
		Logger = logger;
		Clock = clock;
		Factory = factory;
		Model = model;
	}
	~ModelContext() {
		Logger = nullptr;
		Clock = nullptr;
		Factory = nullptr;
		Model = nullptr;
	}
};

class IModelSchema : public BaseSchema {
	ACCESSOR(ModelContext, Context)
public:
	IModelSchema(const char* type, const char* name, IContext* context) : BaseSchema(type, name) {
		Context = dynamic_cast<ModelContext*>(context);
	}
	~IModelSchema() {
		Context = nullptr;
	}
};

SILK_OBJ(IComponent) {
	ACCESSOR(IIdentity, Id)
public:
	IComponent(IIdentity* id) {
		Id = id;
	}
	~IComponent() {
		delete Id;
	}
};

template<class S>
SILK_OBJ(IModel) {
	ACCESSOR(IIdentity, Id)
	ACCESSOR(S, Schema)
public:
	IModel(IIdentity* id, ISchema* schema) {
		Id = id;
		Schema = dynamic_cast<S*>(schema);
		components = new silk_data::RBTree<SILK_STRING*, IComponent>(new StringCompareStrategy());
	}
	~IModel() {
		delete Id;
		Schema = nullptr;
		components->Clear();
		delete components;
	}
	template <class T> 
	T* Get(SILK_STRING* key) {
		return (T*)components->Find(key);
	}
private:
	silk_data::RBTree<SILK_STRING*, IComponent>* components;
};

template <class S>
class ICollectionSchema : public silk_data::Vector<S*>, public IModelSchema {
protected:
	ICollectionSchema(const char* type, const char* name, IContext* context) : Vector(), IModelSchema(type, name, context) { }
public:
	~ICollectionSchema() {
		Clear();
	}
};

template <class T, class S>
class IModelCollection : public silk_data::Vector<T*>, public IModel<S> {
protected:
	IModelCollection(IIdentity* id, ISchema* schema) : Vector(), IModel(id, schema) {
		auto factory = Schema->GetContext()->GetFactory();
		auto guid = Schema->GetContext()->GetGuid();
		foreach(Schema->Length()) {
			auto itemSchema = Schema->GetItem(i);
			auto item = factory->Build<T>(itemSchema->GetType()->GetValue(), guid->Get(), itemSchema);
			PushBack(item);
		}
	}
public:
	~IModelCollection() {
		Clear();
	}
	T* GetByName(const char* name) {
		foreach(Length())
			if (GetItem(i)->GetSchema()->CheckName(name))
				return GetItem(i);
		return nullptr;
	}
};

#define DEFINE_ISCHEMA(child) class child : public IModelSchema
#define DEFINE_IMODEL(child, T) class child : public IModel<T>
#define DEFINE_ICOLLECTION_SCHEMA(child) class child : public ICollectionSchema
#define DEFINE_ICOLLECTION_MODEL(child) class child : public IModelCollection

#define DEFINE_SCHEMA(child, base) class child : public base
#define DEFINE_MODEL(child, base) class child : public base
#define DEFINE_COLLECTION_SCHEMA(child, base) class child : public base
#define DEFINE_COLLECTION_MODEL(child, base) class child : public base

DEFINE_ISCHEMA(IRequirement) {
public:
	virtual bool Check() = 0;
protected:
	IRequirement(const char* type, const char* name, IContext* context) : IModelSchema(type, name, context) { }
};

DEFINE_SCHEMA(TrueRequirement, IRequirement) {
public:
	bool Check() {
		return true;
	}
public:
	TrueRequirement(const char* type, const char* name, IContext* context) : IRequirement(type, name, context) { }
};

DEFINE_SCHEMA(FalseRequirement, IRequirement) {
public:
	bool Check() {
		return false;
	}
public:
	FalseRequirement(const char* type, const char* name, IContext* context) : IRequirement(type, name, context) { }
};

DEFINE_SCHEMA(RoshanKilledRequirement, IRequirement) {
public:
	bool Check();
public:
	RoshanKilledRequirement(const char* type, const char* name, IContext* context) : IRequirement(type, name, context) { }
};

DEFINE_SCHEMA(RoshanAliveManualRequirement, IRequirement) {
public:
	bool Check();
public:
	RoshanAliveManualRequirement(const char* type, const char* name, IContext* context) : IRequirement(type, name, context) { }
};

DEFINE_SCHEMA(TimeRequirement, IRequirement) {
	ACCESSOR(SILK_STRING, ResourceName)
	ACCESSOR(SILK_LL, Offset)
public:
	bool Check();
public:
	TimeRequirement(const char* type, const char* name, IContext* context) : IRequirement(type, name, context) { }
	~TimeRequirement() {
		delete ResourceName;
		delete Offset;
	}
};

DEFINE_SCHEMA(RoshanStateRequirement, IRequirement) {
	ACCESSOR(SILK_STRING, StateName)
public:
	bool Check();
public:
	RoshanStateRequirement(const char* type, const char* name, IContext* context) : IRequirement(type, name, context) { }
	~RoshanStateRequirement() {
		delete StateName;
	}
};

DEFINE_ISCHEMA(IAction) {
public:
	virtual void Make() = 0;
protected:
	IAction(const char* type, const char* name, IContext* context) : IModelSchema(type, name, context) { }
};
DEFINE_SCHEMA(EmptyAction, IAction) {
public:
	void Make() { }
public:
	EmptyAction(const char* type, const char* name, IContext* context) : IAction(type, name, context) { }
};
DEFINE_SCHEMA(SetCurrentTimeAction, IAction) {
	ACCESSOR(SILK_STRING, ResourceName)
public:
	void Make();
public:
	SetCurrentTimeAction(const char* type, const char* name, IContext* context) : IAction(type, name, context) { }
	~SetCurrentTimeAction() { 
		delete ResourceName;
	}
};

DEFINE_ISCHEMA(ITransitionSchema) {
	VIRTUAL_ACCESSOR(SILK_STRING, From)
	VIRTUAL_ACCESSOR(SILK_STRING, To)
	VIRTUAL_ACCESSOR(IRequirement, Requirement)
	VIRTUAL_ACCESSOR(IAction, Action)
protected:
	ITransitionSchema(const char* type, const char* name, IContext* context) : IModelSchema(type, name, context) { }
};
DEFINE_SCHEMA(TransitionSchema, ITransitionSchema) {
	ACCESSOR(SILK_STRING, From)
	ACCESSOR(SILK_STRING, To)
	ACCESSOR(IRequirement, Requirement)
	ACCESSOR(IAction, Action)
public:
	TransitionSchema(const char* type, const char* name, IContext* context) : ITransitionSchema(type, name, context) { }
	~TransitionSchema() {
		delete From; 
		delete To; 
		delete Requirement;
		delete Action;
	}
};

DEFINE_ICOLLECTION_SCHEMA(ITransitionsSchema) <ITransitionSchema> { 
protected:
	ITransitionsSchema(const char* type, const char* name, IContext* context) : ICollectionSchema(type, name, context) { }
};
DEFINE_COLLECTION_SCHEMA(TransitionsSchema, ITransitionsSchema) {
public:
	TransitionsSchema(const char* type, const char* name, IContext* context) : ITransitionsSchema(type, name, context) { }
};

DEFINE_ISCHEMA(IStateSchema) {
	VIRTUAL_ACCESSOR(ITransitionsSchema, Transitions)
protected:
	IStateSchema(const char* type, const char* name, IContext* context) : IModelSchema(type, name, context) { }
};
DEFINE_SCHEMA(StateSchema, IStateSchema) {
	ACCESSOR(ITransitionsSchema, Transitions)
public:
	StateSchema(const char* type, const char* name, IContext* context) : IStateSchema(type, name, context) { }
	~StateSchema() { 
		delete Transitions; 
	}
};

DEFINE_ICOLLECTION_SCHEMA(IStatesSchema) < IStateSchema > {
	VIRTUAL_ACCESSOR(SILK_STRING, StartState)
protected:
	IStatesSchema(const char* type, const char* name, IContext* context) : ICollectionSchema(type, name, context) { }
};
DEFINE_COLLECTION_SCHEMA(StatesSchema, IStatesSchema) {
	ACCESSOR(SILK_STRING, StartState)
public:
	StatesSchema(const char* type, const char* name, IContext* context) : IStatesSchema(type, name, context) { }
	~StatesSchema() {
		delete StartState;
	}
};

DEFINE_ISCHEMA(IRoshanStatusSchema) {
	VIRTUAL_ACCESSOR(IStatesSchema, States)
protected:
	IRoshanStatusSchema(const char* type, const char* name, IContext* context) : IModelSchema(type, name, context) { }
};
DEFINE_SCHEMA(RoshanStatusSchema, IRoshanStatusSchema) {
	ACCESSOR(IStatesSchema, States)
public:
	RoshanStatusSchema(const char* type, const char* name, IContext* context) : IRoshanStatusSchema(type, name, context) { }
	~RoshanStatusSchema() { 
		delete States; 
	}
};

DEFINE_IMODEL(IStateModel, IStateSchema) {
protected:
	IStateModel(IIdentity* id, ISchema* schema) : IModel(id, schema) { }
};
DEFINE_MODEL(StateModel, IStateModel) {
public:
	StateModel(IIdentity* id, ISchema* schema) : IStateModel(id, schema) {}
};

DEFINE_ICOLLECTION_MODEL(IStatesModel) < IStateModel, IStatesSchema > {
	VIRTUAL_REACTIVE_COMPONENT(IStateModel, Current)
protected:
	IStatesModel(IIdentity* id, ISchema* schema) : IModelCollection(id, schema) { }
};
DEFINE_COLLECTION_SCHEMA(StatesModel, IStatesModel) {
	REACTIVE_COMPONENT(IStateModel, Current)
public:
	StatesModel(IIdentity* id, ISchema* schema) : IStatesModel(id, schema) {
		Current = GetByName(Schema->GetStartState()->GetValue());
		CurrentChanged = new Event();
	}
	~StatesModel() {
		delete CurrentChanged;
	}
};

DEFINE_IMODEL(IRoshanStatusModel, IRoshanStatusSchema) {
	VIRTUAL_COMPONENT(IStatesModel, States)
public:
	virtual void Resolve() = 0;
protected:
	IRoshanStatusModel(IIdentity* id, ISchema* schema) : IModel(id, schema) { }
};
DEFINE_MODEL(RoshanStatusModel, IRoshanStatusModel) {
	COMPONENT(IStatesModel, States)
public:
	RoshanStatusModel(IIdentity* id, ISchema* schema) : IRoshanStatusModel(id, schema) {
		auto factory = Schema->GetContext()->GetFactory();
		auto guid = Schema->GetContext()->GetGuid();
		auto statesSchema = Schema->GetStates();
		States = factory->Build<IStatesModel>(statesSchema->GetType()->GetValue(), guid->Get(), statesSchema);
	}
	~RoshanStatusModel() {
		delete States;
	}
	void Resolve() {
		auto currentStateSchema = States->GetCurrent()->GetSchema();
		Schema->GetContext()->GetLogger()->Log("RESOLVE\n");
		foreach(currentStateSchema->GetTransitions()->Length()) {
			auto transition = currentStateSchema->GetTransitions()->GetItem(i);
			if (transition->GetRequirement()->Check()) {
				transition->GetAction()->Make();
				States->SetCurrent(States->GetByName(transition->GetTo()->GetValue()));
				break;
			}
		}
	}
};

DEFINE_ISCHEMA(ITriggerSchema) {
protected:
	ITriggerSchema(const char* type, const char* name, IContext* context) : IModelSchema(type, name, context) { }
};
DEFINE_SCHEMA(TriggerSchema, ITriggerSchema) {
public:
	TriggerSchema(const char* type, const char* name, IContext* context) : ITriggerSchema(type, name, context) { }
};

DEFINE_ICOLLECTION_SCHEMA(ITriggersSchema) < ITriggerSchema > { 
protected:
	ITriggersSchema(const char* type, const char* name, IContext* context) : ICollectionSchema(type, name, context) { }
};
DEFINE_COLLECTION_SCHEMA(TriggersSchema, ITriggersSchema) {
public:
	TriggersSchema(const char* type, const char* name, IContext* context) : ITriggersSchema(type, name, context) { }
};

DEFINE_IMODEL(ITrigger, ITriggerSchema) {
	VIRTUAL_COMPONENT(SILK_BOOL, Value)
public:
	virtual void Activate() = 0;
	virtual void Deactivate() = 0;
	virtual bool IsActive() = 0;
protected:
	ITrigger(IIdentity* id, ISchema* schema) : IModel(id, schema) {}
};
DEFINE_MODEL(Trigger, ITrigger) {
	COMPONENT(SILK_BOOL, Value)
public:
	Trigger(IIdentity* id, ISchema* schema) : ITrigger(id, schema) {
		Value = new SILK_BOOL(false); 
	}
	~Trigger() {
		delete Value;
	}
	void Activate() { 
		Value->SetRaw(true);
	}
	void Deactivate() { 
		Value->SetRaw(false);
	}
	bool IsActive() {
		return *Value->GetValue(); 
	}
};

DEFINE_ICOLLECTION_MODEL(ITriggerCollection) <ITrigger, ITriggersSchema> {
protected:
	ITriggerCollection(IIdentity* id, ISchema* schema) : IModelCollection(id, schema) { }
public:
	virtual void Reset() = 0;
};
DEFINE_COLLECTION_MODEL(TriggerCollection, ITriggerCollection) {
public:
	TriggerCollection(IIdentity* id, ISchema* schema) : ITriggerCollection(id, schema) { }
	void Reset() {
		foreach(Length())
			GetItem(i)->Deactivate();
	}
};

DEFINE_ISCHEMA(IResourceSchema) {
protected:
	IResourceSchema(const char* type, const char* name, IContext* context) : IModelSchema(type, name, context) { }
};
DEFINE_SCHEMA(ResourceSchema, IResourceSchema) {
public:
	ResourceSchema(const char* type, const char* name, IContext* context) : IResourceSchema(type, name, context) { }
};

DEFINE_ICOLLECTION_SCHEMA(IResourcesSchema) < IResourceSchema > {
protected:
	IResourcesSchema(const char* type, const char* name, IContext* context) : ICollectionSchema(type, name, context) { }
};
DEFINE_COLLECTION_SCHEMA(ResourcesSchema, IResourcesSchema) {
public:
	ResourcesSchema(const char* type, const char* name, IContext* context) : IResourcesSchema(type, name, context) { }
};

DEFINE_IMODEL(IResource, IResourceSchema) {
	VIRTUAL_COMPONENT(SILK_LL, Value)
protected:
	IResource(IIdentity* id, ISchema* schema) : IModel(id, schema) { }
};
DEFINE_MODEL(Resource, IResource) {
	COMPONENT(SILK_LL, Value)
public:
	Resource(IIdentity* id, ISchema* schema) : IResource(id, schema) {
		Value = new SILK_LL(0); 
	}
	~Resource() {
		delete Value;
	}
};

DEFINE_ICOLLECTION_MODEL(IResourceCollection) <IResource, IResourcesSchema> {
protected:
	IResourceCollection(IIdentity* id, ISchema* schema) : IModelCollection(id, schema) { }
};
DEFINE_COLLECTION_MODEL(ResourceCollection, IResourceCollection) {
public:
	ResourceCollection(IIdentity* id, ISchema* schema) : IResourceCollection(id, schema) {}
};

DEFINE_ISCHEMA(IGameSchema) {
	VIRTUAL_ACCESSOR(IRoshanStatusSchema, RoshanStatus)
	VIRTUAL_ACCESSOR(ITriggersSchema, Triggers)
	VIRTUAL_ACCESSOR(IResourcesSchema, Resources)
	VIRTUAL_ACCESSOR(SILK_INT, RessurectBaseTime)
	VIRTUAL_ACCESSOR(SILK_INT, RessurectExtraTime)
protected:
	IGameSchema(const char* type, const char* name, IContext* context) : IModelSchema(type, name, context) { }
};
DEFINE_SCHEMA(GameSchema, IGameSchema) {
	ACCESSOR(IRoshanStatusSchema, RoshanStatus)
	ACCESSOR(ITriggersSchema, Triggers)
	ACCESSOR(IResourcesSchema, Resources)
	ACCESSOR(SILK_INT, RessurectBaseTime)
	ACCESSOR(SILK_INT, RessurectExtraTime)
public:
	GameSchema(const char* type, const char* name, IContext* context) : IGameSchema(type, name, context) { }
	~GameSchema() { 
		delete RoshanStatus; 
		delete Triggers; 
		delete Resources; 
		delete RessurectBaseTime;
		delete RessurectExtraTime;
	}
};

DEFINE_IMODEL(IGameModel, IGameSchema) {
	VIRTUAL_COMPONENT(IRoshanStatusModel, RoshanStatus)
	VIRTUAL_COMPONENT(ITriggerCollection, Triggers)
	VIRTUAL_COMPONENT(IResourceCollection, Resources)
protected:
	IGameModel(IIdentity* id, ISchema* schema) : IModel(id, schema) { };
};
DEFINE_MODEL(GameModel, IGameModel) {
	COMPONENT(IRoshanStatusModel, RoshanStatus)
	COMPONENT(ITriggerCollection, Triggers)
	COMPONENT(IResourceCollection, Resources)
public:
	GameModel(IIdentity* id, ISchema* schema) : IGameModel(id, schema) {
		auto factory = Schema->GetContext()->GetFactory();
		auto guid = Schema->GetContext()->GetGuid();
		auto roshanStatusSchema = Schema->GetRoshanStatus();
		RoshanStatus = factory->Build<IRoshanStatusModel>(roshanStatusSchema->GetType()->GetValue(), guid->Get(), roshanStatusSchema);
		auto triggersSchema = Schema->GetTriggers();
		Triggers = factory->Build<ITriggerCollection>(triggersSchema->GetType()->GetValue(), guid->Get(), triggersSchema);
		auto resourcesSchema = Schema->GetResources();
		Resources = factory->Build<IResourceCollection>(resourcesSchema->GetType()->GetValue(), guid->Get(), resourcesSchema);
	}
	~GameModel() {
		delete RoshanStatus;
		delete Triggers;
		delete Resources;
	}
};

class ModelRegistrator {
public:
	void Register(GigaFactory* factories);
};