#pragma once
#include "Controller\ControllerClass.h"

namespace Engine
{
	class ScriptController
	{
	public:
		static void SetController(Engine::ControllerClass* controller)
		{
			s_Controller = controller;
		}

		static bool IsConnected()
		{
			return s_Controller && s_Controller->IsConnected();
		}

		static float LeftX()
		{
			return s_Controller ? s_Controller->GetLeftX() : 0.0f;
		}

		static float LeftY()
		{
			return s_Controller ? s_Controller->GetLeftY() : 0.0f;
		}

		static float RightX()
		{
			return s_Controller ? s_Controller->GetRightX() : 0.0f;
		}

		static float RightY()
		{
			return s_Controller ? s_Controller->GetRightY() : 0.0f;
		}

		static bool IsButtonDown(int button)
		{
			return s_Controller && s_Controller->IsButtonDown(static_cast<WORD>(button));
		}

	private:
		inline static Engine::ControllerClass* s_Controller = nullptr;
	};
}