#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include "GrowthBuffer.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

enum ArcherType : unsigned char
{
	ArcherType_None,
	ArcherType_Disconnect,
	ArcherType_CrashDump,
	ArcherType_Setup,
	ArcherType_Key,
	ArcherType_Manage,
	ArcherType_Battleye,
	ArcherType_BatlleyeEnd
};

#pragma pack(push, 1)
struct ArcherPacket
{
	unsigned long long Key;
	unsigned long long HashOffset;
	unsigned long Size;
	ArcherType Type;
	char Buffer[1];
};
#pragma pack(pop)

#define RESULT_VALID(SocketResult) ((SocketResult) != ~0)
#define RESULT_INVALID(SocketResult) (!RESULT_VALID(SocketResult))

#define DESCRIPTOR_SIZE (sizeof(ArcherPacket) - sizeof(ArcherPacket::Buffer))
#define GET_PACKET_SIZE(PacketDescriptor) (DESCRIPTOR_SIZE + ((ArcherPacket*)(PacketDescriptor))->Size)

class ClientManager
{
public:
	void Attach();

	static bool Initialize(WSAPROTOCOL_INFOA* ProtocolInfo);

	static bool Send(ArcherType Type, const void* Buffer, unsigned long Size);
	static ArcherType Receive(void* Buffer, unsigned long Size, unsigned long* ReceiveSize);
	
	static void Disconnect();
	static void Duplicate(unsigned long ProcessID, WSAPROTOCOL_INFOA* ProtocolInfo);

private:
	static ClientManager* Instance;

private:
	SOCKET Connect();

	bool GetNearestDescriptor(ArcherPacket* Descriptor, bool Progress = false);
	bool VerifyHash(const void* Data, unsigned long Size, unsigned long long SubHash);

	unsigned long long GetDataHash(const void* Data, unsigned long Size);
	unsigned long long GetDataHashOffset(const void* Data, unsigned long Size);

	void EncryptPacket(ArcherPacket* Packet);
	void EncryptDescriptor(ArcherPacket* Descriptor);

	void DecryptDescriptor(ArcherPacket* Descriptor);
	void DecryptPacket(unsigned long long Key, void* Data, unsigned long Size);

	ArcherType ReadIntoBuffer(void* Buffer, unsigned long Size);
	bool ReceiveData(void* Buffer, unsigned long Size, bool Progress = true);

private:
	SOCKET ClientSocket;
	GrowthBuffer SendBuffer;

	unsigned short ReceiveSize;
	unsigned short ReceiveOffset;
	unsigned char ReceiveBuffer[0x1000];

private:
	constexpr static unsigned long long ServerEncryption[] = { 0x895c799bebe1d1b0, 0x46faea950c486fda, 0x6cd5db8075a47660, 0x63b1f64d6ff71064, 0x8a9df759345c7759, 0x1de9f938cb8f8b7a, 0x77b941595c33bdbd, 0x51ae60df5637cf0d, 0x470a5e7950aa855b, 0x12e274cc287c9a3d, 0xf437183be55e01a0, 0x9f52e797424693eb, 0x5c166a85b7907241, 0x303ec848525c0202, 0x98c6da4cb44b6ae8, 0xc0a8a0195950acaf, 0xde42dbac9d703be1, 0xdb481168476058e3, 0xcbd2830e95021781, 0x1a76224743d16c15, 0x829dddd84c581738, 0x6f596ab8c907b946, 0x2df2ebd42cc89893, 0x6d253fe82318cb22, 0xc38178606b27c3d0, 0x8803ca0e3b0052f1, 0x29b3a655028015db, 0x0193a6c8fce25cfd, 0xa442d71a9de44158, 0xb26bdc3508fb774a };
	constexpr static unsigned long long ClientEncryption[] = { 0x7201ab64a09390e2, 0x3efb68291f6a7469, 0xd15fbf0433234dac, 0xad438f9b22982d5b, 0x6230c909885cd426, 0x3db4c26378fb64fc, 0xeca507843b15ac27, 0x5849316be21c364e, 0x86481d61b3494e98, 0xaea5d8fe67bd65a9, 0xe11855ed5b54416b, 0x2108a093ee4daf5f, 0x43b3330c62c990dd, 0x9ddd432e2c559a3d, 0x7007400edcfff79d, 0x706d1713f1b168dc, 0xfcb631407af2ed6b, 0xceffe188b9b9cee8, 0xb4e9bbf91e1d041f, 0x06758850f45b6b7e, 0x8acda5cb8278baef, 0x0cc9c7dddb89928b, 0x08518d9895138745, 0x0c1495e14424ec5d, 0x62677fc93deee0c6, 0xa1ec4a69fd0bf9d9, 0x0bace4e0ab41f072, 0xe58d9aaed1378df1, 0xb9fce7abd1fa6848, 0xecd86ecf46a5091b };

	constexpr static unsigned long long ServerStaticHash[] = { 0xb1c17279e301cf2a, 0xd93bceb429025657, 0xd003207840ed321d, 0xc1d7755a9a2adaad, 0xdd2fde1d85a424f6, 0x8d62e4ef1be855ee, 0x1c044fa222a9ff02, 0x3dbe1550f13f3aaa, 0x8f7cb3b79d1090bf, 0xbe6a1284a445a67f, 0x972bc38896554e07, 0x22a3f8743e03e47e, 0x91a332bab9cf8c6c };
	constexpr static unsigned long long ClientStaticHash[] = { 0x25016ceef474e186, 0x2f2f27c8f0f39375, 0x1c4e01a35d177429, 0x37b5a21a82de9e97, 0x00ca074ff8ddd80d, 0x153c52eee4a888dc, 0xe22a910004b1c899, 0x2fdad8638fcdc44f, 0xcb4e547619d30f1e, 0x6ca5d2948f91ef29, 0xd4c3a87d46209ff4, 0x29820a65f727a634, 0x24b11ac104b1c899 };
};
