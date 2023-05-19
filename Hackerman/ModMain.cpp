#include <Prey/CryScriptSystem/IScriptSystem.h>
#include <Prey/GameDll/ark/player/ArkPlayerInteraction.h>
#include <Prey/GameDll/ark/player/ArkPlayer.h>
#include <Prey/GameDll/ark/npc/ArkNpc.h>
#include <Prey/GameDll/ark/ui/ArkHackingUI.h>
#include <Prey/GameDll/ark/ArkGameMetricsComponent.h>
#include "ModMain.h"
#include <Chairloader/ModSDK/ChairGlobalModName.h>

ModMain* gMod = nullptr;

static auto g_ArkPlayerInteraction_PerformInteraction_Hook = ArkPlayerInteraction::FPerformInteraction.MakeHook();
static auto g_ArkHackingUI_AttemptToHack_Hook = ArkHackingUI::FAttemptToHack.MakeHook();

bool CallScriptFunction(IEntity* pEntity, IScriptTable* pScriptTable, const char* functionName)
{
	bool result = false;

	if ((pEntity != NULL) && (pScriptTable != NULL))
	{
		IScriptSystem* pScriptSystem = pScriptTable->GetScriptSystem();
		if (pScriptTable->GetValueType(functionName) == svtFunction)
		{
			pScriptSystem->BeginCall(pScriptTable, functionName);
			pScriptSystem->PushFuncParam(pEntity->GetScriptTable());
			pScriptSystem->EndCall(result);
		}
	}

	return result;
}

static void HackingSuccess(unsigned _entityToHack, ArkHackingUI::HackingCallback _callback)
{
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(_entityToHack);
	CallScriptFunction(pEntity, pEntity->GetScriptTable(), "HackSuccess");

	// Update metrics: ObjectsHacked
	ArkPlayer* pPlayer = ArkPlayer::GetInstancePtr();
	if (pPlayer)
		pPlayer->m_playerComponent.GetGameMetricComponent().IncrementValue(10739735956152977655, 1);

	// Call back
	if (_callback)
		_callback(true);
}

static void ArkPlayerInteraction_PerformInteraction_Hook(ArkPlayerInteraction* const _this, EArkInteractionType _interaction, EArkInteractionMode _mode, IEntity* const _pEntity, float _delay)
{
	if (_interaction == EArkInteractionType::hack)
	{
		ArkNpc* pNpcEntity = EntityUtils::GetArkNpc(_pEntity);
		if (!pNpcEntity || !pNpcEntity->IsMimicking())
		{
			HackingSuccess(_pEntity->GetId(), ArkHackingUI::HackingCallback());
		}
	}
	else
	{
		g_ArkPlayerInteraction_PerformInteraction_Hook.InvokeOrig(_this, _interaction, _mode, _pEntity, _delay);
	}
}

static bool ArkHackingUI_AttemptToHack_Hook(ArkHackingUI* const _this, unsigned _entityToHack, Functor1<bool> _callback, uint64_t _hackingRequirement)
{
	uint64_t req = _hackingRequirement;
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(_entityToHack);

	if (_hackingRequirement == 0)
	{
		req = ArkPlayerInteraction::GetEntityHackingRequirement(pEntity);
	}

	if (!ArkPlayer::GetInstance().HasAbility(req))
	{
		// Let the original code handle that
		return g_ArkHackingUI_AttemptToHack_Hook.InvokeOrig(_this, _entityToHack, _callback, _hackingRequirement);
	}

	HackingSuccess(_entityToHack, _callback);
}

void ModMain::FillModInfo(ModDllInfoEx& info)
{
	info.modName = "tmp64.Hackerman"; // CHANGE ME
	info.logTag = "Hackerman"; // CHANGE ME
	info.supportsHotReload = true; // TODO: Add comment/wiki link
}

void ModMain::InitHooks()
{
	g_ArkPlayerInteraction_PerformInteraction_Hook.SetHookFunc(&ArkPlayerInteraction_PerformInteraction_Hook);
	g_ArkHackingUI_AttemptToHack_Hook.SetHookFunc(&ArkHackingUI_AttemptToHack_Hook);
}

void ModMain::InitSystem(const ModInitInfo& initInfo, ModDllInfo& dllInfo)
{
	BaseClass::InitSystem(initInfo, dllInfo);
    ChairSetGlobalModName("tmp64.Hackerman"); // CHANGE ME
}

void ModMain::InitGame(bool isHotReloading)
{
	BaseClass::InitGame(isHotReloading);
	// Your code goes here
}

void ModMain::ShutdownGame(bool isHotUnloading)
{
	// Your code goes here
	BaseClass::ShutdownGame(isHotUnloading);
}

void ModMain::ShutdownSystem(bool isHotUnloading)
{
	// Your code goes here
	BaseClass::ShutdownSystem(isHotUnloading);
}

void ModMain::Draw()
{
}

void ModMain::MainUpdate(unsigned updateFlags)
{
	// Your code goes here
}



void *ModMain::QueryInterface(const char *ifaceName) {
    return nullptr;
}

void ModMain::Connect(const std::vector<IChairloaderMod *> &mods) {
}

extern "C" DLL_EXPORT IChairloaderMod* ClMod_Initialize()
{
	CRY_ASSERT(!gMod);
	gMod = new ModMain();
	return gMod;
}

extern "C" DLL_EXPORT void ClMod_Shutdown()
{
	CRY_ASSERT(gMod);
	delete gMod;
	gMod = nullptr;
}

// Validate that declarations haven't changed
static_assert(std::is_same_v<decltype(ClMod_Initialize), IChairloaderMod::ProcInitialize>);
static_assert(std::is_same_v<decltype(ClMod_Shutdown), IChairloaderMod::ProcShutdown>);
