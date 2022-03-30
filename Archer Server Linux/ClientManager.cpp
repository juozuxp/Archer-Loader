#include "ClientManager.h"
#include "BasicUtilities.h"
#include "ServerHandler.h"
#include <x86intrin.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "Randomizer.h"

#ifndef _DEBUG
#define DEFAULT_PORT "848446"
#else
#define DEFAULT_PORT "8756"
#endif

constexpr unsigned long long ClientManager::ServerEncryption[];
constexpr unsigned long long ClientManager::ClientEncryption[];

constexpr unsigned long long ClientManager::ServerStaticHash[];
constexpr unsigned long long ClientManager::ClientStaticHash[];

volatile unsigned int ClientManager::WaitingConnection = ~0;

Mutex ClientManager::HandlerLock = Mutex();
HashMap<unsigned int, ClientManager*> ClientManager::IPHandlers = HashMap<unsigned int, ClientManager*>(0);

ClientManager::ClientManager(bool AllowSizeDifference)
{
	if (WaitingConnection == ~0)
		return;

	this->ReceiveSize = 0;
	this->ReceiveOffset = 0;
	this->ClientSocket = WaitingConnection;
	this->InfractSizeDifference = !AllowSizeDifference;
	this->IPAddress = GetSocketIPAddress(WaitingConnection);

	RegisterHandler();

	WaitingConnection = ~0;
}

void ClientManager::RegisterHandler()
{
	HandlerLock.WaitForLock();

	IPHandlers.Add(IPAddress, this);

	HandlerLock.Free();
}

void ClientManager::UnregisterHandler()
{
	HandlerLock.WaitForLock();

	IPHandlers.Remove(IPAddress);

	HandlerLock.Free();
}

void ClientManager::TerminateHandler(unsigned int IPAddress)
{
	ClientManager* Handler;

	HandlerLock.WaitForLock();

	Handler = IPHandlers[IPAddress];
	if (Handler)
		Handler->Terminate();

	HandlerLock.Free();
}

unsigned long long ClientManager::GetDataHash(void* Data, unsigned int Size)
{
	unsigned long long ResultHash;

	ResultHash = 0;
	while (Size > sizeof(unsigned long long))
	{
		if (*((unsigned char*)Data) / 7)
			ResultHash ^= *(unsigned long long*)Data;
		else if (*((unsigned char*)Data) / 4)
			ResultHash ^= __rolq(*(unsigned long long*)Data, 15);
		else if (*((unsigned char*)Data) / 2)
			ResultHash ^= __rolq(*(unsigned long long*)Data, 45);
		else
			ResultHash ^= __rolq(*(unsigned long long*)Data, 25);

		Size -= sizeof(unsigned long long);
		Data = (char*)Data + sizeof(unsigned long long);
	}

	if (Size >= sizeof(unsigned int))
	{
		ResultHash ^= *(unsigned int*)Data;

		Size -= sizeof(unsigned int);
		Data = (char*)Data + sizeof(unsigned int);
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

	return __rolq(ResultHash, 0x7);
}

unsigned long long ClientManager::GetDataHashOffset(void* Data, unsigned int Size)
{
	unsigned long long Hash;

	Hash = GetDataHash(Data, Size);
	return ClientStaticHash[((unsigned char)Hash) % GetArraySize(ClientStaticHash)] ^ Hash;
}

bool ClientManager::VerifyHash(void* Data, unsigned int Size, unsigned long long SubHash)
{
	unsigned long long Hash;

	Hash = GetDataHash(Data, Size);
	return ServerStaticHash[((unsigned char)Hash) % GetArraySize(ServerStaticHash)] == (Hash ^ SubHash);
}

void ClientManager::EncryptDescriptor(ArcherPacket* Descriptor)
{
	unsigned char EncStartIndex;
	unsigned int PacketProgress;
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

	if (PacketProgress >= sizeof(unsigned int))
	{
		unsigned int Hash;

		Hash = ClientEncryption[0] ^ (Descriptor->Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned int*)RunPacket) ^= Hash;
		else
			(*(unsigned int*)RunPacket) ^= ClientEncryption[0];

		PacketProgress -= sizeof(unsigned int);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned int));
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

void ClientManager::EncryptPacket(ArcherPacket* Packet)
{
	unsigned char EncStartIndex;
	unsigned int PacketProgress;
	unsigned long long* RunPacket;
	const unsigned long long* LocalEncryption;

	Packet->Key = Randomizer::RandomNumber();

	EncStartIndex = ((Packet->Key >> 13) ^ (Packet->Key >> 27) ^ (Packet->Key >> 45) ^ (Packet->Key >> 7)) % GetArraySize(ClientEncryption);

	PacketProgress = Packet->Size;

	EncryptDescriptor(Packet);

	RunPacket = (unsigned long long*)Packet->Buffer;
	LocalEncryption = &ClientEncryption[EncStartIndex];
	for (unsigned char i = EncStartIndex; (PacketProgress & ~(sizeof(unsigned long long) - 1)); i++, RunPacket++, LocalEncryption++, PacketProgress -= sizeof(unsigned long long))
	{
		unsigned long long Hash;

		if (i >= GetArraySize(ClientEncryption))
		{
			i = 0;
			LocalEncryption = ClientEncryption;
		}

		Hash = *LocalEncryption ^ Packet->Key;

		if (Hash)
			*RunPacket ^= Hash;
		else
			*RunPacket ^= *LocalEncryption;
	}

	if (PacketProgress >= sizeof(unsigned int))
	{
		unsigned int Hash;

		Hash = ClientEncryption[0] ^ (Packet->Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned int*)RunPacket) ^= Hash;
		else
			(*(unsigned int*)RunPacket) ^= ClientEncryption[0];

		PacketProgress -= sizeof(unsigned int);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned int));
	}

	if (PacketProgress >= sizeof(unsigned short))
	{
		unsigned short Hash;

		Hash = ClientEncryption[1] ^ (Packet->Key & ((1 << 16) - 1));
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

		Hash = ClientEncryption[2] ^ (Packet->Key & ((1 << 8) - 1));
		if (Hash)
			(*(unsigned char*)RunPacket) ^= Hash;
		else
			(*(unsigned char*)RunPacket) ^= ClientEncryption[2];

		PacketProgress -= sizeof(unsigned char);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned char));
	}
}

void ClientManager::DecryptDescriptor(ArcherPacket* Descriptor)
{
	unsigned char EncStartIndex;
	unsigned int PacketProgress;
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

	if (PacketProgress >= sizeof(unsigned int))
	{
		unsigned int Hash;

		Hash = ServerEncryption[0] ^ (Descriptor->Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned int*)RunPacket) ^= Hash;
		else
			(*(unsigned int*)RunPacket) ^= ServerEncryption[0];

		PacketProgress -= sizeof(unsigned int);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned int));
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

void ClientManager::DecryptPacket(unsigned long long Key, void* Data, unsigned int Size)
{
	unsigned char EncStartIndex;
	unsigned int PacketProgress;
	unsigned long long* RunPacket;
	const unsigned long long* LocalEncryption;

	EncStartIndex = ((Key >> 13) ^ (Key >> 27) ^ (Key >> 45) ^ (Key >> 7)) % GetArraySize(ServerEncryption);

	PacketProgress = Size;
	RunPacket = (unsigned long long*)Data;
	LocalEncryption = &ServerEncryption[EncStartIndex];
	for (unsigned char i = EncStartIndex; (PacketProgress & ~(sizeof(unsigned long long) - 1)); i++, RunPacket++, LocalEncryption++, PacketProgress -= sizeof(unsigned long long))
	{
		unsigned long long Hash;

		if (i >= GetArraySize(ServerEncryption))
		{
			i = 0;
			LocalEncryption = ServerEncryption;
		}

		Hash = *LocalEncryption ^ Key;

		if (Hash)
			*RunPacket ^= Hash;
		else
			*RunPacket ^= *LocalEncryption;
	}

	if (PacketProgress >= sizeof(unsigned int))
	{
		unsigned int Hash;

		Hash = ServerEncryption[0] ^ (Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned int*)RunPacket) ^= Hash;
		else
			(*(unsigned int*)RunPacket) ^= ServerEncryption[0];

		PacketProgress -= sizeof(unsigned int);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned int));
	}

	if (PacketProgress >= sizeof(unsigned short))
	{
		unsigned short Hash;

		Hash = ServerEncryption[1] ^ (Key & ((1 << 16) - 1));
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

		Hash = ServerEncryption[2] ^ (Key & ((1 << 8) - 1));
		if (Hash)
			(*(unsigned char*)RunPacket) ^= Hash;
		else
			(*(unsigned char*)RunPacket) ^= ServerEncryption[2];

		PacketProgress -= sizeof(unsigned char);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned char));
	}
}

void ClientManager::Disconnect()
{
	UnregisterHandler();
	Terminate();
}

void ClientManager::Terminate()
{
	shutdown(ClientSocket, SHUT_RDWR);
	close(ClientSocket);
}

bool ClientManager::ReceiveData(void* Buffer, unsigned int Size, unsigned long long Timeout, bool Progress)
{
	unsigned int BufferProgress;

	timeval TimeoutTime;

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
			if (RESULT_INVALID(ReceiveSize))
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

				break;
			}
		}
	}
	else
	{
		ReceiveSize = recv(ClientSocket, (char*)ReceiveBuffer + BufferProgress, sizeof(ReceiveBuffer) - BufferProgress, 0) + BufferProgress;
		if (RESULT_INVALID(ReceiveSize))
			return false;

		memcpy(ReceiveBuffer, Buffer, BufferProgress);
		memcpy(((char*)Buffer) + BufferProgress, ReceiveBuffer + BufferProgress, ReceiveSize < Size ? (ReceiveSize - BufferProgress) : (Size - BufferProgress));
	}

	/*if (Timeout)
	{
		ReceiveSize = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, &TimeoutTime, sizeof(TimeoutTime));
		if (RESULT_INVALID(ReceiveSize))
			return false;
	}*/

	return true;
}

bool ClientManager::GetNearestDescriptor(ArcherPacket* Descriptor, unsigned long long Timeout, bool Progress)
{
	if (!ReceiveData(Descriptor, DESCRIPTOR_SIZE, Timeout, Progress))
		return false;

	DecryptDescriptor(Descriptor);
	return true;
}

ArcherType ClientManager::ReadIntoBuffer(void* Buffer, unsigned int Size, unsigned long long Timeout)
{
	ArcherPacket Descriptor;
	void* DataBuffer;

	if (!GetNearestDescriptor(&Descriptor, Timeout, true))
		return ArcherType_Disconnect;

	if (InfractSizeDifference && (Descriptor.Size > Size))
		return ArcherType_Disconnect;

	if (Size < Descriptor.Size)
		DataBuffer = malloc(Descriptor.Size);
	else
	{
		DataBuffer = Buffer;
		Size = Descriptor.Size;
	}

	if (!ReceiveData(DataBuffer, Descriptor.Size, Timeout, true))
		return ArcherType_Disconnect;

	DecryptPacket(Descriptor.Key, DataBuffer, Descriptor.Size);
	if (!VerifyHash(DataBuffer, Descriptor.Size, Descriptor.HashOffset))
	{
		/*if (Banner.Infract(50, InfractType_HashMissmatch))
		{
			Disconnect();

			Descriptor.Type = ArcherType_Disconnect;
		}
		else*/
			Descriptor.Type = ArcherType_None;
	}

	if (DataBuffer != Buffer)
	{
		memcpy(Buffer, DataBuffer, Size);
		free(DataBuffer);
	}

	return Descriptor.Type;
}

bool ClientManager::Send(ArcherType Type, void* Buffer, unsigned int Size)
{
	unsigned int Result;
	ArcherPacket* Allocated;

	Allocated = (ArcherPacket*)malloc(Size + DESCRIPTOR_SIZE);

	Allocated->Type = Type;
	Allocated->Size = Size;
	Allocated->HashOffset = GetDataHashOffset(Buffer, Size);

	memcpy(Allocated->Buffer, Buffer, Size);

	EncryptPacket(Allocated);

	Result = send(ClientSocket, (const char*)Allocated, Size + DESCRIPTOR_SIZE, 0);

	free(Allocated);

	return RESULT_VALID(Result);
}

ArcherType ClientManager::Receive(void* Buffer, unsigned int Size, unsigned int* ReceiveSize, unsigned long long Timeout)
{
	ArcherPacket PacketDescriptor;

	if (ReceiveSize)
	{
		if (!GetNearestDescriptor(&PacketDescriptor, Timeout))
			return ArcherType_Disconnect;

		*ReceiveSize = PacketDescriptor.Size;
	}

	return ReadIntoBuffer(Buffer, Size, Timeout);
}

void ClientManager::StartListening()
{
	addrinfo* ResultInfo;
	addrinfo AddressInfo;

	unsigned long long ReservedIndex;
	unsigned int ListenSocket;
	unsigned int ClientSocket;
	unsigned int IPAddress;

	unsigned long Result;

	bool Reserved;

	memset(&AddressInfo, 0, sizeof(AddressInfo));

	AddressInfo.ai_family = AF_INET;
	AddressInfo.ai_socktype = SOCK_STREAM;
	AddressInfo.ai_protocol = IPPROTO_TCP;
	AddressInfo.ai_flags = AI_PASSIVE;

	Result = getaddrinfo(0, DEFAULT_PORT, &AddressInfo, &ResultInfo);
	if (Result)
		return;

	ListenSocket = socket(ResultInfo->ai_family, ResultInfo->ai_socktype, ResultInfo->ai_protocol);
	if (ListenSocket == ~0)
	{
		freeaddrinfo(ResultInfo);
		return;
	}

	Result = bind(ListenSocket, ResultInfo->ai_addr, ResultInfo->ai_addrlen);
	if (Result)
	{
		freeaddrinfo(ResultInfo);
		close(ListenSocket);
		return;
	}

	freeaddrinfo(ResultInfo);

	while (true)
	{
		Result = listen(ListenSocket, SOMAXCONN);
		if (Result)
		{
			close(ListenSocket);
			return;
		}

		ClientSocket = accept(ListenSocket, 0, 0);
		if (ClientSocket == ~0)
		{
			close(ListenSocket);
			return;
		}

		WaitingConnection = ClientSocket;

		TerminateHandler(GetSocketIPAddress(ClientSocket));
		pthread_create(&Result, 0, (void*(*)(void*))ServerHandler::CreateHandler, 0);

		while (WaitingConnection != ~0);
	}

	close(ListenSocket);
}

unsigned int ClientManager::GetSocketIPAddress(unsigned int ClientSocket)
{
	sockaddr_in AddressInfo;
	unsigned int InfoLength;
	char* IPAddressStuff;

	InfoLength = sizeof(AddressInfo);
	getpeername(ClientSocket, (sockaddr*)&AddressInfo, &InfoLength);

	return AddressInfo.sin_addr.s_addr;
}

void ClientManager::GetIPAddress(char* Buffer)
{
	char* Address;

	Address = inet_ntoa({ IPAddress });
	if (!Address)
		return;

	strcpy(Buffer, Address);
}