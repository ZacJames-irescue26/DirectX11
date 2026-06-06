#pragma once
#include "fmod.hpp"
#include "Channel.h"
#include "src/Entity.h"

namespace Engine
{
	class SoundSystem
	{
	public:
		~SoundSystem();
		bool Initialize();
		void CreateSound(const std::string& filepath, FMOD::Sound** inSound);
		void PlaySound(FMOD::Sound* sound, bool Paused, Channel outchannel);
		void Update();
		void DestroySounds(Entity* entity);
	private:

		FMOD::System* m_System;
	};
}