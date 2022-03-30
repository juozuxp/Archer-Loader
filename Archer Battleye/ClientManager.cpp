#include "ClientManager.h"
#include <BasicUtilities.h>
#include <intrin.h>
#include "Randomizer.h"

#include "CryptString.h"

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

ClientManager ClientManager::Instance;

bool ClientManager::Initialize(SOCKET ClientSocket)
{
	Instance.ReceiveSize = 0;
	Instance.ReceiveOffset = 0;
	Instance.ClientSocket = /*Connect();*/ ClientSocket;
	Instance.SendBuffer = GrowthBuffer(0x1000);

	return true;
}

SOCKET ClientManager::Connect()
{
	WSADATA Data;
	addrinfo* ResultInfo;
	addrinfo AddressInfo;

	SOCKET ConnectSocket;

	unsigned long Result;

	Result = WSAStartup(0x0202, &Data);
	if (Result)
		return INVALID_SOCKET;

	memset(&AddressInfo, 0, sizeof(AddressInfo));

	AddressInfo.ai_family = AF_INET;
	AddressInfo.ai_socktype = SOCK_STREAM;
	AddressInfo.ai_protocol = IPPROTO_TCP;
	AddressInfo.ai_flags = AI_PASSIVE;

#ifdef _DEBUG
	Result = getaddrinfo("5.206.227.166", __CS("16385"), &AddressInfo, &ResultInfo);
#else
	Result = getaddrinfo(__CS("5.206.227.166")/*"localhost"*/, __CS("16384"), &AddressInfo, &ResultInfo);
#endif

	if (Result)
	{
		WSACleanup();

		return INVALID_SOCKET;
	}

	for (addrinfo* Current = ResultInfo; Current; Current = ResultInfo->ai_next)
	{
		ConnectSocket = socket(Current->ai_family, Current->ai_socktype, Current->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET)
		{
			freeaddrinfo(ResultInfo);
			WSACleanup();

			return INVALID_SOCKET;
		}

		Result = connect(ConnectSocket, Current->ai_addr, Current->ai_addrlen);
		if (Result)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;

			continue;
		}

		break;
	}

	freeaddrinfo(ResultInfo);

	if (ConnectSocket == INVALID_SOCKET)
	{
		WSACleanup();

		return INVALID_SOCKET;
	}

	return ConnectSocket;
}

unsigned long long ClientManager::GetDataHash(void* Data, unsigned long Size)
{


	unsigned long long ResultHash;

	ResultHash = 0;
	while (Size > sizeof(unsigned long long))
	{
		if (*((unsigned char*)Data) / 7)
			ResultHash ^= *(unsigned long long*)Data;
		else if (*((unsigned char*)Data) / 4)
			ResultHash ^= _rotl64(*(unsigned long long*)Data, 15);
		else if (*((unsigned char*)Data) / 2)
			ResultHash ^= _rotl64(*(unsigned long long*)Data, 45);
		else
			ResultHash ^= _rotl64(*(unsigned long long*)Data, 25);

		Size -= sizeof(unsigned long long);
		Data = (char*)Data + sizeof(unsigned long long);
	}

	if (Size >= sizeof(unsigned long))
	{
		ResultHash ^= *(unsigned long*)Data;

		Size -= sizeof(unsigned long);
		Data = (char*)Data + sizeof(unsigned long);
	}

	if (Size >= sizeof(unsigned short))
	{
		ResultHash ^= *(unsigned short*)Data;

		Size -= sizeof(unsigned short);
		Data = (char*)Data + sizeof(unsigned short);
	}

	if (Size >= sizeof(unsigned char))
	{
		ResultHash ^= *(unsigned char*)Data;

		Size -= sizeof(unsigned char);
		Data = (char*)Data + sizeof(unsigned char);
	}



	return _rotl64(ResultHash, 0x7);
}

unsigned long long ClientManager::GetDataHashOffset(void* Data, unsigned long Size)
{
	unsigned long long Hash;

	Hash = GetDataHash(Data, Size);
	return ServerStaticHash[((unsigned char)Hash) % GetArraySize(ServerStaticHash)] ^ Hash;
}

bool ClientManager::VerifyHash(void* Data, unsigned long Size, unsigned long long SubHash)
{
	unsigned long long Hash;

	Hash = GetDataHash(Data, Size);
	return ClientStaticHash[((unsigned char)Hash) % GetArraySize(ClientStaticHash)] == (Hash ^ SubHash);
}

void ClientManager::EncryptDescriptor(ArcherPacket* Descriptor)
{
	unsigned char EncStartIndex;
	unsigned long PacketProgress;
	unsigned long long* RunPacket;
	const unsigned long long* LocalEncryption;

	EncStartIndex = ((Descriptor->Key >> 17) ^ (Descriptor->Key >> 21) ^ (Descriptor->Key >> 49) ^ (Descriptor->Key >> 3)) % GetArraySize(ServerEncryption);

	LocalEncryption = &ServerEncryption[EncStartIndex];
	PacketProgress = DESCRIPTOR_SIZE - sizeof(ArcherPacket::Key);
	RunPacket = (unsigned long long*)((char*)Descriptor + sizeof(ArcherPacket::Key));
	for (unsigned char i = EncStartIndex; (PacketProgress & ~(sizeof(unsigned long long) - 1)); i++, RunPacket++, LocalEncryption++, PacketProgress -= sizeof(unsigned long long))
	{
		unsigned long long Hash;

		if (i >= GetArraySize(ServerEncryption))
		{
			i = 0;
			LocalEncryption = ServerEncryption;
		}

		Hash = *LocalEncryption ^ Descriptor->Key;

		if (Hash)
			*RunPacket ^= Hash;
		else
			*RunPacket ^= *LocalEncryption;
	}

	if (PacketProgress >= sizeof(unsigned long))
	{
		unsigned long Hash;

		Hash = ServerEncryption[0] ^ (Descriptor->Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned long*)RunPacket) ^= Hash;
		else
			(*(unsigned long*)RunPacket) ^= ServerEncryption[0];

		PacketProgress -= sizeof(unsigned long);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned long));
	}

	if (PacketProgress >= sizeof(unsigned short))
	{
		unsigned short Hash;

		Hash = ServerEncryption[1] ^ (Descriptor->Key & ((1 << 16) - 1));
		if (Hash)
			(*(unsigned short*)RunPacket) ^= Hash;
		else
			(*(unsigned short*)RunPacket) ^= ServerEncryption[1];

		PacketProgress -= sizeof(unsigned short);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned short));
	}

	if (PacketProgress >= sizeof(unsigned char))
	{
		unsigned char Hash;

		Hash = ServerEncryption[2] ^ (Descriptor->Key & ((1 << 8) - 1));
		if (Hash)
			(*(unsigned char*)RunPacket) ^= Hash;
		else
			(*(unsigned char*)RunPacket) ^= ServerEncryption[2];

		PacketProgress -= sizeof(unsigned char);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned char));
	}
}

void ClientManager::EncryptPacket(ArcherPacket* Packet)
{


	unsigned char EncStartIndex;
	unsigned long PacketProgress;
	unsigned long long* RunPacket;
	const unsigned long long* LocalEncryption;

	Packet->Key = Randomizer::RandomNumber();
	EncStartIndex = ((Packet->Key >> 13) ^ (Packet->Key >> 27) ^ (Packet->Key >> 45) ^ (Packet->Key >> 7)) % GetArraySize(ServerEncryption);

	PacketProgress = Packet->Size;

	EncryptDescriptor(Packet);

	RunPacket = (unsigned long long*)Packet->Buffer;
	LocalEncryption = &ServerEncryption[EncStartIndex];
	for (unsigned char i = EncStartIndex; (PacketProgress & ~(sizeof(unsigned long long) - 1)); i++, RunPacket++, LocalEncryption++, PacketProgress -= sizeof(unsigned long long))
	{
		unsigned long long Hash;

		if (i >= GetArraySize(ServerEncryption))
		{
			i = 0;
			LocalEncryption = ServerEncryption;
		}

		Hash = *LocalEncryption ^ Packet->Key;

		if (Hash)
			*RunPacket ^= Hash;
		else
			*RunPacket ^= *LocalEncryption;
	}

	if (PacketProgress >= sizeof(unsigned long))
	{
		unsigned long Hash;

		Hash = ServerEncryption[0] ^ (Packet->Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned long*)RunPacket) ^= Hash;
		else
			(*(unsigned long*)RunPacket) ^= ServerEncryption[0];

		PacketProgress -= sizeof(unsigned long);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned long));
	}

	if (PacketProgress >= sizeof(unsigned short))
	{
		unsigned short Hash;

		Hash = ServerEncryption[1] ^ (Packet->Key & ((1 << 16) - 1));
		if (Hash)
			(*(unsigned short*)RunPacket) ^= Hash;
		else
			(*(unsigned short*)RunPacket) ^= ServerEncryption[1];

		PacketProgress -= sizeof(unsigned short);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned short));
	}



	if (PacketProgress >= sizeof(unsigned char))
	{
		unsigned char Hash;

		Hash = ServerEncryption[2] ^ (Packet->Key & ((1 << 8) - 1));
		if (Hash)
			(*(unsigned char*)RunPacket) ^= Hash;
		else
			(*(unsigned char*)RunPacket) ^= ServerEncryption[2];

		PacketProgress -= sizeof(unsigned char);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned char));
	}
}

void ClientManager::DecryptDescriptor(ArcherPacket* Descriptor)
{
	unsigned char EncStartIndex;
	unsigned long PacketProgress;
	unsigned long long* RunPacket;
	const unsigned long long* LocalEncryption;

	EncStartIndex = ((Descriptor->Key >> 17) ^ (Descriptor->Key >> 21) ^ (Descriptor->Key >> 49) ^ (Descriptor->Key >> 3)) % GetArraySize(ClientEncryption);

	LocalEncryption = &ClientEncryption[EncStartIndex];
	PacketProgress = DESCRIPTOR_SIZE - sizeof(ArcherPacket::Key);
	RunPacket = (unsigned long long*)((char*)Descriptor + sizeof(ArcherPacket::Key));
	for (unsigned char i = EncStartIndex; (PacketProgress & ~(sizeof(unsigned long long) - 1)); i++, RunPacket++, LocalEncryption++, PacketProgress -= sizeof(unsigned long long))
	{
		unsigned long long Hash;

		if (i >= GetArraySize(ClientEncryption))
		{
			i = 0;
			LocalEncryption = ClientEncryption;
		}

		Hash = *LocalEncryption ^ Descriptor->Key;

		if (Hash)
			*RunPacket ^= Hash;
		else
			*RunPacket ^= *LocalEncryption;
	}

	if (PacketProgress >= sizeof(unsigned long))
	{
		unsigned long Hash;

		Hash = ClientEncryption[0] ^ (Descriptor->Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned long*)RunPacket) ^= Hash;
		else
			(*(unsigned long*)RunPacket) ^= ClientEncryption[0];

		PacketProgress -= sizeof(unsigned long);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned long));
	}

	if (PacketProgress >= sizeof(unsigned short))
	{
		unsigned short Hash;

		Hash = ClientEncryption[1] ^ (Descriptor->Key & ((1 << 16) - 1));
		if (Hash)
			(*(unsigned short*)RunPacket) ^= Hash;
		else
			(*(unsigned short*)RunPacket) ^= ClientEncryption[1];

		PacketProgress -= sizeof(unsigned short);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned short));
	}

	if (PacketProgress >= sizeof(unsigned char))
	{
		unsigned char Hash;

		Hash = ClientEncryption[2] ^ (Descriptor->Key & ((1 << 8) - 1));
		if (Hash)
			(*(unsigned char*)RunPacket) ^= Hash;
		else
			(*(unsigned char*)RunPacket) ^= ClientEncryption[2];

		PacketProgress -= sizeof(unsigned char);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned char));
	}
}

void ClientManager::DecryptPacket(unsigned long long Key, void* Data, unsigned long Size)
{
	unsigned char EncStartIndex;
	unsigned long PacketProgress;
	unsigned long long* RunPacket;
	const unsigned long long* LocalEncryption;

	EncStartIndex = ((Key >> 13) ^ (Key >> 27) ^ (Key >> 45) ^ (Key >> 7)) % GetArraySize(ClientEncryption);

	PacketProgress = Size;
	RunPacket = (unsigned long long*)Data;
	LocalEncryption = &ClientEncryption[EncStartIndex];
	for (unsigned char i = EncStartIndex; (PacketProgress & ~(sizeof(unsigned long long) - 1)); i++, RunPacket++, LocalEncryption++, PacketProgress -= sizeof(unsigned long long))
	{
		unsigned long long Hash;

		if (i >= GetArraySize(ClientEncryption))
		{
			i = 0;
			LocalEncryption = ClientEncryption;
		}

		Hash = *LocalEncryption ^ Key;

		if (Hash)
			*RunPacket ^= Hash;
		else
			*RunPacket ^= *LocalEncryption;
	}

	if (PacketProgress >= sizeof(unsigned long))
	{
		unsigned long Hash;

		Hash = ClientEncryption[0] ^ (Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned long*)RunPacket) ^= Hash;
		else
			(*(unsigned long*)RunPacket) ^= ClientEncryption[0];

		PacketProgress -= sizeof(unsigned long);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned long));
	}

	if (PacketProgress >= sizeof(unsigned short))
	{
		unsigned short Hash;

		Hash = ClientEncryption[1] ^ (Key & ((1 << 16) - 1));
		if (Hash)
			(*(unsigned short*)RunPacket) ^= Hash;
		else
			(*(unsigned short*)RunPacket) ^= ClientEncryption[1];

		PacketProgress -= sizeof(unsigned short);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned short));
	}

	if (PacketProgress >= sizeof(unsigned char))
	{
		unsigned char Hash;

		Hash = ClientEncryption[2] ^ (Key & ((1 << 8) - 1));
		if (Hash)
			(*(unsigned char*)RunPacket) ^= Hash;
		else
			(*(unsigned char*)RunPacket) ^= ClientEncryption[2];

		PacketProgress -= sizeof(unsigned char);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned char));
	}
}

void ClientManager::Disconnect()
{
	shutdown(Instance.ClientSocket, SD_BOTH);
}

bool ClientManager::ReceiveData(void* Buffer, unsigned long Size, bool Progress)
{


	unsigned long BufferProgress;

	if (!Size)
		return true;

	BufferProgress = 0;
	if (ReceiveSize - ReceiveOffset)
	{
		if ((ReceiveSize - ReceiveOffset) >= Size)
		{
			memcpy(Buffer, ReceiveBuffer + ReceiveOffset, Size);
			if (Progress)
				ReceiveOffset += Size;

			return true;
		}

		memcpy(Buffer, ReceiveBuffer + ReceiveOffset, (ReceiveSize - ReceiveOffset));
		BufferProgress += ReceiveSize - ReceiveOffset;
	}

	ReceiveOffset = 0;
	if (Progress)
	{
		while (true)
		{
			ReceiveSize = recv(ClientSocket, (char*)ReceiveBuffer, sizeof(ReceiveBuffer), 0);
			if (!ReceiveSize || RESULT_INVALID(ReceiveSize))
				return false;

			if (ReceiveSize < (Size - BufferProgress))
			{
				memcpy((char*)Buffer + BufferProgress, ReceiveBuffer, ReceiveSize);
				BufferProgress += ReceiveSize;
			}
			else
			{
				memcpy((char*)Buffer + BufferProgress, ReceiveBuffer, Size - BufferProgress);
				ReceiveOffset = Size - BufferProgress;

				return true;
			}
		}
	}
	else
	{
		ReceiveSize = recv(ClientSocket, (char*)ReceiveBuffer + BufferProgress, sizeof(ReceiveBuffer) - BufferProgress, 0) + BufferProgress;
		if (!ReceiveSize || RESULT_INVALID(ReceiveSize))
			return false;

		memcpy(ReceiveBuffer, Buffer, BufferProgress);
		memcpy(((char*)Buffer) + BufferProgress, ReceiveBuffer + BufferProgress, ReceiveSize < Size ? (ReceiveSize - BufferProgress) : (Size - BufferProgress));
	}



	return true;
}

bool ClientManager::GetNearestDescriptor(ArcherPacket* Descriptor, bool Progress)
{


	if (!ReceiveData(Descriptor, DESCRIPTOR_SIZE, Progress))
		return false;

	DecryptDescriptor(Descriptor);



	return true;
}

ArcherType ClientManager::ReadIntoBuffer(void* Buffer, unsigned long Size)
{


	ArcherPacket Descriptor;
	void* DataBuffer;

	if (!GetNearestDescriptor(&Descriptor, true))
		return ArcherType_Disconnect;

	if (Size < Descriptor.Size)
		DataBuffer = malloc(Descriptor.Size);
	else
	{
		DataBuffer = Buffer;
		Size = Descriptor.Size;
	}

	if (!ReceiveData(DataBuffer, Descriptor.Size, true))
		return ArcherType_Disconnect;

	DecryptPacket(Descriptor.Key, DataBuffer, Descriptor.Size);
	if (!VerifyHash(DataBuffer, Descriptor.Size, Descriptor.HashOffset))
		Descriptor.Type = ArcherType_None;

	if (DataBuffer != Buffer)
	{
		memcpy(Buffer, DataBuffer, Size);
		free(DataBuffer);
	}



	return Descriptor.Type;
}

bool ClientManager::Send(ArcherType Type, void* Buffer, unsigned long Size)
{


	unsigned long Result;
	ArcherPacket* Allocated;

	Allocated = (ArcherPacket*)Instance.SendBuffer.Get(Size + DESCRIPTOR_SIZE);

	Allocated->Type = Type;
	Allocated->Size = Size;
	Allocated->HashOffset = Instance.GetDataHashOffset(Buffer, Size);

	memcpy(Allocated->Buffer, Buffer, Size);

	Instance.EncryptPacket(Allocated);	
	
	do
	{
		Result = send(Instance.ClientSocket, (const char*)Allocated, Size + DESCRIPTOR_SIZE, 0);
		if (!Result)
			Sleep(1000);
	} while (!Result);
	


	return RESULT_VALID(Result);
}

ArcherType ClientManager::Receive(void* Buffer, unsigned long Size, unsigned long* ReceiveSize)
{
	ArcherPacket PacketDescriptor;

	if (ReceiveSize)
	{
		if (!Instance.GetNearestDescriptor(&PacketDescriptor))
			return ArcherType_Disconnect;

		*ReceiveSize = PacketDescriptor.Size;
	}

	return Instance.ReadIntoBuffer(Buffer, Size);
}

void ClientManager::Duplicate(unsigned long ProcessID, WSAPROTOCOL_INFOA* ProtocolInfo)
{
	WSADuplicateSocketA(Instance.ClientSocket, ProcessID, ProtocolInfo);
}