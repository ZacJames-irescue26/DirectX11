#pragma once
#include <stdint.h>
#include <algorithm>
#include "pch.h"


namespace Engine
{

	static const int MAXBONEPERVERTEX = 4;

	struct VertexBoneData
	{
		uint32_t BoneIDs[MAXBONEPERVERTEX];
		float Weights[MAXBONEPERVERTEX];

		VertexBoneData()
		{
			for (int i = 0; i < MAXBONEPERVERTEX; ++i)
			{
				BoneIDs[i] = 0;
				Weights[i] = 0.0f;
			}
		}

		void AddBoneData(uint32_t boneID, float weight)
		{
			if (weight <= 0.0f)
				return;

			// fill empty slot first
			for (int i = 0; i < MAXBONEPERVERTEX; ++i)
			{
				if (Weights[i] == 0.0f)
				{
					BoneIDs[i] = boneID;
					Weights[i] = weight;
					return;
				}
			}

			// no empty slot: replace the weakest influence if this one is stronger
			int minIndex = 0;
			float minWeight = Weights[0];

			for (int i = 1; i < MAXBONEPERVERTEX; ++i)
			{
				if (Weights[i] < minWeight)
				{
					minWeight = Weights[i];
					minIndex = i;
				}
			}

			if (weight > minWeight)
			{
				BoneIDs[minIndex] = boneID;
				Weights[minIndex] = weight;
			}
		}

		void SortByWeightDesc()
		{
			for (int i = 0; i < MAXBONEPERVERTEX - 1; ++i)
			{
				for (int j = i + 1; j < MAXBONEPERVERTEX; ++j)
				{
					if (Weights[j] > Weights[i])
					{
						std::swap(Weights[i], Weights[j]);
						std::swap(BoneIDs[i], BoneIDs[j]);
					}
				}
			}
		}

		void Normalize()
		{
			float sum = 0.0f;
			for (int i = 0; i < MAXBONEPERVERTEX; ++i)
				sum += Weights[i];

			if (sum > 1e-8f)
			{
				for (int i = 0; i < MAXBONEPERVERTEX; ++i)
					Weights[i] /= sum;
			}
		}
	};
	struct BoneInfo
	{
		XMMATRIX OffsetMatrix;
		XMMATRIX FinalTransformation;

		BoneInfo(const XMMATRIX& Offset)
		{
			OffsetMatrix = Offset;
			FinalTransformation = XMMatrixIdentity();
		}

	};
}