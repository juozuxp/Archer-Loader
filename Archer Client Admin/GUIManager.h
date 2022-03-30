#pragma once
#include <Imports.h>
#include "AdminManager.h"

enum GameWindowType
{
	GameWindowType_ARK
};

enum GameLaunchOptions
{
	GameLaunchOptions_None,
	GameLaunchOptions_Steam,
	GameLaunchOptions_Epic,
	GameLaunchOptions_License,
	GameLaunchOptions_Admin
};

#define LICENSEKEYSIZE 33

struct GameWindowReserve
{
	unsigned long long UnixTimestamp;
	GameLaunchOptions LaunchOption;
	union
	{
		float Progress;
		char LicenseKey[LICENSEKEYSIZE + 1];
	};
};

struct AdminPanelReserve
{
	const UserInfo* Users;
	unsigned long long UserCount;
	AdminAction Action;
	char RegistrationKey[REGISRATIONKEYSIZE];
	union
	{
		char NewPassword[PASSWORDSIZE];
		struct
		{
			char LicenseKey[LICENSEKEYSIZE];
			unsigned long LicenseLength;
		};
	};
};

class GUIManager
{
public:
	void Attach();

public:
	static GameWindowReserve* PromptGameWindow(unsigned long long SubTimeStamp);
	static AdminPanelReserve* PromptAdminPanel(const UserInfo* Users, unsigned long long UserCount);

private:
	static GUIManager* Instance;

private:
	static class ID3D11ShaderResourceView* ARKLogoTexture;

private:
	HWND Window;
	class ImFont* Font;
	class ID3D11Device* Device;
	class IDXGISwapChain* Swapchain;
	class ID3D11DeviceContext* Context;
	class ID3D11RenderTargetView* RenderView;

private:
	void* CurrentReserve;
	bool(*RenderFunction)(void* Reserved);
	LRESULT(*WndProcFunction)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	static constexpr unsigned long GameWindowSize[] = { 500, 300 };
	static constexpr unsigned long AdminWindowSize[] = { 800, 300 };

private:
	static bool Stub();

private:
	static bool GameWindow(GameWindowReserve* GameObject);
	static bool AdminWindow(AdminPanelReserve* AdminObject);

private:
	void InitResources();
	void SetImGuiStyle();
	void EndFrameImGui();
	void StartFrameImGui();
	void ResizeWindow(unsigned long SizeX, unsigned long SizeY);
};

