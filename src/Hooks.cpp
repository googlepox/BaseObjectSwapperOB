#include "Hooks.h"
#include "Manager.h"

namespace BaseObjectSwapper
{
	struct InitItemImpl
	{
		static void __fastcall thunk(TESObjectREFR* a_ref, void* esp)
		{
			if (const auto base = a_ref->baseForm) {
				Manager::GetSingleton()->LoadFormsOnce();
				const auto& [swapBase, transformData] = Manager::GetSingleton()->GetSwapData(a_ref, base);
				if (swapBase && swapBase != base) {
					//a_ref->InitBaseForm(swapBase);
					a_ref->baseForm = swapBase;
				}

				if (transformData) {
					transformData->SetTransform(a_ref);
				}
			}
			return ThisStdCallNew<void>(originalAddress, a_ref);
		}
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0xA46C48 + (4 * 0x5B), reinterpret_cast<UInt32>(thunk)); // kVtbl_TESObjectREFR

			_MESSAGE("Installed vtable hook");
		}
	};

	void Install()
	{
		_MESSAGE("-HOOKS-");

		InitItemImpl::Install();
	}
}
