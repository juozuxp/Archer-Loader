#include "DynamicImage.h"
#include "ClientManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Randomizer.h"

DynamicImage::DynamicImage(void* ImageBuffer, unsigned long long ImageSize)
{
	this->ImageSize = ImageSize;
	this->ImageBuffer = ImageBuffer;
}

unsigned char DynamicImage::GetIndexFromRandom(bool* UsedFlags)
{
	while (true)
	{
		unsigned char FragIndex;

		FragIndex = Randomizer.GetNumber() % FragmentCount;
		if (!UsedFlags[FragIndex])
		{
			UsedFlags[FragIndex] = true;
			return FragIndex;
		}
	}

	return 0;
}

void DynamicImage::BuildStreamIndex()
{
	bool* UsedFlags;

	UsedFlags = (bool*)malloc(FragmentCount);
	memset(UsedFlags, 0, FragmentCount);

	IndexMap = LongIndex(FragmentCount);
	for (unsigned long i = 0; i < FragmentCount; i++)
	{
		unsigned char FragmentIndex = GetIndexFromRandom(UsedFlags);

		IndexMap.MapIndex(i, FragmentIndex);
	}
}

bool DynamicImage::StreamImage(ClientManager& Client)
{
	if (!DescribeStream(Client))
		return false;

	for (unsigned char i = 0; i < FragmentCount; i++)
	{
		unsigned char FragmentIndex = IndexMap[i];

		if (!Client.Send(ArcherType_Setup, GetIndexBuffer(FragmentIndex), GetIndexSize(FragmentIndex)))
			return false;
	}

	return true;
}

unsigned long long DynamicImage::GetIndexSize(unsigned char Index)
{
	if (Index == 0xFF || (Index + 1) == FragmentCount)
		return ImageSize - (FragmentSize * (FragmentCount - 1));

	return FragmentSize;
}

void* DynamicImage::GetIndexBuffer(unsigned char Index)
{
	return ((char*)ImageBuffer) + (((unsigned long long)Index) * FragmentSize);
}

bool DynamicImage::DescribeStream(ClientManager& Client)
{
	DynamicDescriptor Descriptor;

	Descriptor.Key = Randomizer::RandomNumber();

	Descriptor.SizeKey = ImageSize ^ Descriptor.Key;

	FragmentCount = ((Descriptor.Key >> 56) ^ (Descriptor.Key >> 48) ^ (Descriptor.Key >> 40) ^ (Descriptor.Key >> 32) ^ (Descriptor.Key >> 24) ^ (Descriptor.Key >> 16) ^ (Descriptor.Key >> 8) ^ Descriptor.Key) % 15;
	if (FragmentCount < 5)
		FragmentCount = 5;

	FragmentSize = ImageSize / FragmentCount;
	Randomizer = PseudoRandom(Randomizer::RandomNumber());

	BuildStreamIndex();

	IndexMap.ExportIndexMap(Descriptor.IndexBuffer);
	return Client.Send(ArcherType_Setup, &Descriptor, sizeof(DynamicDescriptor) - sizeof(DynamicDescriptor::IndexBuffer) + IndexMap.GetIndexMapSize());
}
