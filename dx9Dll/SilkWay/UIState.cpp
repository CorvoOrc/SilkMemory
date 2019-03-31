#include "UIState.h"

void UIStateRegistrator::Register(GigaFactory* factory) {
	auto uiStateSchema = new SilkFactory();
	uiStateSchema->Register("empty", new SchemaImplementator<EmptyUIStateSchema>());
	uiStateSchema->Register("label_alive", new SchemaImplementator<LabelAliveStateSchema>());
	uiStateSchema->Register("label_ressurect", new SchemaImplementator<LabelRessurectStateSchema>());
	uiStateSchema->Register("button_alive", new SchemaImplementator<ButtonAliveStateSchema>());
	uiStateSchema->Register("button_ressurect_base", new SchemaImplementator<ButtonRessurectBaseStateSchema>());
	uiStateSchema->Register("button_ressurect_extra", new SchemaImplementator<ButtonRessurectExtraStateSchema>());
	uiStateSchema->Register("states", new SchemaImplementator<UIStatesSchema>());
	factory->Register<UIStateSchema>(uiStateSchema);

	auto labelStateSchema = new SilkFactory();
	labelStateSchema->Register("alive", new SchemaImplementator<EmptyUIStateSchema>());
	labelStateSchema->Register("ressurect", new SchemaImplementator<EmptyUIStateSchema>());
	factory->Register<LabelStateSchema>(labelStateSchema);

	auto buttonStateSchema = new SilkFactory();
	buttonStateSchema->Register("alive", new SchemaImplementator<ButtonAliveStateSchema>());
	buttonStateSchema->Register("ressurect_base", new SchemaImplementator<ButtonRessurectBaseStateSchema>());
	buttonStateSchema->Register("ressurect_extra", new SchemaImplementator<ButtonRessurectExtraStateSchema>());
	factory->Register<ButtonStateSchema>(buttonStateSchema);

	auto collectionSchema = new SilkFactory();
	collectionSchema->Register("states", new SchemaImplementator<UIStatesSchema>());
	factory->Register<IUIStatesSchema>(collectionSchema);

	auto uiState = new SilkFactory();
	uiState->Register("empty", new ConcreteImplementator<EmptyUIState>);
	uiState->Register("label_alive", new ConcreteImplementator<LabelAliveState>);
	uiState->Register("label_ressurect", new ConcreteImplementator<LabelRessurectState>);
	uiState->Register("button_alive", new ConcreteImplementator<ButtonAliveState>);
	uiState->Register("button_ressurect_base", new ConcreteImplementator<ButtonRessurectBaseState>);
	uiState->Register("button_ressurect_extra", new ConcreteImplementator<ButtonRessurectExtraState>);
	uiState->Register("states", new ConcreteImplementator<UIStates>);
	factory->Register<UIState<>>(uiState);

	auto labelState = new SilkFactory();
	labelState->Register("alive", new ConcreteImplementator<LabelAliveState>());
	labelState->Register("ressurect", new ConcreteImplementator<LabelRessurectState>());
	factory->Register<LabelState>(labelState);

	auto buttonState = new SilkFactory();
	buttonState->Register("alive", new ConcreteImplementator<ButtonAliveState>());
	buttonState->Register("ressurect_base", new ConcreteImplementator<ButtonRessurectBaseState>());
	buttonState->Register("ressurect_extra", new ConcreteImplementator<ButtonRessurectExtraState>());
	factory->Register<ButtonState>(buttonState);

	auto collection = new SilkFactory();
	collection->Register("states", new ConcreteImplementator<UIStates>());
	factory->Register<IUIStates>(collection);
}