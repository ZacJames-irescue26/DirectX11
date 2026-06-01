#pragma once
#include "pch.h"

namespace Engine
{

	struct PositionKey
	{
		PositionKey(float time, XMFLOAT3 pos)
			:mTime(time), Position(pos)
		{
		}
		float mTime;
		XMFLOAT3 Position;
	};
	struct RotationKey
	{
		RotationKey(float time, XMFLOAT4 rot)
			:mTime(time), Rotation(rot)
		{
		}
		float mTime;
		XMFLOAT4 Rotation;
	};
	struct ScaleKey
	{
		ScaleKey(float time, XMFLOAT3 scale)
			:mTime(time), Scale(scale)
		{
		}
		float mTime;
		XMFLOAT3 Scale;
	};

	struct AnimationChannel
	{
		std::string BoneName;

		std::vector<PositionKey> Positions;
		std::vector<RotationKey> Rotations;
		std::vector<ScaleKey> Scales;
	};

	struct AnimationClip
	{
		std::string Name;
		double Duration = 0.0;
		double TicksPerSecond = 25.0;
		std::string filepath;
		std::vector<AnimationChannel> Channels;
	};

}