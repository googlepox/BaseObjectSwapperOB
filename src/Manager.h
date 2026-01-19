#pragma once

#include "SwapData.h"

namespace BaseObjectSwapper
{
	inline Script* HasKeywordScript;
	[[nodiscard]] bool HasKeyword(TESObjectCELL* a_cell, const std::string& a_keyword);

	struct ConditionalInput
	{
		ConditionalInput(TESObjectREFR* a_ref, TESForm* a_form) :
			ref(a_ref),
			base(a_form),
			currentCell(a_ref->parentCell)
		{
		}

		[[nodiscard]] bool IsValid(const FormIDStr& a_data, TESObjectREFR* refToCheck) const;
		[[nodiscard]] bool IsValidAll(const FormIDStr& a_data, TESObjectREFR* refToCheck) const;

		// members
		TESObjectREFR* ref;
		TESForm* base;
		TESObjectCELL* currentCell;
	};

	class Manager
	{
	public:
		static Manager* GetSingleton()
		{
			static Manager singleton;
			return std::addressof(singleton);
		}

		void LoadFormsOnce();

		void            PrintConflicts() const;
		SwapResult      GetSwapData(TESObjectREFR* a_ref, TESForm* a_base);
		SwapResult      GetSwapConditionalBase(TESObjectREFR* a_ref, TESForm* a_base);
		TransformResult GetTransformConditional(TESObjectREFR* a_ref, TESForm* a_base);

		void InsertLeveledItemRef(const TESObjectREFR* a_refr);
		bool IsLeveledItemRefSwapped(const TESObjectREFR* a_refr) const;

	private:
		Manager() = default;
		~Manager() = default;

		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;

		void LoadForms();

		SwapMap<SwapDataVec>& get_form_map(const std::string& a_str);
		static void           get_forms(const std::string& a_path, const std::string& a_str, SwapMap<SwapDataVec>& a_map);
		void                  get_forms(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs);
		void                  get_transforms(const std::string& a_path, const std::string& a_str);
		void                  get_transforms(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs);

		// members
		SwapMap<SwapDataVec>         swapForms{};
		SwapMap<SwapDataVec>         swapRefs{};
		SwapMap<SwapDataConditional> swapFormsConditional{};

		SwapMap<TransformDataVec>         transforms{};
		SwapMap<TransformDataConditional> transformsConditional{};

		std::unordered_set<std::uint32_t> swappedLeveledItemRefs{};

		bool           hasConflicts{ false };
		std::once_flag init{};
	};
}