#include "Hooks.h"
#include "Manager.h"

namespace BaseObjectSwapper
{
	struct LoadFormREFRImpl
	{
		static void __fastcall LoadFormHook(TESObjectREFR* a_ref, void* edx, ModEntry::Data* tesFile)
		{
			ThisStdCall(originalAddress, a_ref, tesFile);
			if (const auto base = a_ref->baseForm) {
				Manager::GetSingleton()->LoadFormsOnce();
				const auto& [swapBase, transformData] = Manager::GetSingleton()->GetSwapData(a_ref, base);
				if (swapBase && swapBase != base) {
					a_ref->baseForm = swapBase;
				}
				if (transformData != std::nullopt) {
					transformData->SetTransform(a_ref);
				}
			}
		}
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0xA46C60, reinterpret_cast<UInt32>(LoadFormHook)); // kVtbl_TESObjectREFR_LoadForm
			_MESSAGE("Installed TESObjectREFR vtable hook");
		}
	};

	//not implemented
	struct LoadFormNPCImpl
	{
		static void __fastcall LoadFormHook(TESObjectREFR* a_ref, void* edx, ModEntry::Data* tesFile)
		{
			ThisStdCall(originalAddress, a_ref, tesFile);
			if (const auto base = a_ref->baseForm) {
				Manager::GetSingleton()->LoadFormsOnce();
				const auto& [swapBase, transformData] = Manager::GetSingleton()->GetSwapData(a_ref, base);
				if (swapBase && swapBase != base) {
					a_ref->baseForm = swapBase;
				}
				if (transformData != std::nullopt) {
					transformData->SetTransform(a_ref);
				}
			}
		}
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0xA6FCB8, reinterpret_cast<UInt32>(LoadFormHook)); // kVtbl_Character_LoadForm
			_MESSAGE("Installed Character vtable hook");
		}
	};

	//not implemented
	struct LoadFormCREAImpl
	{
		static void __fastcall LoadFormHook(TESObjectREFR* a_ref, void* edx, ModEntry::Data* tesFile)
		{
			ThisStdCall(originalAddress, a_ref, tesFile);
			if (const auto base = a_ref->baseForm) {
				Manager::GetSingleton()->LoadFormsOnce();
				const auto& [swapBase, transformData] = Manager::GetSingleton()->GetSwapData(a_ref, base);
				if (swapBase && swapBase != base) {
					a_ref->baseForm = swapBase;
				}
				if (transformData != std::nullopt) {
					transformData->SetTransform(a_ref);
				}
			}
		}
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0xA71110, reinterpret_cast<UInt32>(LoadFormHook)); // kVtbl_Creature_LoadForm
			_MESSAGE("Installed Creature vtable hook");
		}
	};

	void Install()
	{
		_MESSAGE("-HOOKS-");
		LoadFormREFRImpl::Install();
		_MESSAGE("Installed all vtable hooks");

	}
}
