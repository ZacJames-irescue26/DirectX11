#pragma once
#include <filesystem>

namespace Engine
{
	class Project
	{
	public:

		static void SetEditorRoot(const std::filesystem::path& path)
		{
			s_EditorRoot = std::filesystem::absolute(path).generic_string();
		}

		static void SetProjectRoot(const std::filesystem::path& path)
		{
			s_ProjectRoot = std::filesystem::absolute(path).generic_string();
			s_AssetsRoot = (s_ProjectRoot / "Assets").generic_string();
		}
		static std::filesystem::path GetShaderPath(const std::filesystem::path& relativePath)
		{
			return (s_ProjectRoot / "CompiledShaders" / relativePath).generic_string();
		}
		static std::filesystem::path GetEditorShaderPath(const std::filesystem::path& relativePath)
		{
			return (s_EditorRoot / "CompiledShaders" / relativePath).generic_string();
		}
		static const std::filesystem::path& GetProjectRoot()
		{
			return s_ProjectRoot.generic_string();
		}

		static const std::filesystem::path& GetAssetsPath()
		{
			return s_AssetsRoot.generic_string();
		}

		static const std::filesystem::path& GetScriptsPath()
		{
			return (s_AssetsRoot / "Scripts").generic_string();
		}
		static std::filesystem::path ResolveAssetPath(const std::filesystem::path& relativePath)
		{
			return (s_AssetsRoot / relativePath).generic_string();
		}

	private:
		inline static std::filesystem::path s_EditorRoot = "Editor";
		inline static std::filesystem::path s_ProjectRoot = "Game";
		inline static std::filesystem::path s_AssetsRoot = "Game/Assets";
	};
}