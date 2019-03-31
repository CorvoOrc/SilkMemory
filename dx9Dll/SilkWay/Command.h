#pragma once
#include "Model.h"

class ICommand {
public:
	virtual void Execute() = 0;
};

class KillRoshanCommand : public ICommand {
public:
	KillRoshanCommand(IGameModel* _model) {
		model = _model;
	}
	void Execute() {
		auto currentState = model->GetRoshanStatus()->GetStates()->GetCurrent();
		auto schema = currentState->GetSchema();
		if (schema->CheckName("alive") || schema->CheckName("alive_manual"))
			model->GetTriggers()->GetByName("roshan_killed")->Activate();
		else if (schema->CheckName("ressurect_extra"))
			model->GetTriggers()->GetByName("roshan_alive_manual")->Activate();
	}
protected:
	IGameModel* model;
};

class ICommands : public silk_data::Queue<ICommand*>, public ICommand {
protected:
	ICommands(IGameModel* _model) : Queue(), ICommand() { }
};

class Commands : public ICommands {
public:
	Commands(IGameModel* _model) : ICommands(_model) { }
	void Execute() {
		foreach(Length()) {
			auto command = Dequeue();
			command->Execute();
			delete command;
		}
	}
};