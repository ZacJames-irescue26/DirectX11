#pragma once
#include <memory>
#include "EngineInclude.h"

using namespace Engine;
class Application : public EngineInit
{
public:
	void Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	void OnCreate();
	void OnUpdate();

private:

	
};