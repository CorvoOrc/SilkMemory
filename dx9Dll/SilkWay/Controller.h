#pragma once
#include "Event.h"
#include "MemoryLogger.h"
#include "Chrono.h"
#include "Command.h"
#include "Model.h"
#include "View.h"
#include "UIState.h"

class IController {
public:
	virtual ~IController() {}
	virtual void Update() = 0;
	virtual void SetDevice(IDirect3DDevice9* device) = 0;
};

class GameController : public IController {
	ACCESSOR(SILK_GUID, Guid)
public:
	GameController(ILogger* _logger) {
		logger = _logger;
		chrono = new Chrono();
		Guid = new SILK_GUID();
		factory = new GigaFactory();

		auto modelRegistrator = ModelRegistrator();
		modelRegistrator.Register(factory);
		modelContext = new ModelContext(Guid, logger, chrono, factory, &Model);
		InitModelSchema(&modelSchema);
		Model = factory->Build<IGameModel>(modelSchema->GetType()->GetValue(), Guid->Get(), modelSchema);

		commands = new Commands(Model);

		auto viewRegistrator = ViewRegistrator();
		viewRegistrator.Register(factory);
		viewContext = new ViewContext(Guid, logger, chrono, factory);
		InitViewSchema(&viewSchema);
		canvas = factory->Build<ICanvas>(viewSchema->GetType()->GetValue(), Guid->Get(), viewSchema);
		statusLabel = (ILabel*)canvas->Get("roshan_status_label");
		killButton = (IButton*)canvas->Get("roshan_kill_button");

		auto uiStateRegistrator = UIStateRegistrator();
		uiStateRegistrator.Register(factory);
		uiStateContext = new UIStateContext(Guid, logger, chrono, factory, Model);
		InitUIStateSchema(&labelStateSchema, &buttonStateSchema);
		statusLabelStates = factory->Build<IUIStates>(labelStateSchema->GetType()->GetValue(), Guid->Get(), labelStateSchema);
		statusLabelStates->SetElement((IView<IViewSchema>*)statusLabel);
		killButtonStates = factory->Build<IUIStates>(buttonStateSchema->GetType()->GetValue(), Guid->Get(), buttonStateSchema);
		killButtonStates->SetElement((IView<IViewSchema>*)killButton);

		Attach();
	}
	~GameController() {
		Detach();
		delete viewSchema;
		delete modelSchema;
		delete Guid;
		delete factory;
		delete modelContext;
		delete viewContext;
		delete statesChangedCallback;
		delete buttonClickedCallback;
		delete canvas;
		delete commands;
		delete chrono;
		delete device;
	}
	void Update() {
		HandleInput();
		RunCommands();
		UpdateModel();
		Draw();
		chrono->Tick();
	}
	void SetDevice(IDirect3DDevice9* _device) { 
		device = new D9Device(_device);
		canvas->SetDevice(device);
	}
private:
	void HandleInput() {
		canvas->HandleInput();
	}
	void RunCommands() {
		commands->Execute();
	}
	void UpdateModel() {
		Model->GetRoshanStatus()->Resolve();
		Model->GetTriggers()->Reset();
	}
	void Draw() {
		canvas->Draw();
		canvas->Release();
	}
	void OnStatesChanged(IEventArgs* args) {
		statusLabelStates->Update();
		killButtonStates->Update();
	}
	void OnKillRoshanClicked(IEventArgs* args) {
		ICommand* command = new KillRoshanCommand(Model);
		commands->Enqueue(command);
	}
	void Attach() {
		statesChangedCallback = new MemberCallback<GameController, IEventArgs>(&GameController::OnStatesChanged, this);
		Model->GetRoshanStatus()->GetStates()->GetCurrentChanged()->Add(statesChangedCallback);
		buttonClickedCallback = new MemberCallback<GameController, IEventArgs>(&GameController::OnKillRoshanClicked, this);
		killButton->GetClickedEvent()->Add(buttonClickedCallback);
	}
	void Detach() {
		Model->GetRoshanStatus()->GetStates()->GetCurrentChanged()->Remove(statesChangedCallback);
		killButton->GetClickedEvent()->Remove(buttonClickedCallback);
	}
	void InitModelSchema(IGameSchema** schema);
	void InitViewSchema(ICanvasSchema** schema);
	void InitUIStateSchema(IUIStatesSchema** labelSchema, IUIStatesSchema** buttonSchema);
private:
	ILogger* logger;
	IChrono* chrono;
	GigaFactory* factory;

	ICommands* commands;
	IGameModel* Model;
	ICanvas* canvas;
	ILabel* statusLabel;
	IButton* killButton;

	IUIStates* statusLabelStates;
	IUIStates* killButtonStates;

	ModelContext* modelContext;
	ViewContext* viewContext;
	UIStateContext* uiStateContext;
	IGameSchema* modelSchema;
	ICanvasSchema* viewSchema;
	IUIStatesSchema* labelStateSchema;
	IUIStatesSchema* buttonStateSchema;
	ICallback* statesChangedCallback;
	ICallback* buttonClickedCallback;

	IGraphicDeviceHolder* device;
};