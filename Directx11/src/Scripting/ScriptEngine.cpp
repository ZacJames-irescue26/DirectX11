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
#include <DirectXMath.h>
#include "Physics/RaytraceInfo.h"
#include "ErrorLogger.h"
#include "Scene/Scene.h"

namespace Engine
{
	bool ScriptEngine::Initialize(const EngineContext& context)
	{
		SetContext(context);
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
		RegisterLogging();
		RegisterAudio();
		m_Initialized = true;
		return true;
	}

	void ScriptEngine::RegisterTypes()
	{
		m_Lua.new_usertype<XMFLOAT3>(
			"Vec3",
			sol::constructors<
			XMFLOAT3(),
			XMFLOAT3(float, float, float)
			>(),
			"x", &XMFLOAT3::x,
			"y", &XMFLOAT3::y,
			"z", &XMFLOAT3::z
		);
		m_Lua.new_usertype<XMFLOAT4>(
			"Quat",
			sol::constructors<
			XMFLOAT4(),
			XMFLOAT4(float, float, float, float)
			>(),
			"x", &XMFLOAT4::x,
			"y", &XMFLOAT4::y,
			"z", &XMFLOAT4::z,
			"w", &XMFLOAT4::w
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
			"GetUUID", &Entity::GetUUIDString,
			"GetName", &Entity::GetName,
			"GetTransform", [](Entity& entity) -> TransformComponent*
			{
				return entity.GetComponent<TransformComponent>();
			},
			"GetCamera", [](Entity& entity) -> CameraComponent*
			{
				return entity.GetComponent<CameraComponent>();
			},
			"GetChildByName", &Entity::GetChildByName
			"DestroyEntity", [this](Entity& entity) ->void
			{
				Scene* scene = m_Context.ActiveScene;

				if (!scene)
					return;

				scene->QueueDestroyEntity(entity.GetUUID());
			},
		);

		m_Lua.set_function("DestroyEntityByString",[this](const std::string& uuidString){
			Scene* scene = m_Context.ActiveScene;

			if (!scene)
				return;

			try
			{
				UUID uuid = static_cast<UUID>(std::stoull(uuidString));
				scene->QueueDestroyEntity(uuid);
			}
			catch (...)
			{
				// Optional: log invalid UUID string
			}
		});

		m_Lua.new_usertype<CameraComponent>(
			"CameraComponent",

			"primary", &CameraComponent::Primary,

			"GetForward",
			[](CameraComponent& self) -> XMFLOAT3
			{
				return self.ForwardVector;
			}
		);
		m_Lua.new_usertype<LuaRaycastHit>(
			"RaycastHit",
			"hit", &LuaRaycastHit::Hit,
			"entity", &LuaRaycastHit::Entity,
			"position", &LuaRaycastHit::Position,
			"normal", &LuaRaycastHit::Normal,
			"distance", &LuaRaycastHit::Distance
		);

		m_Lua.new_usertype<LuaOverlapHit>(
			"OverlapHit",
			"entity", &LuaOverlapHit::Entity,
			"position", &LuaOverlapHit::Position,
			"normal", &LuaOverlapHit::Normal,
			"distance", &LuaOverlapHit::Distance
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
		mouse.set_function("IsLeftMouseDown", &ScriptMouse::IsMouseLeftDown);
		mouse.set_function("IsRightMouseDown", &ScriptMouse::IsMouseRightDown);

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
		controller.set_function("LeftTrigger", &ScriptController::LeftTrigger);
		controller.set_function("RightTrigger", &ScriptController::RightTrigger);

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
	void ScriptEngine::RegisterLogging()
	{
		sol::table Logging = m_Lua.create_named_table("Logging");

		Logging.set_function("LogToWindow", [](const std::string& message) {ErrorLogger::Log(message); });
		Logging.set_function("LogToConsol", [](const std::string& message) {ErrorLogger::LogToDebug(message); });
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

		physics.set_function("Raycast",
			[this](const XMFLOAT3& origin,
				const XMFLOAT3& direction,
				float maxDistance,
				sol::optional<std::vector<std::string>> excludedUUIDs) -> LuaRaycastHit
			{
				Scene* scene = m_Context.ActiveScene;
				PhysicsEngine* physicsEngine = m_Context.Physics;

				LuaRaycastHit luaHit = {};

				if (!scene || !physicsEngine)
					return luaHit;

				RayCastInfo info = {};
				info.Origin = origin;
				info.Direction = direction;
				info.MaxDistance = maxDistance;

				if (excludedUUIDs)
				{
					for (const std::string& uuidString : *excludedUUIDs)
					{
						try
						{
							uint64_t uuid = std::stoull(uuidString);

							// Use whichever matches your ExcludedEntityMap type:
							info.ExcludedEntities.insert(uuid);
							// or:
							// info.ExcludedEntities[uuid] = true;
						}
						catch (...)
						{
							// Invalid UUID string. Ignore it or log a warning.
						}
					}
				}

				RayHit hit = {};

				bool didHit = physicsEngine->CastRay(&info,hit, m_Context.ActiveScene);

				luaHit.Hit = didHit;

				if (didHit)
				{
					luaHit.Entity = std::to_string(hit.HitEntity);
					luaHit.Position = hit.Position;
					luaHit.Normal = hit.Normal;
					luaHit.Distance = hit.Distance;
				}

				return luaHit;
			});

		physics.set_function("OverlapSphere", 
		[this](const XMFLOAT3& origin,
			float radius,
			sol::optional<sol::table> excludedTable) -> std::vector<LuaOverlapHit>
		{
				std::vector<LuaOverlapHit> luaHits;

				Scene* scene = m_Context.ActiveScene;
				PhysicsEngine* physicsEngine = m_Context.Physics;

				if (!scene || !physicsEngine)
					return luaHits;

				SphereOverlapInfo info = {};
				info.Origin = origin;
				info.Radius = radius;

				if (excludedTable)
				{
					sol::table table = *excludedTable;

					for (auto& pair : table)
					{
						sol::object value = pair.second;

						if (!value.is<std::string>())
							continue;

						uint64_t uuid = 0;

						if (Utils::TryParseUUID(value.as<std::string>(), uuid))
						{
							// Use the version that matches your ExcludedEntityMap:
							info.ExcludedEntities.insert(uuid);
							// or:
							// info.ExcludedEntities[uuid] = true;
						}
					}
				}

				RayHit* hits = nullptr;

				int32_t count = physicsEngine->OverlapShape(
					scene,
					&info,
					&hits
				);

				luaHits.reserve(count);

				for (int32_t i = 0; i < count; ++i)
				{
					LuaOverlapHit out = {};
					out.Entity = std::to_string(hits[i].HitEntity);
					out.Position = hits[i].Position;
					out.Normal = hits[i].Normal;
					out.Distance = hits[i].Distance;

					luaHits.push_back(out);
				}

				return luaHits;
		});

	}

	void ScriptEngine::RegisterMath()
	{
		sol::table mathEx = m_Lua.create_named_table("MathEx");

		mathEx.set_function("ForwardFromYawPitch", &ScriptMath::ForwardFromYawPitch);
		mathEx.set_function("RightFromYaw", &ScriptMath::RightFromYaw);
		mathEx.set_function("Normalize", &ScriptMath::Normalize);

		mathEx.set_function("QuatFromEuler", [](const XMFLOAT3& euler)
			{
				XMVECTOR q =
					XMQuaternionRotationRollPitchYaw(
						euler.x,
						euler.y,
						euler.z
					);

				XMFLOAT4 out;
				XMStoreFloat4(&out, XMQuaternionNormalize(q));
				return out;
			});
	}
	void ScriptEngine::RegisterAudio()
	{
		sol::table audio = m_Lua.create_named_table("Audio");
		audio.set_function("PlayEntityAudio", [this](Entity* entity) {

			Scene* scene = m_Context.ActiveScene;
			PhysicsEngine* physicsEngine = m_Context.Physics;

			if (!scene || !physicsEngine || !entity)
				return;
			
			scene->PlayAudio(entity);


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

		sol::protected_function_result result = script.OnCreate(*entity);

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

		sol::protected_function_result result = script.OnUpdate(*entity, dt);

		if (!result.valid())
		{
			sol::error err = result;
			OutputDebugStringA(err.what());
			OutputDebugStringA("\n");
		}
	}
}