#pragma once
#include "Mouse\MouseClass.h"
namespace Engine
{

class ScriptMouse
{
public:
	static void SetMouse(MouseClass* mouse)
	{
		s_Mouse = mouse;
	}

	static int GetDeltaX()
	{
		return s_Mouse ? s_Mouse->GetDeltaX() : 0;
	}

	static int GetDeltaY()
	{
		return s_Mouse ? s_Mouse->GetDeltaY() : 0;
	}
	static bool IsMouseLeftDown()
	{
		return s_Mouse->IsLeftDown();
	}
	static bool IsMouseRightDown()
	{
		return s_Mouse->IsRightDown();
	}
private:
	inline static MouseClass* s_Mouse = nullptr;
};
}