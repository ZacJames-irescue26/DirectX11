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
		void SetListenerAttribs(XMFLOAT3 pos, XMFLOAT3 vel, XMFLOAT3 forward, XMFLOAT3 up);
		void DestroySounds(Entity* entity);
	private:

		FMOD::System* m_System;
	};
}