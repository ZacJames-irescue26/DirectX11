#pragma once
#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include <Sol2/sol.hpp>
#include <filesystem>

namespace Engine
{
	class Entity;
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

	class ScriptEngine
	{
	public:
		bool Initialize();

		bool LoadScript(Entity* entity, LuaScriptComponent& script);

		void CallOnCreate(Entity* entity, LuaScriptComponent& script);
		void CallOnUpdate(Entity* entity, LuaScriptComponent& script, float dt);
		sol::state& GetState() { return m_Lua; }

	private:
		void RegisterTypes();

		void RegisterInput();
		void RegisterController();
		void RegisterPhysics();
		void RegisterMath();
		void GenerateLuaAPIFile(const std::filesystem::path& path);
	private:
		sol::state m_Lua;
		std::vector<LuaTypeInfo> m_LuaTypes;
		bool m_Initialized;
	};
}