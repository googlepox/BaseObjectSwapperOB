#include "Hooks.h"
#include "Manager.h"

namespace BaseObjectSwapper
{
	struct LinkFormREFRImpl
	{
		static void __fastcall LinkFormHookREFR(TESObjectREFR* a_ref, void* edx)
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
			ThisStdCall(originalAddressREFR, a_ref);
		}
		static inline std::uint32_t originalAddressREFR;

		static void Install()
		{
			originalAddressREFR = DetourVtable(0xA46C44 + (4 * 0x22), reinterpret_cast<UInt32>(LinkFormHookREFR)); // kVtbl_TESObjectREFR_LinkForm
			//originalAddressREFR = DetourVtable(0xA46CB0, reinterpret_cast<UInt32>(LinkFormHookREFR)); // kVtbl_TESObjectREFR_LinkForm
			_MESSAGE("Installed TESObjectREFR vtable hook");
		}
	};

	struct LinkFormNPCImpl
	{
		static void __fastcall LinkFormHookNPC(TESObjectREFR* a_ref, void* edx)
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
			ThisStdCall(originalAddressNPC, a_ref);
		}
		static inline std::uint32_t originalAddressNPC;

		static void Install()
		{
			originalAddressNPC = DetourVtable(0xA6FD08, reinterpret_cast<UInt32>(LinkFormHookNPC)); // kVtbl_Character_LinkForm
			_MESSAGE("Installed Character vtable hook");
		}
	};

	struct LinkFormCREAImpl
	{
		static void __fastcall LinkFormHookCREA(TESObjectREFR* a_ref, void* edx)
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
			ThisStdCall(originalAddressCREA, a_ref);
		}
		static inline std::uint32_t originalAddressCREA;

		static void Install()
		{
			originalAddressCREA = DetourVtable(0xA71160, reinterpret_cast<UInt32>(LinkFormHookCREA)); // kVtbl_Creature_LinkForm
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
		//LinkFormNPCImpl::Install();
		//LinkFormCREAImpl::Install();
		_MESSAGE("Installed all vtable hooks");

	}
}
