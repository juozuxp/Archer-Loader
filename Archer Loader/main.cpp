#include "ClientManager.h"
#include "ProcessManager.h"
#include "PasswordManager.h"
#include "ConfigManager.h"
#include "GUIManager.h"
#include "DynamicImage.h"
#include "Pair.h"

#include <Imports.h>

#include "CryptString.h"

#ifndef _DEBUG
#include <ThemidaSDK.h>
#endif

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{


	Pair<ClientManager*, GUIManager*> Instances;
	DynamicImage ImageBuilder;

	ClientManager::Initialize(0);

	GUIManager::Initialize(500, 500);
	if (!PasswordManager::ProcessLogin())
		return 0;

	ImageBuilder = DynamicImage(true);
	if (!ImageBuilder.GetImageSize())
		return 0;

	Instances.First = ClientManager::GetInterface();
	Instances.Second = GUIManager::GetInterface();
	ProcessManager::ExecuteDll(ImageBuilder.GetImage(), ImageBuilder.GetImageSize(), &Instances);



	return 0;
}