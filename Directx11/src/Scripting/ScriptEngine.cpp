#include "pch.h"
#include "ScriptEngine.h"
#include "Entity.h"
#include "Components/Components.h"
#include "src/Scene/Project.h"
#include <Keyboard\KeyboardClass.h>
#include "ScriptPhysics.h"
#include "SciptMouse.h"
#include "ScriptMath.h"
#include "ScriptController.h"

namespace Engine
{
	bool ScriptEngine::Initialize()
	{
		if (m_Initialized)
			return true;

		m_Lua.open_libraries(
			sol::lib::base,
			sol::lib::math,
			sol::lib::table,
			sol::lib::string
		);

		RegisterTypes();
		RegisterInput();
		RegisterController();
		RegisterPhysics();
		RegisterMath();
		m_Initialized = true;
		return true;
	}

	void ScriptEngine::RegisterTypes()
	{
		m_Lua.new_usertype<DirectX::XMFLOAT3>(
			"Vec3",
			sol::constructors<
			DirectX::XMFLOAT3(),
			DirectX::XMFLOAT3(float, float, float)
			>(),
			"x", &DirectX::XMFLOAT3::x,
			"y", &DirectX::XMFLOAT3::y,
			"z", &DirectX::XMFLOAT3::z
		);
		m_Lua.new_usertype<DirectX::XMFLOAT4>(
			"Quat",
			sol::constructors<
			DirectX::XMFLOAT4(),
			DirectX::XMFLOAT4(float, float, float, float)
			>(),
			"x", &DirectX::XMFLOAT4::x,
			"y", &DirectX::XMFLOAT4::y,
			"z", &DirectX::XMFLOAT4::z,
			"w", &DirectX::XMFLOAT4::w
		);


		m_Lua.new_usertype<TransformComponent>(
			"TransformComponent",

			"Position", &TransformComponent::Position,
			"Scale", &TransformComponent::Scale,

			// UI/editor Euler storage if you want Lua access
			"RotationEuler", &TransformComponent::Rotation,

			// direct quaternion storage if you want Lua access
			"RotationQuat", &TransformComponent::RotationQuat,

			"SetRotationEuler", &TransformComponent::SetRotationEulerRadians,
			"SetRotationQuat", &TransformComponent::SetRotationQuat,
			"GetRotationQuat", &TransformComponent::GetRotationQuat
		);

		m_Lua.new_usertype<Entity>(
			"Entity",
			"GetName", &Entity::GetName,
			"GetTransform", [](Entity& entity) -> TransformComponent*
			{
				return entity.GetComponent<TransformComponent>();
			},
			"GetChildByName", &Entity::GetChildByName
		);
		m_LuaTypes.push_back({
	   "Entity",
	   {},
	   {
		   { "GetName", "fun(self: Entity): string", "string" },
		   { "GetTransform", "fun(self: Entity): TransformComponent|nil", "TransformComponent|nil" }
	   }
			});

	}
	void ScriptEngine::RegisterInput()
	{
		sol::table input = m_Lua.create_named_table("Input");

		input.set_function("IsKeyDown", &Engine::ScriptInput::IsKeyDownName);
		input.set_function("IsKeyCodeDown", &Engine::ScriptInput::IsKeyDown);


		sol::table mouse = m_Lua.create_named_table("Mouse");

		mouse.set_function("GetDeltaX", &ScriptMouse::GetDeltaX);
		mouse.set_function("GetDeltaY", &ScriptMouse::GetDeltaY);


		sol::table key = m_Lua.create_named_table("Key");

		key["W"] = 'W';
		key["A"] = 'A';
		key["S"] = 'S';
		key["D"] = 'D';

		key["Space"] = VK_SPACE;
		key["Shift"] = VK_SHIFT;
		key["Ctrl"] = VK_CONTROL;
		key["Alt"] = VK_MENU;

		key["Left"] = VK_LEFT;
		key["Right"] = VK_RIGHT;
		key["Up"] = VK_UP;
		key["Down"] = VK_DOWN;
	}
	void ScriptEngine::RegisterController()
	{
		sol::table controller = m_Lua.create_named_table("Controller");

		controller.set_function("IsConnected", &ScriptController::IsConnected);
		controller.set_function("LeftX", &ScriptController::LeftX);
		controller.set_function("LeftY", &ScriptController::LeftY);
		controller.set_function("RightX", &ScriptController::RightX);
		controller.set_function("RightY", &ScriptController::RightY);
		controller.set_function("IsButtonDown", &ScriptController::IsButtonDown);

		sol::table gamepad = m_Lua.create_named_table("GamepadButton");

		gamepad["A"] = XINPUT_GAMEPAD_A;
		gamepad["B"] = XINPUT_GAMEPAD_B;
		gamepad["X"] = XINPUT_GAMEPAD_X;
		gamepad["Y"] = XINPUT_GAMEPAD_Y;
		gamepad["LB"] = XINPUT_GAMEPAD_LEFT_SHOULDER;
		gamepad["RB"] = XINPUT_GAMEPAD_RIGHT_SHOULDER;
		gamepad["Back"] = XINPUT_GAMEPAD_BACK;
		gamepad["Start"] = XINPUT_GAMEPAD_START;
		gamepad["LeftThumb"] = XINPUT_GAMEPAD_LEFT_THUMB;
		gamepad["RightThumb"] = XINPUT_GAMEPAD_RIGHT_THUMB;
		gamepad["DPadUp"] = XINPUT_GAMEPAD_DPAD_UP;
		gamepad["DPadDown"] = XINPUT_GAMEPAD_DPAD_DOWN;
		gamepad["DPadLeft"] = XINPUT_GAMEPAD_DPAD_LEFT;
		gamepad["DPadRight"] = XINPUT_GAMEPAD_DPAD_RIGHT;
	}
	void ScriptEngine::RegisterPhysics()
	{
		sol::table physics = m_Lua.create_named_table("Physics");

		physics.set_function("SetRotationEuler", &ScriptPhysics::SetRotationEuler);
		physics.set_function("SetRotationQuat", &ScriptPhysics::SetRotationQuat);
		physics.set_function("SetPosition", &ScriptPhysics::SetPosition);
		physics.set_function("SetLinearVelocity", &ScriptPhysics::SetLinearVelocity);
		physics.set_function("AddForce", &ScriptPhysics::AddForce);
		physics.set_function("AddImpulse", &ScriptPhysics::AddImpulse);
		physics.set_function("GetLinearVelocity", &ScriptPhysics::GetLinearVelocity);
	}

	void ScriptEngine::RegisterMath()
	{
		sol::table mathEx = m_Lua.create_named_table("MathEx");

		mathEx.set_function("ForwardFromYawPitch", &ScriptMath::ForwardFromYawPitch);
		mathEx.set_function("RightFromYaw", &ScriptMath::RightFromYaw);
		mathEx.set_function("Normalize", &ScriptMath::Normalize);

		mathEx.set_function("QuatFromEuler", [](const DirectX::XMFLOAT3& euler)
			{
				DirectX::XMVECTOR q =
					DirectX::XMQuaternionRotationRollPitchYaw(
						euler.x,
						euler.y,
						euler.z
					);

				DirectX::XMFLOAT4 out;
				DirectX::XMStoreFloat4(&out, DirectX::XMQuaternionNormalize(q));
				return out;
			});
	}

	void ScriptEngine::GenerateLuaAPIFile(const std::filesystem::path& path)
	{
		std::ofstream out(path);

		out << "-- Auto-generated by Engine. Do not edit manually.\n\n";

		for (const LuaTypeInfo& type : m_LuaTypes)
		{
			out << "---@class " << type.Name << "\n";

			for (const auto& [fieldName, fieldType] : type.Fields)
			{
				out << "---@field " << fieldName << " " << fieldType << "\n";
			}

			out << type.Name << " = {}\n\n";

			for (const LuaMethodInfo& method : type.Methods)
			{
				out << "---@type " << method.Signature << "\n";
				out << "function " << type.Name << ":" << method.Name << "() end\n\n";
			}
		}

		out << "---@type Entity\n";
		out << "entity = nil\n";
	}
	bool ScriptEngine::LoadScript(Entity* entity, LuaScriptComponent& script)
	{
		if (!entity)
			return false;

		std::filesystem::path fullPath =
			Project::ResolveAssetPath(script.ScriptPath);

		script.Environment = sol::environment(m_Lua, sol::create, m_Lua.globals());
		script.Environment["entity"] = entity;

		sol::load_result loaded = m_Lua.load_file(fullPath.string());

		if (!loaded.valid())
		{
			sol::error err = loaded;
			ErrorLogger::LogToDebug(err.what());
			return false;
		}

		sol::protected_function fileFunc = loaded;
		sol::protected_function_result result = fileFunc(script.Environment);

		if (!result.valid())
		{
			sol::error err = result;
			ErrorLogger::LogToDebug(err.what());
			return false;
		}

		script.OnCreate = script.Environment["OnCreate"];
		script.OnUpdate = script.Environment["OnUpdate"];

		return true;
	}

	void ScriptEngine::CallOnCreate(Entity* entity, LuaScriptComponent& script)
	{
		if (!script.OnCreate.valid())
			return;

		sol::protected_function_result result = script.OnCreate(entity);

		if (!result.valid())
		{
			sol::error err = result;
			OutputDebugStringA(err.what());
			OutputDebugStringA("\n");
		}
	}

	void ScriptEngine::CallOnUpdate(Entity* entity, LuaScriptComponent& script, float dt)
	{
		if (!script.OnUpdate.valid())
			return;

		sol::protected_function_result result = script.OnUpdate(entity, dt);

		if (!result.valid())
		{
			sol::error err = result;
			OutputDebugStringA(err.what());
			OutputDebugStringA("\n");
		}
	}
}