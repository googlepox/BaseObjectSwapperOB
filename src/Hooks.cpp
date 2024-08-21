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

	struct LoadFormLevNPCImpl
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
			_MESSAGE("Installed Leveled Character vtable hook");
		}
	};

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

	struct LoadFormLevCREAImpl
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
			originalAddress = DetourVtable(0xA42BB8, reinterpret_cast<UInt32>(LoadFormHook)); // kVtbl_TESLevCreature_LoadForm
			_MESSAGE("Installed Leveled Creature vtable hook");
		}
	};

	// Credits to lStewieAl
	[[nodiscard]] __declspec(noinline) UInt32 __stdcall DetourVtable(UInt32 addr, UInt32 dst)
	{
		UInt32 originalFunction = *(UInt32*)addr;
		SafeWrite32(addr, dst);
		return originalFunction;
	}

	void Install()
	{
		_MESSAGE("-HOOKS-");
		LoadFormREFRImpl::Install();
		LoadFormNPCImpl::Install();
		//LoadFormLevNPCImpl::Install();
		LoadFormCREAImpl::Install();
		//LoadFormLevCREAImpl::Install();
		_MESSAGE("Installed all vtable hooks");

	}
}
