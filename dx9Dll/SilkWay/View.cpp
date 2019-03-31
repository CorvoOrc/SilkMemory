#include "View.h"

void ViewRegistrator::Register(GigaFactory* factory) {
	auto viewSchema = new SilkFactory();
	viewSchema->Register("canvas_d9", new SchemaImplementator<CanvasD9Schema>());
	viewSchema->Register("label_d9", new SchemaImplementator<LabelD9Schema>());
	viewSchema->Register("button_d9", new SchemaImplementator<ButtonD9Schema>());
	viewSchema->Register("elements", new SchemaImplementator<ElementCollectionSchema>());
	factory->Register<IViewSchema>(viewSchema);

	auto canvasSchema = new SilkFactory();
	canvasSchema->Register("canvas_d9", new SchemaImplementator<CanvasD9Schema>());
	factory->Register<ICanvasSchema>(canvasSchema);
	auto labelSchema = new SilkFactory();
	labelSchema->Register("label_d9", new SchemaImplementator<LabelD9Schema>());
	factory->Register<ILabelSchema>(labelSchema);
	auto buttonSchema = new SilkFactory();
	buttonSchema->Register("button_d9", new SchemaImplementator<ButtonD9Schema>());
	factory->Register<IButtonSchema>(buttonSchema);
	auto collectionSchema = new SilkFactory();
	collectionSchema->Register("elements", new SchemaImplementator<ElementCollectionSchema>());
	factory->Register<IViewCollectionSchema>(collectionSchema);

	auto view = new SilkFactory();
	view->Register("canvas_d9", new ConcreteImplementator<CanvasD9>());
	view->Register("label_d9", new ConcreteImplementator<LabelD9>());
	view->Register("button_d9", new ConcreteImplementator<ButtonD9>());
	view->Register("elements", new ConcreteImplementator<ElementCollection>());
	factory->Register<IView<IViewSchema>>(view);

	auto canvas = new SilkFactory();
	canvas->Register("canvas_d9", new ConcreteImplementator<CanvasD9>());
	factory->Register<ICanvas>(canvas);
	auto label = new SilkFactory();
	label->Register("label_d9", new ConcreteImplementator<LabelD9>());
	factory->Register<ILabel>(label);
	auto button = new SilkFactory();
	button->Register("button_d9", new ConcreteImplementator<ButtonD9>());
	factory->Register<IButton>(button);
	auto collection = new SilkFactory();
	collection->Register("elements", new ConcreteImplementator<ElementCollection>());
	factory->Register<IElementCollection>(collection);
}