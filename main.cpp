#include "src/Hooks.h"
#include "src/Manager.h"

IDebugLog		gLog("BaseObjectSwapper.log");
PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

OBSEMessagingInterface* g_messagingInterface{};
OBSEInterface* g_OBSEInterface{};
OBSECommandTableInterface* g_cmdTableInterface{};

// OBLIVION = Is not being compiled as a CS plugin.
#if OBLIVION
OBSEScriptInterface* g_script{};
OBSEStringVarInterface* g_stringInterface{};
OBSEArrayVarInterface* g_arrayInterface{};
OBSESerializationInterface* g_serializationInterface{};
OBSEConsoleInterface* g_consoleInterface{};
OBSEEventManagerInterface* g_eventInterface{};

#define ENABLE_EXTRACT_ARGS_MACROS 1	// #define this as 0 if you prefer not to use this

#if ENABLE_EXTRACT_ARGS_MACROS

OBSEScriptInterface* g_scriptInterface = NULL;	// make sure you assign to this
#define ExtractArgsEx(...) g_scriptInterface->ExtractArgsEx(__VA_ARGS__)
#define ExtractFormatStringArgs(...) g_scriptInterface->ExtractFormatStringArgs(__VA_ARGS__)

#endif
#endif

// Test command

#if OBLIVION

bool Cmd_SwapBase_Execute(COMMAND_ARGS)
{

	TESObjectREFR* baseObject;
	TESObjectREFR* swapObject;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &baseObject, &swapObject)) {
		
		_MESSAGE("BOS: SwapBase called via script...");
		//Console_Print("BOS: Swapping object...");

		baseObject->baseForm = swapObject->baseForm;
	}

	return true;
}

static ParamInfo kParams_SwapBase[2] =
{
	{ "object reference", kParamType_ObjectRef, 0 },
	{ "base form", kParamType_ObjectRef, 0 },
};

static CommandInfo kSwapBaseCommand =
{
	"SwapBase",
	"",
	0,
	"Swap base objects",
	0,
	2,
	kParams_SwapBase,
	HANDLER(Cmd_SwapBase_Execute)
};

bool Cmd_SwapModel_Execute(COMMAND_ARGS)
{

	TESObjectREFR* baseObject;
	std::string modelString;

	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &baseObject, &modelString)) {

		_MESSAGE("BOS: SwapModel called via script...");
		//Console_Print("BOS: Swapping object...");

		TESModel* model = OBLIVION_CAST(baseObject->baseForm, TESForm, TESModel);
		model->SetModelPath(modelString.c_str());
	}

	return true;
}

static ParamInfo kParams_SwapModel[2] =
{
	{ "object reference", kParamType_ObjectRef, 0 },
	{ "model path", kParamType_String, 0 },
};

static CommandInfo kSwapModelCommand =
{
	"SwapModel",
	"",
	0,
	"Swap base objects",
	0,
	2,
	kParams_SwapModel,
	HANDLER(Cmd_SwapModel_Execute)
};

#endif

// This is a message handler for OBSE events
// With this, plugins can listen to messages such as whenever the game loads
void MessageHandler(OBSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case OBSEMessagingInterface::kMessage_GameInitialized: break;
	default: break;
	}
}

bool OBSEPlugin_Query(const OBSEInterface* OBSE, PluginInfo* info)
{
	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "BaseObjectSwapper";
	info->version = 1;

	// version checks
	if (OBSE->obseVersion < OBSE_VERSION_INTEGER)
	{
		_ERROR("OBSE version too old (got %08X expected at least %08X)", OBSE->obseVersion, OBSE_VERSION_PADDEDSTRING);
		return false;
	}

	if (!OBSE->isEditor)
	{
		if (OBSE->oblivionVersion < OBLIVION_VERSION_1_2_416)
		{
			_ERROR("incorrect runtime version (got %08X need at least %08X)", OBSE->oblivionVersion, OBLIVION_VERSION_1_2_416);
			return false;
		}
	}
	else
	{
		if (OBSE->editorVersion < CS_VERSION_1_2)
		{
			_ERROR("incorrect editor version (got %08X need at least %08X)", OBSE->editorVersion, CS_VERSION_1_2);
			return false;
		}
	}

	// version checks pass
	// any version compatibility checks should be done here
	return true;
}

bool OBSEPlugin_Load(OBSEInterface* OBSE)
{
	g_pluginHandle = OBSE->GetPluginHandle();

	// save the OBSE interface in case we need it later
	g_OBSEInterface = OBSE;

	// register to receive messages from OBSE
	g_messagingInterface = static_cast<OBSEMessagingInterface*>(OBSE->QueryInterface(kInterface_Messaging));
	g_messagingInterface->RegisterListener(g_pluginHandle, "OBSE", MessageHandler);

	if (!OBSE->isEditor)
	{
#if OBLIVION
		// script and function-related interfaces
		g_script = static_cast<OBSEScriptInterface*>(OBSE->QueryInterface(kInterface_Script));
		g_stringInterface = static_cast<OBSEStringVarInterface*>(OBSE->QueryInterface(kInterface_StringVar));
		g_arrayInterface = static_cast<OBSEArrayVarInterface*>(OBSE->QueryInterface(kInterface_ArrayVar));
		g_eventInterface = static_cast<OBSEEventManagerInterface*>(OBSE->QueryInterface(kInterface_EventManager));
		g_serializationInterface = static_cast<OBSESerializationInterface*>(OBSE->QueryInterface(kInterface_Serialization));
		g_consoleInterface = static_cast<OBSEConsoleInterface*>(OBSE->QueryInterface(kInterface_Console));
#endif
	}

	//OBSE->SetOpcodeBase(0x2B00);
	//->RegisterCommand(&kSwapBaseCommand);
	//OBSE->RegisterCommand(&kSwapModelCommand);

	BaseObjectSwapper::Install();

	return true;
}
