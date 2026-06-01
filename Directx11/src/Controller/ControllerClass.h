#pragma once
#include <Windows.h>
#include <Xinput.h>
#include <DirectXMath.h>
#include <algorithm>

namespace Engine
{
	class ControllerClass
	{
	public:
		void Update()
		{
			ZeroMemory(&m_State, sizeof(XINPUT_STATE));

			DWORD result = XInputGetState(m_Index, &m_State);
			m_Connected = result == ERROR_SUCCESS;
		}

		bool IsConnected() const
		{
			return m_Connected;
		}

		bool IsButtonDown(WORD button) const
		{
			if (!m_Connected)
				return false;

			return (m_State.Gamepad.wButtons & button) != 0;
		}

		float GetLeftX() const
		{
			return NormalizeThumb(m_State.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		}

		float GetLeftY() const
		{
			return NormalizeThumb(m_State.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		}

		float GetRightX() const
		{
			return NormalizeThumb(m_State.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		}

		float GetRightY() const
		{
			return NormalizeThumb(m_State.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		}

		float GetLeftTrigger() const
		{
			return NormalizeTrigger(m_State.Gamepad.bLeftTrigger);
		}

		float GetRightTrigger() const
		{
			return NormalizeTrigger(m_State.Gamepad.bRightTrigger);
		}

		void SetIndex(DWORD index)
		{
			m_Index = index;
		}

	private:
		static float NormalizeThumb(SHORT value, SHORT deadzone)
		{
			if (std::abs(value) < deadzone)
				return 0.0f;

			float normalized = 0.0f;

			if (value > 0)
				normalized = static_cast<float>(value - deadzone) / static_cast<float>(32767 - deadzone);
			else
				normalized = static_cast<float>(value + deadzone) / static_cast<float>(32768 - deadzone);

			return std::clamp(normalized, -1.0f, 1.0f);
		}

		static float NormalizeTrigger(BYTE value)
		{
			if (value < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				return 0.0f;

			return static_cast<float>(value - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) /
				static_cast<float>(255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
		}

	private:
		DWORD m_Index = 0;
		bool m_Connected = false;
		XINPUT_STATE m_State = {};
	};
}