#pragma once
#include "KeyboardEvent.h"

class KeyboardClass
{
public:
	KeyboardClass();
	bool KeyIsPressed(const unsigned char keycode);
	bool KeyBufferIsEmpty();
	bool CharBufferIsEmpty();
	KeyboardEvent ReadKey();
	unsigned char ReadChar();
	void OnKeyPressed(const unsigned char key);
	void OnKeyReleased(const unsigned char key);
	void OnChar(const unsigned char key);
	void EnableAutoRepeatKeys();
	void DisableAutoRepeatKeys();
	void EnableAutoRepeatChars();
	void DisableAutoRepeatChars();
	bool IsKeysAutoRepeat();
	bool IsCharsAutoRepeat();
private:
	bool autoRepeatKeys = false;
	bool autoRepeatChars = false;
	bool keyStates[256];
	std::queue<KeyboardEvent> keyBuffer;
	std::queue<unsigned char> charBuffer;
};

namespace Engine
{
	class ScriptInput
	{
	public:
		static void SetKeyboard(KeyboardClass* keyboard)
		{
			s_Keyboard = keyboard;
		}

		static bool IsKeyDown(int keyCode)
		{
			if (!s_Keyboard)
				return false;

			if (keyCode < 0 || keyCode > 255)
				return false;

			return s_Keyboard->KeyIsPressed(static_cast<unsigned char>(keyCode));
		}

		static bool IsKeyDownName(const std::string& key)
		{
			if (!s_Keyboard || key.empty())
				return false;

			if (key.length() == 1)
			{
				unsigned char keyCode =
					static_cast<unsigned char>(std::toupper(key[0]));

				return s_Keyboard->KeyIsPressed(keyCode);
			}

			if (key == "Space") return IsKeyDown(VK_SPACE);
			if (key == "Shift") return IsKeyDown(VK_SHIFT);
			if (key == "Ctrl")  return IsKeyDown(VK_CONTROL);
			if (key == "Alt")   return IsKeyDown(VK_MENU);
			if (key == "Left")  return IsKeyDown(VK_LEFT);
			if (key == "Right") return IsKeyDown(VK_RIGHT);
			if (key == "Up")    return IsKeyDown(VK_UP);
			if (key == "Down")  return IsKeyDown(VK_DOWN);

			return false;
		}

	private:
		inline static KeyboardClass* s_Keyboard = nullptr;
	};
}