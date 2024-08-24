#include "Hooks.h"
#include "Manager.h"

namespace BaseObjectSwapper
{
	struct LinkFormREFRImpl
	{
		static void __fastcall LinkFormHook(TESObjectREFR* a_ref, void* edx)
		{
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
			ThisStdCall(originalAddress, a_ref);
		}
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0xA46CB0, reinterpret_cast<UInt32>(LinkFormHook)); // kVtbl_TESObjectREFR_LinkForm
			_MESSAGE("Installed TESObjectREFR vtable hook");
		}
	};

	struct LinkFormNPCImpl
	{
		static void __fastcall LinkFormHook(TESObjectREFR* a_ref, void* edx)
		{
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
			ThisStdCall(originalAddress, a_ref);
		}
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0xA6FD08, reinterpret_cast<UInt32>(LinkFormHook)); // kVtbl_Character_LinkForm
			_MESSAGE("Installed Character vtable hook");
		}
	};

	struct LinkFormCREAImpl
	{
		static void __fastcall LinkFormHook(TESObjectREFR* a_ref, void* edx)
		{
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
			ThisStdCall(originalAddress, a_ref);
		}
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0xA71160, reinterpret_cast<UInt32>(LinkFormHook)); // kVtbl_Creature_LinkForm
			_MESSAGE("Installed Creature vtable hook");
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
		LinkFormREFRImpl::Install();
		LinkFormNPCImpl::Install();
		LinkFormCREAImpl::Install();
		_MESSAGE("Installed all vtable hooks");

	}
}
