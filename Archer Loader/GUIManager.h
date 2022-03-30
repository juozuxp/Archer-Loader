#pragma once
#include <Imports.h>
#include "Pair.h"

enum LoginType
{
	LoginType_None,
	LoginType_Login,
	LoginType_Register,
	LoginType_ForgotPassword
};

enum ResetType
{
	ResetType_None,
	ResetType_ResetPassword,
	ResetType_Login
};

struct LoginReserve
{
	char* RegistrationKey;
	char* Username;
	char* Password;

	LoginType Login;
};

struct ResetReserve
{
	char* RegistrationKey;
	char* Password;

	ResetType Type;
};

class GUIManager
{
public:
	static GUIManager* GetInterface();
	static void Initialize(unsigned long SizeX, unsigned long SizeY);
	static ResetType PromptReset(char* Password, char* RegistrationKey);
	static LoginType PromptLogin(char* Username, char* Password, char* RegistrationKey);

private:
	static GUIManager Instance;

private:
	HWND Window;
	class ImFont* Font;
	class ID3D11Device* Device;
	class IDXGISwapChain* Swapchain;
	class ID3D11DeviceContext* Context;
	class ID3D11RenderTargetView* RenderView;

private:
	void* CurrentReserve = 0;
	bool(*RenderFunction)(void* Reserved) = (bool(*)(void*))Stub;
	LRESULT(*WndProcFunction)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	static constexpr float LoginWindowSize[] = { 600, 125 };
	static constexpr float ResetWindowSize[] = { 300, 125 };

private:
	static bool Stub();
	static bool LoginWindow(LoginReserve* LoginObject);
	static bool ResetWindow(ResetReserve* ResetObject);
	static void RenderThread(unsigned long long WindowSize);

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HWND InitializeWindow(unsigned long SizeX, unsigned long SizeY);
	LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void SetImGuiStyle();
	void EndFrameImGui();
	void StartFrameImGui();
	void Render(unsigned long SizeX, unsigned long SizeY);
	void RandomString(char* String, unsigned long Length);
	void ResizeWindow(unsigned long SizeX, unsigned long SizeY);
	void InitializeD3D11(HWND Window, class ID3D11RenderTargetView** RenderView);
};

