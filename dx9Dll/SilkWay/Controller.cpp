#include "Controller.h"

void GameController::InitModelSchema(IGameSchema** schema) {
	*schema = factory->Build<IGameSchema>("game", "game", "game_schema", modelContext);
	(*schema)->SetRessurectBaseTime(new SILK_INT(10));
	(*schema)->SetRessurectExtraTime(new SILK_INT(15));

	IStateSchema* aliveState = factory->Build<IStateSchema>("state", "state", "alive", modelContext);
	IStateSchema* ressurectBaseState = factory->Build<IStateSchema>("state", "state", "ressurect_base", modelContext);
	IStateSchema* ressurectExtraState = factory->Build<IStateSchema>("state", "state", "ressurect_extra", modelContext);

	ITransitionsSchema* transitionsAlive = factory->Build<ITransitionsSchema>("transitions", "transitions", "alive_transitions", modelContext);
	ITransitionSchema* transition1 = factory->Build<ITransitionSchema>("transition", "transition", "alive_to_ressurect_base", modelContext);
	transition1->SetFrom(new SILK_STRING(aliveState->GetName()->GetValue()));
	transition1->SetTo(new SILK_STRING(ressurectBaseState->GetName()->GetValue()));
	IRequirement* trans1Requirement = factory->Build<IRequirement>("roshan_killed", "roshan_killed", "requirement", modelContext);
	transition1->SetRequirement(trans1Requirement);
	IAction* timeAction = factory->Build<IAction>("set_current_time", "set_current_time", "action", modelContext);
	dynamic_cast<SetCurrentTimeAction*>(timeAction)->SetResourceName(new SILK_STRING("roshan_death_ts"));
	transition1->SetAction(timeAction);
	transitionsAlive->PushBack(transition1);
	aliveState->SetTransitions(transitionsAlive);

	ITransitionsSchema* transitionsResBase = factory->Build<ITransitionsSchema>("transitions", "transitions", "ressurect_base_transitions", modelContext);
	ITransitionSchema* transition2 = factory->Build<ITransitionSchema>("transition", "transition", "ressurect_base_to_ressurect_extra", modelContext);
	transition2->SetFrom(new SILK_STRING(ressurectBaseState->GetName()->GetValue()));
	transition2->SetTo(new SILK_STRING(ressurectExtraState->GetName()->GetValue()));
	IRequirement* trans2Requirement = factory->Build<IRequirement>("time", "time", "requirement", modelContext);
	dynamic_cast<TimeRequirement*>(trans2Requirement)->SetResourceName(new SILK_STRING("roshan_death_ts"));
	dynamic_cast<TimeRequirement*>(trans2Requirement)->SetOffset(new SILK_LL(10));
	transition2->SetRequirement(trans2Requirement);
	IAction* actionTr2 = factory->Build<IAction>("action", "action", "action", modelContext);
	transition2->SetAction(actionTr2);
	transitionsResBase->PushBack(transition2);
	ressurectBaseState->SetTransitions(transitionsResBase);

	ITransitionsSchema* transitionsResExtra = factory->Build<ITransitionsSchema>("transitions", "transitions", "ressurect_extra_transitions", modelContext);
	ITransitionSchema* transition3 = factory->Build<ITransitionSchema>("transition", "transition", "ressurect_extra_to_alive", modelContext);
	transition3->SetFrom(new SILK_STRING(ressurectExtraState->GetName()->GetValue()));
	transition3->SetTo(new SILK_STRING(aliveState->GetName()->GetValue()));
	IRequirement* trans3Requirement = factory->Build<IRequirement>("time", "time", "requirement", modelContext);
	dynamic_cast<TimeRequirement*>(trans3Requirement)->SetResourceName(new SILK_STRING("roshan_death_ts"));
	dynamic_cast<TimeRequirement*>(trans3Requirement)->SetOffset(new SILK_LL(15));
	transition3->SetRequirement(trans3Requirement);
	IAction* actionTr3 = factory->Build<IAction>("action", "action", "action", modelContext);
	transition3->SetAction(actionTr3);
	ITransitionSchema* transition4 = factory->Build<ITransitionSchema>("transition", "transition", "ressurect_extra_to_alive_manual", modelContext);
	transition4->SetFrom(new SILK_STRING(ressurectExtraState->GetName()->GetValue()));
	transition4->SetTo(new SILK_STRING(aliveState->GetName()->GetValue()));
	IRequirement* trans4Requirement = factory->Build<IRequirement>("roshan_alive_manual", "roshan_alive_manual", "requirement", modelContext);
	transition4->SetRequirement(trans4Requirement);
	IAction* actionTr4 = factory->Build<IAction>("action", "action", "action", modelContext);
	transition4->SetAction(actionTr4);
	transitionsResExtra->PushBack(transition3);
	transitionsResExtra->PushBack(transition4);
	ressurectExtraState->SetTransitions(transitionsResExtra);

	IStatesSchema* states = factory->Build<IStatesSchema>("states", "states", "states", modelContext);
	states->PushBack(aliveState);
	states->PushBack(ressurectBaseState);
	states->PushBack(ressurectExtraState);
	states->SetStartState(new SILK_STRING("alive"));
	IRoshanStatusSchema* roshanStatus = factory->Build<IRoshanStatusSchema>("roshan_status", "roshan_status", "roshan_status", modelContext);
	roshanStatus->SetStates(states);

	ITriggersSchema* triggers = factory->Build<ITriggersSchema>("triggers", "triggers", "triggers", modelContext);
	ITriggerSchema* roshKilledTrigger = factory->Build<ITriggerSchema>("trigger", "trigger", "roshan_killed", modelContext);
	ITriggerSchema* roshAliveManualTrigger = factory->Build<ITriggerSchema>("trigger", "trigger", "roshan_alive_manual", modelContext);
	triggers->PushBack(roshKilledTrigger);
	triggers->PushBack(roshAliveManualTrigger);

	IResourcesSchema* resources = factory->Build<IResourcesSchema>("resources", "resources", "resources", modelContext);
	IResourceSchema* roshanDeathTs = factory->Build<IResourceSchema>("resource", "resource", "roshan_death_ts", modelContext);
	resources->PushBack(roshanDeathTs);

	(*schema)->SetRoshanStatus(roshanStatus);
	(*schema)->SetTriggers(triggers);
	(*schema)->SetResources(resources);
}

void GameController::InitViewSchema(ICanvasSchema** schema) {
	*schema = factory->Build<ICanvasSchema>("canvas_d9", "canvas_d9", "canvas_d9", viewContext);

	IViewCollectionSchema* elements = factory->Build<IViewCollectionSchema>("elements", "elements", "elements", viewContext);
	(*schema)->SetElements(elements);

	ILabelSchema* labelSchema = factory->Build<ILabelSchema>("label_d9", "label_d9", "roshan_status_label", viewContext);
	labelSchema->SetRecLeft(new SILK_INT(30));
	labelSchema->SetRecTop(new SILK_INT(100));
	labelSchema->SetRecRight(new SILK_INT(230));
	labelSchema->SetRecDown(new SILK_INT(250));

	labelSchema->SetColorR(new SILK_FLOAT(1.0f));
	labelSchema->SetColorG(new SILK_FLOAT(1.0f));
	labelSchema->SetColorB(new SILK_FLOAT(1.0f));
	labelSchema->SetColorA(new SILK_FLOAT(1.0f));

	labelSchema->SetText(new SILK_STRING("Roshan status: alive\0"));
	elements->PushBack((IViewSchema*&)labelSchema);

	IButtonSchema* buttonSchema = factory->Build<IButtonSchema>("button_d9", "button_d9", "roshan_kill_button", viewContext);
	ILabelSchema* buttonLabelSchema = factory->Build<ILabelSchema>("label_d9", "label_d9", "button_text", viewContext);

	buttonLabelSchema->SetRecLeft(new SILK_INT(30));
	buttonLabelSchema->SetRecTop(new SILK_INT(115));
	buttonLabelSchema->SetRecRight(new SILK_INT(110));
	buttonLabelSchema->SetRecDown(new SILK_INT(130));

	buttonLabelSchema->SetColorR(new SILK_FLOAT(1.0f));
	buttonLabelSchema->SetColorG(new SILK_FLOAT(0.0f));
	buttonLabelSchema->SetColorB(new SILK_FLOAT(0.0f));
	buttonLabelSchema->SetColorA(new SILK_FLOAT(1.0f));

	buttonLabelSchema->SetText(new SILK_STRING("Kill Roshan\0"));

	buttonSchema->SetLabel(buttonLabelSchema);

	buttonSchema->SetBorderColorR(new SILK_INT(0));
	buttonSchema->SetBorderColorG(new SILK_INT(0));
	buttonSchema->SetBorderColorB(new SILK_INT(0));
	buttonSchema->SetBorderColorA(new SILK_INT(70));

	buttonSchema->SetFillColorR(new SILK_INT(255));
	buttonSchema->SetFillColorG(new SILK_INT(119));
	buttonSchema->SetFillColorB(new SILK_INT(0));
	buttonSchema->SetFillColorA(new SILK_INT(150));

	buttonSchema->SetPushColorR(new SILK_INT(0));
	buttonSchema->SetPushColorG(new SILK_INT(0));
	buttonSchema->SetPushColorB(new SILK_INT(0));
	buttonSchema->SetPushColorA(new SILK_INT(70));

	buttonSchema->SetBorder(new SILK_FLOAT(5));
	elements->PushBack((IViewSchema*&)buttonSchema);
}

void GameController::InitUIStateSchema(IUIStatesSchema** labelSchema, IUIStatesSchema** buttonSchema) {
	*labelSchema = factory->Build<IUIStatesSchema>("states", "states", "states", uiStateContext);
	auto labelAlive = factory->Build<UIStateSchema>("label_alive", "label_alive", "label_alive", uiStateContext);
	auto labelAliveRequirement = factory->Build<IRequirement>("roshan_state", "roshan_state", "roshan_state", modelContext);
	((RoshanStateRequirement*)labelAliveRequirement)->SetStateName(new SILK_STRING("alive"));
	labelAlive->SetRequirement(labelAliveRequirement);
	auto labelRessurect = factory->Build<UIStateSchema>("label_ressurect", "label_ressurect", "label_ressurect", uiStateContext);
	auto labelRessurectRequirement = factory->Build<IRequirement>("roshan_state", "roshan_state", "roshan_state", modelContext);
	((RoshanStateRequirement*)labelRessurectRequirement)->SetStateName(new SILK_STRING("ressurect_base"));
	labelRessurect->SetRequirement(labelRessurectRequirement);
	(*labelSchema)->PushBack(labelAlive);
	(*labelSchema)->PushBack(labelRessurect);

	*buttonSchema = factory->Build<IUIStatesSchema>("states", "states", "states", uiStateContext);
	auto buttonAlive = factory->Build<UIStateSchema>("button_alive", "button_alive", "button_alive", uiStateContext);
	auto buttonAliveRequirement = factory->Build<IRequirement>("roshan_state", "roshan_state", "roshan_state", modelContext);
	((RoshanStateRequirement*)buttonAliveRequirement)->SetStateName(new SILK_STRING("alive"));
	buttonAlive->SetRequirement(buttonAliveRequirement);
	auto buttonRessurectBase = factory->Build<UIStateSchema>("button_ressurect_base", "button_ressurect_base", "button_ressurect_base", uiStateContext);
	auto buttonRessurectBaseRequirement = factory->Build<IRequirement>("roshan_state", "roshan_state", "roshan_state", modelContext);
	((RoshanStateRequirement*)buttonRessurectBaseRequirement)->SetStateName(new SILK_STRING("ressurect_base"));
	buttonRessurectBase->SetRequirement(buttonRessurectBaseRequirement);
	auto buttonRessurectExtra = factory->Build<UIStateSchema>("button_ressurect_extra", "button_ressurect_extra", "button_ressurect_extra", uiStateContext);
	auto buttonRessurectExtraRequirement = factory->Build<IRequirement>("roshan_state", "roshan_state", "roshan_state", modelContext);
	((RoshanStateRequirement*)buttonRessurectExtraRequirement)->SetStateName(new SILK_STRING("ressurect_extra"));
	buttonRessurectExtra->SetRequirement(buttonRessurectExtraRequirement);
	(*buttonSchema)->PushBack(buttonAlive);
	(*buttonSchema)->PushBack(buttonRessurectBase);
	(*buttonSchema)->PushBack(buttonRessurectExtra);
}