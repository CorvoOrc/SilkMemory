#include "Model.h"

void ModelRegistrator::Register(GigaFactory* factory) {
	auto requirement = new SilkFactory();
	requirement->Register("true", new SchemaImplementator<TrueRequirement>);
	requirement->Register("false", new SchemaImplementator<FalseRequirement>);
	requirement->Register("roshan_killed", new SchemaImplementator<RoshanKilledRequirement>);
	requirement->Register("roshan_alive_manual", new SchemaImplementator<RoshanAliveManualRequirement>);
	requirement->Register("time", new SchemaImplementator<TimeRequirement>);
	requirement->Register("roshan_state", new SchemaImplementator<RoshanStateRequirement>);
	factory->Register<IRequirement>(requirement);

	auto action = new SilkFactory();
	action->Register("action", new SchemaImplementator<EmptyAction>);
	action->Register("set_current_time", new SchemaImplementator<SetCurrentTimeAction>);
	factory->Register<IAction>(action);

	auto transition = new SilkFactory();
	transition->Register("transition", new SchemaImplementator<TransitionSchema>);
	factory->Register<ITransitionSchema>(transition);

	auto transitions = new SilkFactory();
	transitions->Register("transitions", new SchemaImplementator<TransitionsSchema>);
	factory->Register<ITransitionsSchema>(transitions);

	auto stateSchema = new SilkFactory();
	stateSchema->Register("state", new SchemaImplementator<StateSchema>);
	factory->Register<IStateSchema>(stateSchema);

	auto statesSchema = new SilkFactory();
	statesSchema->Register("states", new SchemaImplementator<StatesSchema>);
	factory->Register<IStatesSchema>(statesSchema);

	auto roshanStatusSchema = new SilkFactory();
	roshanStatusSchema->Register("roshan_status", new SchemaImplementator<RoshanStatusSchema>);
	factory->Register<IRoshanStatusSchema>(roshanStatusSchema);

	auto triggerSchema = new SilkFactory();
	triggerSchema->Register("trigger", new SchemaImplementator<TriggerSchema>);
	factory->Register<ITriggerSchema>(triggerSchema);

	auto triggersSchema = new SilkFactory();
	triggersSchema->Register("triggers", new SchemaImplementator<TriggersSchema>);
	factory->Register<ITriggersSchema>(triggersSchema);

	auto resourceSchema = new SilkFactory();
	resourceSchema->Register("resource", new SchemaImplementator<ResourceSchema>);
	factory->Register<IResourceSchema>(resourceSchema);

	auto resourcesSchema = new SilkFactory();
	resourcesSchema->Register("resources", new SchemaImplementator<ResourcesSchema>);
	factory->Register<IResourcesSchema>(resourcesSchema);

	auto gameSchema = new SilkFactory();
	gameSchema->Register("game", new SchemaImplementator<GameSchema>);
	factory->Register<IGameSchema>(gameSchema);


	auto gameModel = new SilkFactory();
	gameModel->Register("game", new ConcreteImplementator<GameModel>);
	factory->Register<IGameModel>(gameModel);

	auto resources = new SilkFactory();
	resources->Register("resources", new ConcreteImplementator<ResourceCollection>);
	factory->Register<IResourceCollection>(resources);

	auto resource = new SilkFactory();
	resource->Register("resource", new ConcreteImplementator<Resource>);
	factory->Register<IResource>(resource);

	auto triggers = new SilkFactory();
	triggers->Register("triggers", new ConcreteImplementator<TriggerCollection>);
	factory->Register<ITriggerCollection>(triggers);

	auto trigger = new SilkFactory();
	trigger->Register("trigger", new ConcreteImplementator<Trigger>);
	factory->Register<ITrigger>(trigger);

	auto roshanStatus = new SilkFactory();
	roshanStatus->Register("roshan_status", new ConcreteImplementator<RoshanStatusModel>);
	factory->Register<IRoshanStatusModel>(roshanStatus);

	auto states = new SilkFactory();
	states->Register("states", new ConcreteImplementator<StatesModel>);
	factory->Register<IStatesModel>(states);

	auto state = new SilkFactory();
	state->Register("state", new ConcreteImplementator<StateModel>);
	factory->Register<IStateModel>(state);
}

bool RoshanKilledRequirement::Check() {
	auto context = GetContext();
	auto currentState = (*context->GetModel())->GetRoshanStatus()->GetStates()->GetCurrent();
	auto trigger = (*context->GetModel())->GetTriggers()->GetByName("roshan_killed");
	return (currentState->GetSchema()->CheckName("alive") || currentState->GetSchema()->CheckName("alive_manual"))
		&& trigger->IsActive();
}

bool RoshanAliveManualRequirement::Check() {
	auto context = GetContext();
	auto currentState = (*context->GetModel())->GetRoshanStatus()->GetStates()->GetCurrent();
	auto trigger = (*context->GetModel())->GetTriggers()->GetByName("roshan_alive_manual");
	return currentState->GetSchema()->CheckName("ressurect_extra") && trigger->IsActive();
}

bool TimeRequirement::Check() {
	auto context = GetContext();
	auto resource = (*context->GetModel())->GetResources()->GetByName(ResourceName->GetValue());
	return context->GetClock()->GetDiffS(*resource->GetValue()->GetValue()) >= *Offset->GetValue();
}

bool RoshanStateRequirement::Check() {
	auto context = GetContext();
	auto current = (*context->GetModel())->GetRoshanStatus()->GetStates()->GetCurrent();
	return current->GetSchema()->CheckName(StateName->GetValue());
}

void SetCurrentTimeAction::Make() {
	auto context = GetContext();
	auto resource = (*context->GetModel())->GetResources()->GetByName(ResourceName->GetValue());
	resource->SetValue(new SILK_LL(context->GetClock()->GetStamp()));
}