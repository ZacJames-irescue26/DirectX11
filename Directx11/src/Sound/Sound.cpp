#include "pch.h"
#include "Sound.h"

namespace Engine
{

	Sound::~Sound()
	{
		if (m_Sound)
		{
			m_Sound->release();
		}
	}

	void Sound::SetMinMaxDistance(float Min, float Max)
	{
		if (m_Sound)
		{
			m_Sound->set3DMinMaxDistance(Min, Max);
		}
	}

	void Sound::SetMode(FMOD_MODE mode)
	{
		if (m_Sound)
		{
			m_Sound->setMode(mode);
		}
	}

}