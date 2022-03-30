#include "DynamicImage.h"
#include "ClientManager.h"

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

DynamicImage::~DynamicImage()
{
	if (this->ImageBuffer)
		free(this->ImageBuffer);
}

DynamicImage::DynamicImage(const DynamicImage& Copy)
{
	this->ImageSize = Copy.ImageSize;
	this->FragmentSize = Copy.FragmentSize;
	this->FragmentCount = Copy.FragmentCount;
	this->ImageBuffer = malloc(this->ImageSize);

	memcpy(this->ImageBuffer, Copy.ImageBuffer, this->ImageSize);
}

DynamicImage::DynamicImage(DynamicImage&& Move)
{
	this->ImageSize = Move.ImageSize;
	this->ImageBuffer = Move.ImageBuffer;
	this->FragmentSize = Move.FragmentSize;
	this->FragmentCount = Move.FragmentCount;

	Move.ImageBuffer = 0;
}

DynamicImage& DynamicImage::operator=(const DynamicImage& Copy)
{
	this->ImageSize = Copy.ImageSize;
	this->FragmentSize = Copy.FragmentSize;
	this->FragmentCount = Copy.FragmentCount;
	this->ImageBuffer = malloc(this->ImageSize);

	memcpy(this->ImageBuffer, Copy.ImageBuffer, this->ImageSize);

	return *this;
}

DynamicImage& DynamicImage::operator=(DynamicImage&& Move)
{
	this->ImageSize = Move.ImageSize;
	this->ImageBuffer = Move.ImageBuffer;
	this->FragmentSize = Move.FragmentSize;
	this->FragmentCount = Move.FragmentCount;

	Move.ImageBuffer = 0;

	return *this;
}

DynamicImage::DynamicImage(bool Stream)
{


	if (DecodeStream())
	{
		if (!BuildImage())
		{
			free(ImageBuffer);

			ImageBuffer = 0;
			ImageSize = 0;
		}
	}



}

bool DynamicImage::BuildImage()
{
	ArcherType Type;

	for (unsigned char i = 0; i < FragmentCount; i++)
	{
		unsigned char FragmentIndex = IndexMap[i];

		Type = ArcherType_None;
		while (Type != ArcherType_Setup)
		{
			Type = ClientManager::Receive(GetIndexBuffer(FragmentIndex), GetIndexSize(FragmentIndex), 0);
			if (Type == ArcherType_Disconnect)
				return false;
		}
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

bool DynamicImage::DecodeStream()
{
	DynamicDescriptor Descriptor;
	ArcherType Type;

	Type = ArcherType_None;
	while (Type != ArcherType_Setup)
	{
		Type = ClientManager::Receive(&Descriptor, sizeof(Descriptor), 0);
		if (Type == ArcherType_Disconnect)
			return false;
	}

	ImageSize = Descriptor.Key ^ Descriptor.SizeKey;
	ImageBuffer = malloc(ImageSize);

	FragmentCount = ((Descriptor.Key >> 56) ^ (Descriptor.Key >> 48) ^ (Descriptor.Key >> 40) ^ (Descriptor.Key >> 32) ^ (Descriptor.Key >> 24) ^ (Descriptor.Key >> 16) ^ (Descriptor.Key >> 8) ^ Descriptor.Key) % 15;
	if (FragmentCount < 5)
		FragmentCount = 5;

	FragmentSize = ImageSize / FragmentCount;
	IndexMap = LongIndex(FragmentCount, Descriptor.IndexBuffer);

	return true;
}

void* DynamicImage::GetImage()
{
	return ImageBuffer;
}

unsigned long long DynamicImage::GetImageSize()
{
	return ImageSize;
}