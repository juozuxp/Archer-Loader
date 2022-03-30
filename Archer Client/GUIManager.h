#pragma once
#include <Imports.h>

#define LICENSEKEYSIZE 33

enum GameWindowType
{
	GameWindowType_ARK
};

enum GameLaunchOptions
{
	GameLaunchOptions_None,
	GameLaunchOptions_Steam,
	GameLaunchOptions_Epic,
	GameLaunchOptions_License
};

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

class GUIManager
{
public:
	void Attach();

public:
	static GameWindowReserve* PromptGameWindow(unsigned long long SubTimeStamp);

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

private:
	static bool Stub();

private:
	static bool GameWindow(GameWindowReserve* GameObject);

private:
	void InitResources();
	void SetImGuiStyle();
	void EndFrameImGui();
	void StartFrameImGui();
	void ResizeWindow(unsigned long SizeX, unsigned long SizeY);
};

