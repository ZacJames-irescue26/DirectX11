#include "Gamepch.h"
#include "EngineInclude.h"
#include "Application.h"
int APIENTRY main(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)

{
	Application app;
	app.Initialize(hInstance, "Title", "MyWindowClass", 800, 600);
	app.OnCreate();
	app.OnUpdate();
	return 0;
}