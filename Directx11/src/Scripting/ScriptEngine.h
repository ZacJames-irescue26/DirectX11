#pragma once
#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include <Sol2/sol.hpp>
#include <filesystem>
#include "Sound/System.h"
namespace Engine
{
	class Entity;
	class Scene;
	class PhysicsEngine;
	struct LuaScriptComponent;
	struct LuaMethodInfo
	{
		std::string Name;
		std::string Signature;
		std::string ReturnType;
	};

	struct LuaTypeInfo
	{
		std::string Name;
		std::vector<std::pair<std::string, std::string>> Fields;
		std::vector<LuaMethodInfo> Methods;
	};
	struct EngineContext
	{
		EngineContext(Scene* Active, PhysicsEngine* Phys, SoundSystem* audio)
		:ActiveScene(Active), Physics(Phys), Audio(audio)
		{ }
		Scene* ActiveScene = nullptr;
		PhysicsEngine* Physics = nullptr;
		SoundSystem* Audio = nullptr;
	};
	class ScriptEngine
	{
	public:
		ScriptEngine(EngineContext Context) : m_Context(Context) {}
		bool Initialize(const EngineContext& context);
		void SetContext(const EngineContext& context)
		{
			m_Context = context;
		}
		bool LoadScript(Entity* entity, LuaScriptComponent& script);

		void CallOnCreate(Entity* entity, LuaScriptComponent& script);
		void CallOnUpdate(Entity* entity, LuaScriptComponent& script, float dt);
		sol::state& GetState() { return m_Lua; }

	private:
		void RegisterTypes();

		void RegisterInput();
		void RegisterController();
		void RegisterLogging();
		void RegisterPhysics();
		void RegisterMath();
		void RegisterAudio();
		void GenerateLuaAPIFile(const std::filesystem::path& path);
	private:
		EngineContext m_Context;
		sol::state m_Lua;
		std::vector<LuaTypeInfo> m_LuaTypes;
		bool m_Initialized = false;
	};
}