#pragma once
#include "fmod.hpp"

namespace Engine
{


	class Sound
	{
	public:
		Sound(){}
		~Sound();
		Sound(const std::string& filepath);
		bool Initialize(const std::string& filepath);
		void SetMinMaxDistance(float Min, float Max);
		void SetMode(FMOD_MODE mode);
		FMOD::Sound** GetSoundAddress()
		{
			return &m_Sound;
		}
		FMOD::Sound* Get()
		{
			return m_Sound;
		}
	private:
		FMOD::Sound* m_Sound;
	};

}