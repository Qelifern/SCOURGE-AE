#include "LoadEditorIDs.h"
#include "PapyrusManager.h"
#include "RNJesus.h"
#include "SCOURGE.h"
#include "YamlParser.h"
#include "IPluginCommon.h"
#include "IVersionCheck.h"
#include <pch.h>

#include <config.h>


// Windows
#include <shlobj.h>	// CSIDL_MYCODUMENTS





CSimpleIniA simpleIni(true, false, false);
ModConfigs configData;
PluginData pluginData;
RNJesus mathUtils;



extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* f4se, F4SE::PluginInfo* info)
{
	
	REX::INFO(PLUGIN_VERSION_INFO); // write the plugin version to the log
	REX::INFO("Plugin_Query: Querying");

	// populate info structure
	info->infoVersion = F4SE::PluginInfo::kVersion;
	info->name = PLUGIN_NAME_LONG;		// set in config.h
	info->version = PLUGIN_VERSION_DLL;		// set in config.h


	const auto ver = f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_11_221) {
		REX::INFO("unsupported runtime v{}", ver.string());
		return false;
	}


	REX::INFO("Plugin_Query: Queried Successfully"); // log the successful query

	// supported runtime version
	return true;
}


extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface * a_f4se)
{


	REX::INFO(PLUGIN_VERSION_INFO);
	REX::INFO("Plugin_Load: Loading");


	F4SE::Init(a_f4se);


	Patches::LoadEditorIDs::Install();
	

	const F4SE::PapyrusInterface* papyrus = F4SE::GetPapyrusInterface();
	papyrus->Register(Papyrus::BindFunctions);

	const F4SE::MessagingInterface* messageInterface = F4SE::GetMessagingInterface();
	messageInterface->RegisterListener([](F4SE::MessagingInterface::Message* msg) -> void {
		if (msg->type == F4SE::MessagingInterface::kGameDataReady) {
			// this setting resets values, so if it's enabled we bother the user and close their game
			if (pluginData.RobCoPatcherCheck(simpleIni) != 0.0f) {
				std::string message = "WARNING!\n\nYou must disable iEnableReCalculateStatsWithSaveLoad inside Data\\F4SE\\Plugins\\RobCo_Patcher.ini before using SCOURGE to prevent complications.\n\nThe game will terminate once this message box is closed.";
				MessageBoxA(nullptr, message.c_str(), "SCOURGE", MB_OK | MB_ICONERROR);
				std::abort();
			}

			// make my own mod a requirement for double the DP and CTD the game if they don't have it :V
			if (!F4SE::GetPluginInfo("scaling_flag_remover_ae")) {
				std::string message = "WARNING!\n\nScaling Flag Remover is not installed!\nPlease download it before using SCOURGE: https://www.nexusmods.com/fallout4/mods/105868\n\nThe game will terminate once this message box is closed.";
				MessageBoxA(nullptr, message.c_str(), "SCOURGE", MB_OK | MB_ICONERROR);
				std::abort();
			}

			pluginData.LoadPluginData();

			REX::INFO("---------------------------------------------------------------------");

			if (pluginData.loadedCorrectly == false) {
				REX::INFO("Plugin has not been able to load correctly. Aborting further operations...");
			}
			else {
				configData.LoadConfigs();

				if (auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
					std::uint32_t DLC01CompWorkbenchBot{ 0 };
					std::uint32_t DLC01LvlCompWorkbenchBot{ 0 };
					// exclude automatron robot companion
					if (dataHandler->LookupModByName("DLCRobot.esm")) {
						DLC01CompWorkbenchBot = dataHandler->LookupFormID(69753, "DLCRobot.esm");
						DLC01LvlCompWorkbenchBot = dataHandler->LookupFormID(7917, "DLCRobot.esm");
					}

					for (auto currentNPC : dataHandler->GetFormArray<RE::TESNPC>()) {
						// ignore the player
						// MQ101 Nate and Nora so that they don't become midgets
						if (currentNPC->IsPlayer() ||                                                                       // exclude player
							currentNPC->formID == 687412 || currentNPC->formID == 687413 ||                                 // MQ101 Nate and Nora so that they don't become midgets
							currentNPC->formID == DLC01LvlCompWorkbenchBot || currentNPC->formID == DLC01CompWorkbenchBot)  // robot workbench bots
						{
							continue;
						}

						currentNPC->AddSpell(pluginData.spellForm);
					}
				}
				else {
					REX::INFO("Unable to distribute SCOURGE_SPEL! The plugin will not work...");
				}
			}
		}
		});

	return true;
}