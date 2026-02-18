#include "Manager.h"
#include "EditorIDMapper/EditorIDMapperAPI.h"
#include "lib/boost/trim.hpp"

extern OBSEScriptInterface* g_script;

namespace BaseObjectSwapper
{
	SwapMap<SwapDataVec>& Manager::get_form_map(const std::string& a_str)
	{
		return a_str == "Forms" ? swapForms : swapRefs;
	}

	void Manager::get_transforms(const std::string& a_path, const std::string& a_str)
	{
		return TransformData::GetTransforms(a_path, a_str, [&](RE::FormID a_baseID, const TransformData& a_swapData) {
			transforms[a_baseID].push_back(a_swapData);
			});
	}

	void Manager::get_transforms(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs)
	{
		return TransformData::GetTransforms(a_path, a_str, [&](const RE::FormID a_baseID, const TransformData& a_swapData) {
			for (auto& id : a_conditionalIDs) {
				transformsConditional[a_baseID][id].push_back(a_swapData);
			}
			});
	}

	void Manager::get_forms(const std::string& a_path, const std::string& a_str, SwapMap<SwapDataVec>& a_map)
	{
		return SwapData::GetForms(a_path, a_str, [&](RE::FormID a_baseID, const SwapData& a_swapData) {
			a_map[a_baseID].push_back(a_swapData);
			});
	}

	void Manager::get_forms(const std::string& a_path, const std::string& a_str, const std::vector<FormIDStr>& a_conditionalIDs)
	{
		return SwapData::GetForms(a_path, a_str, [&](const RE::FormID a_baseID, const SwapData& a_swapData) {
			for (auto& id : a_conditionalIDs) {
				swapFormsConditional[a_baseID][id].push_back(a_swapData);
			}
			});
	}

	static bool HasKeywordCell(TESObjectCELL* a_cell, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (a_cell)
		{
			std::string newKey = std::get<std::string>(a_keyword);
			UInt32 cellID = a_cell->refID;
			UInt32 newFormID = SwapData::GetFormID(newKey.c_str());
			if (newFormID)
			{
				if (newFormID && ((newFormID == cellID) || (std::to_string(cellID).contains(std::to_string(newFormID)))))
				{
					return !isExclusion;
				}
				return isExclusion;
			}
			else if (newKey == "Dungeon")
			{
				if (a_cell->extraData.HasType(ExtraDataType::kExtraData_CellMusicType))
				{
					if (a_cell->extraData.GetByType(ExtraDataType::kExtraData_CellMusicType))
					{
						ExtraCellMusicType* extraData = OBLIVION_CAST(a_cell->extraData.GetByType(ExtraDataType::kExtraData_CellMusicType), BSExtraData, ExtraCellMusicType);
						if (extraData && extraData->musicType == ExtraCellMusicType::kMusicType_Dungeon)
						{
							return !isExclusion;
						}
					}
					
				}
				return isExclusion;
			}
			else
			{
				const char* id = EditorIDMapper::ReverseLookup(a_cell->refID);
				if (!id) return false;
				std::string editorID = id;
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos)
				{
					return !isExclusion;
				}
				return isExclusion;
			}
		}
		else
		{
			return false;
		}
	}

	static bool HasKeywordWorldspace(TESObjectCELL* a_cell, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (a_cell)
		{
			std::string newKey = std::get<std::string>(a_keyword);
			if (!a_cell || !a_cell->worldSpace)
				return false;
			UInt32 cellID = a_cell->worldSpace->refID;
			UInt32 newFormID = SwapData::GetFormID(newKey.c_str());
			if (newFormID)
			{
				if (newFormID && ((newFormID == cellID) || (std::to_string(cellID).contains(std::to_string(newFormID)))))
				{
					return !isExclusion;
				}
				return isExclusion;
			}
			else
			{
				const char* worldspaceEditorID = EditorIDMapper::ReverseLookup(a_cell->worldSpace->refID);
				if (!worldspaceEditorID) return false;
				std::string editorID = worldspaceEditorID;
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos)
				{
					return !isExclusion;
				}
				return isExclusion;
			}
		}
		else
		{
			return false;
		}
	}

	static bool HasKeywordRegion(TESObjectCELL* a_cell, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (a_cell)
		{
			ExtraRegionList* regionList = dynamic_cast<ExtraRegionList*>(a_cell->extraData.GetByType(kExtraData_RegionList));
			TESRegionList* regions = regionList->regionList;
			TESRegionList::Entry* regionPtr = &(regions->regionList);
			bool found = false;
			while (regionPtr != NULL)
			{
				UInt32 regionID = regionPtr->region->refID;
				std::string newKey = std::get<std::string>(a_keyword);
				UInt32 newFormID = SwapData::GetFormID(newKey.c_str());
				if (newFormID)
				{
					if (newFormID && ((newFormID == regionID) || (std::to_string(regionID).contains(std::to_string(newFormID)))))
					{
						found = true;
					}
				}
				else
				{
					const char* id = EditorIDMapper::ReverseLookup(regionPtr->region->refID);
					if (!id) return false;
					std::string editorID = id;
					std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
					std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
					std::string cStrKey = newKey.c_str();
					std::string cStrEditorID = editorID.c_str();
					if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos)
					{
						found = true;
					}
				}
				regionPtr = regionPtr->next;
			}
			if (found)
			{
				return !isExclusion;
			}
			return isExclusion;
		}
		else
		{
			return false;
		}
	}

	static bool HasKeywordEditorID(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref)
		{
			std::string newKey = std::get<std::string>(a_keyword);
			UInt32 refID = ref->baseForm->refID;
			UInt32 newFormID = std::atoi(newKey.c_str());
			if (newFormID)
			{
				if (newFormID && ((newFormID == refID)))
				{
					return !isExclusion;
				}
				return isExclusion;
			}
			else
			{
				std::string editorID;
				const char* id;
				if (ref->baseForm)
				{
					id = EditorIDMapper::ReverseLookup(ref->baseForm->refID);
					if (!id) return false;
					std::string editorID = id;
				}
				else
				{
					id = EditorIDMapper::ReverseLookup(ref->refID);
					if (!id) return false;
					std::string editorID = id;
				}
				std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
				std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
				std::string cStrKey = newKey.c_str();
				std::string cStrEditorID = editorID.c_str();
				if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos)
				{
					return !isExclusion;
				}
				return isExclusion;
			}
		}
		else
		{
			return false;
		}
	}

	static bool HasKeywordName(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref)
		{
			std::string newKey = std::get<std::string>(a_keyword);
			std::string editorID;
			if (ref->baseForm)
			{
				editorID = (ref->baseForm->GetFullName()->name.m_data);
			}
			else
			{
				editorID = (ref->GetFullName()->name.m_data);
			}
			std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
			std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
			std::string cStrKey = newKey.c_str();
			std::string cStrEditorID = editorID.c_str();
			if (cStrEditorID.find(cStrKey.c_str()) != std::string::npos)
			{
				return !isExclusion;
			}
			return isExclusion;
		}
		else
		{
			return false;
		}
	}

	static bool HasKeywordMod(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref)
		{
			std::string newKey = std::get<std::string>(a_keyword);
			UInt8 modIndex = ref->baseForm->GetModIndex();
			std::string modName = (*g_dataHandler)->GetNthModName(modIndex);
			std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
			std::transform(modName.begin(), modName.end(), modName.begin(), tolower);
			std::string cStrKey = newKey.c_str();
			std::string cStrModName = modName.c_str();
			if (cStrModName.find(cStrKey.c_str()) != std::string::npos)
			{
				return !isExclusion;
			}
			return isExclusion;
		}
		else
		{
			return false;
		}
	}

	static bool HasKeywordRotation(TESObjectREFR* ref, const FormIDStr& a_keyword, bool isExclusion)
	{
		if (ref)
		{
			std::string newKey = std::get<std::string>(a_keyword);
			UInt8 modIndex = ref->baseForm->GetModIndex();
			std::string modName = (*g_dataHandler)->GetNthModName(modIndex);
			std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
			std::transform(modName.begin(), modName.end(), modName.begin(), tolower);
			std::string cStrKey = newKey.c_str();
			std::string cStrModName = modName.c_str();
			if (cStrModName.find(cStrKey.c_str()) != std::string::npos)
			{
				return !isExclusion;
			}
			return isExclusion;
		}
		else
		{
			return false;
		}
	}

	bool HasKeyword(TESObjectCELL* a_cell, const std::string& a_keyword)
	{
		if (a_cell) {
			std::string editorID = (a_cell->GetEditorID2());
			std::string newKey = a_keyword;
			std::transform(newKey.begin(), newKey.end(), newKey.begin(), tolower);
			std::transform(editorID.begin(), editorID.end(), editorID.begin(), tolower);
			std::string cStrKey = newKey.c_str();
			std::string cStrEditorID = editorID.c_str();
			if ((cStrEditorID.find(cStrKey.c_str()) != std::string::npos) && (cStrKey.find('-') == std::string::npos)) {
				return true;
			}
			return false;
		}
		else {
			return false;
		}

	}bool ConditionalInput::IsValid(const FormIDStr& a_data, TESObjectREFR* refToCheck) const
	{
		if (refToCheck)
		{
			if (std::holds_alternative<std::string>(a_data))
			{
				FormIDStr newData = a_data;
				UInt32 formID;
				std::string formString = std::get<std::string>(a_data);
				boost::trim(formString);
				bool isExclusion = false;
				if (formString.find('-') != std::string::npos)
				{
					std::string::iterator end_pos = std::remove(formString.begin(), formString.end(), '-');
					formString.erase(end_pos, formString.end());
					isExclusion = true;
				}
				if (!formString.contains(':'))
				{
					return HasKeywordEditorID(refToCheck, formString, isExclusion);;
				}
				auto conditionType = string::split(formString, ":");
				boost::trim(conditionType[0]);
				boost::trim(conditionType[1]);
				formID = SwapData::GetFormID(conditionType[1]);
				if (formID)
				{
					newData = std::to_string(formID);
				}
				else
				{
					newData = conditionType[1];
				}
				if (conditionType[0] == "Cell")
				{
					return HasKeywordCell(refToCheck->parentCell, newData, isExclusion);
				}
				else if (conditionType[0] == "EditorID")
				{
					return HasKeywordEditorID(refToCheck, newData, isExclusion);
				}
				else if (conditionType[0] == "Name")
				{
					return HasKeywordName(refToCheck, newData, isExclusion);
				}
				else if (conditionType[0] == "Mod")
				{
					return HasKeywordMod(refToCheck, newData, isExclusion);
				}
				else if (conditionType[0] == "Worldspace")
				{
					return HasKeywordWorldspace(refToCheck->parentCell, newData, isExclusion);
				}
				else if (conditionType[0] == "Region")
				{
					return HasKeywordRegion(refToCheck->parentCell, newData, isExclusion);
				}
				else
				{
					return HasKeyword(refToCheck->parentCell, formString);
				}
			}
			else
			{
				if (std::holds_alternative<uint32_t>(a_data))
				{
					if (const auto form = LookupFormByID(std::get<uint32_t>(a_data)))
					{
						switch (form->GetFormType())
						{
						case kFormType_Region:
						{
							if (const auto region = (form))
							{
								if (const auto regionList = dynamic_cast<ExtraRegionList*>(currentCell ? currentCell->extraData.GetByType(kExtraData_RegionList) : nullptr))
								{
									if (const auto list = (regionList->regionList))
									{
										TESRegionList::Entry* regionPtr = &(list->regionList);
										while (regionPtr != NULL)
										{
											if (regionPtr->region == region)
											{
												return true;
											}
											regionPtr = regionPtr->next;
										}
									}
								}
							}
							return false;
						}
						case kFormType_Cell:
						{
							return currentCell == form;
						}
						default:
							break;
						}
					}
				}
			}
		}
		return false;
	}

	bool ConditionalInput::IsValidAll(const FormIDStr& a_data, TESObjectREFR* refToCheck) const
	{
		if (refToCheck)
		{
			if (std::holds_alternative<std::string>(a_data))
			{
				std::string conditionStr = std::get<std::string>(a_data);
				std::vector<bool> resultVec;
				if (conditionStr.contains("ALL")) return true;
				if (conditionStr.contains("&"))
				{
					auto conditions = string::split(conditionStr, "&");
					for (const auto& condition : conditions)
					{
						resultVec.push_back(IsValid(condition, refToCheck));
					}
					for (const auto& result : resultVec)
					{
						if (!result)
						{
							return false;
						}
					}
					return true;
				}
				else
				{
					return IsValid(conditionStr, refToCheck);
				}
			}
			else
			{
				return IsValid(a_data, refToCheck);
			}

		}
		return false;
	}

	void Manager::LoadFormsOnce()
	{
		std::call_once(init, [this] {
			LoadForms();
			});
	}

	void Manager::LoadForms()
	{
		_MESSAGE("-INI-");

		std::string bosFolderPath = R"(Data\OBSE\Plugins\BaseObjectSwapper)";

		const std::filesystem::path bosFolder{ bosFolderPath };

		if (!exists(bosFolder)) {
			_WARNING("BOS folder not found...");
			return;
		}

		std::vector<std::string> configs = dist::get_configs(bosFolderPath);

		if (configs.empty()) {
			_WARNING("No .ini files were found in Data\\OBSE\\Plugins\\BaseObjectSwapper folder, falling back to Data\\BaseObjectSwapper...");
			return;
		}

		_MESSAGE("%u matching inis found", configs.size());

		for (auto& path : configs) {
			_MESSAGE("\tINI : %s", path.c_str());

			CSimpleIniA ini;
			ini.SetUnicode();
			ini.SetMultiKey();
			ini.SetAllowKeyOnly();

			if (const auto rc = ini.LoadFile(path.c_str()); rc < 0) {
				_ERROR("\tcouldn't read INI");
				continue;
			}

			CSimpleIniA::TNamesDepend sections;
			ini.GetAllSections(sections);
			sections.sort(CSimpleIniA::Entry::LoadOrder());

			constexpr auto push_filter = [](const std::string& a_condition, std::vector<FormIDStr>& a_processedFilters) {
				if (a_condition.contains('&') || a_condition.contains(':') || a_condition.contains("ALL"))
				{
					a_processedFilters.emplace_back(a_condition);
				}
				else if (const auto processedID = SwapData::GetFormID(a_condition); processedID != 0)
				{
					a_processedFilters.emplace_back(processedID);
				}
				else
				{
					_ERROR("\t\tFilter  [%s] INFO - unable to find form, treating filter as string", a_condition.c_str());
					a_processedFilters.emplace_back(a_condition);
				}
				};

			for (auto& [section, comment, keyOrder] : sections) {
				if (string::icontains(section, "|")) {
					auto splitSection = string::split(section, "|");
					auto conditions = string::split(splitSection[1], ",");  //[Forms|EditorID,EditorID2]

					_MESSAGE("\t\treading [%s] : %u conditions", splitSection[0].c_str(), conditions.size());

					std::vector<FormIDStr> processedConditions;
					processedConditions.reserve(conditions.size());
					for (auto& condition : conditions) {
						push_filter(condition, processedConditions);
					}

					CSimpleIniA::TNamesDepend values;
					ini.GetAllKeys(section, values);
					values.sort(CSimpleIniA::Entry::LoadOrder());

					if (!values.empty()) {
						if (splitSection[0] == "Forms") {
							_MESSAGE("\t\t\t%u form swaps found", values.size());
							for (const auto& key : values) {
								get_forms(path, key.pItem, processedConditions);
							}
						}
						else {
							_MESSAGE("\t\t\t%u transform overrides found", values.size());
							for (const auto& key : values) {
								get_transforms(path, key.pItem, processedConditions);
							}
						}
					}
				}
				else {
					_MESSAGE("\t\treading [%s]", section);

					CSimpleIniA::TNamesDepend values;
					ini.GetAllKeys(section, values);
					values.sort(CSimpleIniA::Entry::LoadOrder());

					if (!values.empty()) {
						if (string::iequals(section, "Transforms")) {
							_MESSAGE("\t\t\t%u transform overrides found", values.size());
							for (const auto& key : values) {
								get_transforms(path, key.pItem);
							}
						}
						else {
							_MESSAGE("\t\t\t%u swaps found", values.size());
							auto& map = get_form_map(section);
							for (const auto& key : values) {
								get_forms(path, key.pItem, map);
							}
						}
					}
				}
			}
		}

		_MESSAGE("-RESULT-");

		_MESSAGE("%u form-form swaps processed", swapForms.size());
		_MESSAGE("%u conditional form swaps processed", swapFormsConditional.size());
		_MESSAGE("%u ref-form swaps processed", swapRefs.size());
		_MESSAGE("%u transform overrides processed", transforms.size());
		_MESSAGE("%u conditional transform overrides processed", transformsConditional.size());

		_MESSAGE("-CONFLICTS-");

		const auto log_conflicts = [&]<typename T>(std::string_view a_type, const SwapMap<T>&a_map) {
			if (a_map.empty()) {
				return;
			}
			_MESSAGE("[%s]", a_type.data());
			bool conflicts = false;
			for (auto& [baseID, swapDataVec] : a_map) {
				if (swapDataVec.size() > 1) {
					const auto& winningRecord = swapDataVec.back();
					if (winningRecord.traits.chance != 100) {  //ignore if winning record is randomized
						continue;
					}
					conflicts = true;
					auto winningForm = string::split(winningRecord.record, "|");
					_WARNING("\t%s", winningForm[0].c_str());
					_WARNING("\t\twinning record : %s (%s)", winningForm[1].c_str(), swapDataVec.back().path.c_str());
					_WARNING("\t\t%u conflicts", swapDataVec.size() - 1);
					for (auto it = swapDataVec.rbegin() + 1; it != swapDataVec.rend(); ++it) {
						auto losingRecord = it->record.substr(it->record.find('|') + 1);
						_WARNING("\t\t\t%s (%s)", losingRecord.c_str(), it->path.c_str());
					}
				}
			}
			if (!conflicts) {
				_MESSAGE("\tNo conflicts found");
			}
			else {
				hasConflicts = true;
			}
		};

		log_conflicts("Forms", swapForms);
		log_conflicts("References", swapRefs);
		log_conflicts("Transforms", transforms);

		_MESSAGE("-END-");
	}

	void Manager::PrintConflicts() const
	{
		if (hasConflicts) {
			Console_Print(std::format("[BOS] Conflicts found, check BaseObjectSwapper.log in {} for more info\n", GetOblivionDirectory()).c_str());
		}
	}

	SwapResult Manager::GetSwapConditionalBase(TESObjectREFR* a_ref, TESForm* a_base)
	{
		if (const auto it = swapFormsConditional.find(static_cast<std::uint32_t>(a_base->refID)); it != swapFormsConditional.end()) {
			const ConditionalInput input(a_ref, a_base);
			const auto             result = std::ranges::find_if(it->second, [&](const auto& a_data) {
				return input.IsValidAll(a_data.first, a_ref);
				});

			if (result != it->second.end()) {
				for (auto& swapData : result->second | std::ranges::views::reverse) {
					if (auto swapObject = swapData.GetSwapBase(a_ref)) {
						return { swapObject, swapData.transform };
					}
				}
			}
		}

		return { nullptr, std::nullopt };
	}

	TransformResult Manager::GetTransformConditional(TESObjectREFR* a_ref, TESForm* a_base)
	{
		if (const auto it = transformsConditional.find(a_base->refID); it != transformsConditional.end()) {
			const ConditionalInput input(a_ref, a_base);
			const auto             result = std::ranges::find_if(it->second, [&](const auto& a_data) {
				return input.IsValidAll(a_data.first, a_ref);
				});

			if (result != it->second.end()) {
				for (auto& transformData : result->second | std::ranges::views::reverse) {
					if (transformData.IsTransformValid(a_ref)) {
						return transformData.transform;
					}
				}
			}
		}

		return std::nullopt;
	}

	void Manager::InsertLeveledItemRef(const TESObjectREFR* a_refr)
	{
		swappedLeveledItemRefs.insert(a_refr->refID);
	}

	bool Manager::IsLeveledItemRefSwapped(const TESObjectREFR* a_refr) const
	{
		return swappedLeveledItemRefs.contains(a_refr->refID);
	}

	SwapResult Manager::GetSwapData(TESObjectREFR* a_ref, TESForm* a_base)
	{
		const auto get_swap_base = [a_ref](const TESForm* a_form, const SwapMap<SwapDataVec>& a_map) -> SwapResult {
			if (const auto it = a_map.find(a_form->refID); it != a_map.end()) {
				for (auto& swapData : it->second | std::ranges::views::reverse) {
					if (auto swapObject = swapData.GetSwapBase(a_ref)) {
						return { swapObject, swapData.transform };
					}
				}
			}
			return { nullptr, std::nullopt };
			};

		const auto get_transform = [&](const TESForm* a_form) -> TransformResult {
			if (const auto it = transforms.find(a_form->refID); it != transforms.end()) {
				for (auto& transformData : it->second | std::ranges::views::reverse) {
					if (transformData.IsTransformValid(a_ref)) {
						return transformData.transform;
					}
				}
			}
			return std::nullopt;
			};

		constexpr auto has_transform = [](const TransformResult& a_result) {
			return a_result && a_result->IsValid();
			};

		SwapResult swapData{ nullptr, std::nullopt };

		// get base
		if (a_ref->refID < 0xFF000000) {
			swapData = get_swap_base(a_ref, swapRefs);
		}

		if (!swapData.first) {
			swapData = GetSwapConditionalBase(a_ref, a_base);
		}

		if (!swapData.first) {
			swapData = get_swap_base(a_base, swapForms);
		}

		// get transforms
		if (!has_transform(swapData.second) && a_ref->refID < 0xFF000000) {
			swapData.second = get_transform(a_ref);
		}

		if (!has_transform(swapData.second)) {
			swapData.second = GetTransformConditional(a_ref, a_base);
		}

		if (!has_transform(swapData.second)) {
			swapData.second = get_transform(a_base);
		}

		return swapData;
	}
}