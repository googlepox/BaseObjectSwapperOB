#include "Manager.h"
#include <string>

#if OBLIVION
extern OBSEScriptInterface* g_script;
#endif

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

	bool HasKeyword(TESForm* a_form, const std::string& a_keyword)
	{
		if (!HasKeywordScript)
		{
			HasKeywordScript = g_script->CompileScript(
				R"(Begin Function { Ref akForm, string_var asKeyword }
					SetFunctionValue (HasKeyword akForm (asKeyword))
				End)");
		}

		OBSEArrayVarInterface::Element result;
		g_script->CallFunction(HasKeywordScript, nullptr, nullptr, &result, 2, a_form, a_keyword.c_str());

		return result.Number() != 0.0;
	}

	bool ConditionalInput::IsValid(const FormIDStr& a_data) const
	{
		if (std::holds_alternative<RE::FormID>(a_data)) {
			if (const auto form = LookupFormByID(std::get<RE::FormID>(a_data))) {
				switch (form->typeID) {
				case kFormType_Region:
				{
					if (const auto region = (form)) {
						if (const auto regionList = dynamic_cast<ExtraRegionList*>(currentCell ? currentCell->extraData.GetByType(kExtraData_RegionList) : nullptr)) {
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
		else {
			return HasKeyword(ref, std::get<std::string>(a_data));
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

		const std::filesystem::path bosFolder{ R"(Data\BaseObjectSwapper)" };
		if (!exists(bosFolder)) {
			_WARNING("BOS folder not found...");
			return;
		}

		const auto configs = dist::get_configs(R"(Data\BaseObjectSwapper)");

		if (configs.empty()) {
			_WARNING("No .ini files were found in Data\\BaseObjectSwapper folder, aborting...");
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
				if (const auto processedID = SwapData::GetFormID(a_condition); processedID != 0) {
					a_processedFilters.emplace_back(processedID);
				}
				else {
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
			Console_Print(std::format("[BOS] Conflicts found, check po3_BaseObjectSwapper.log in {} for more info\n", GetOblivionDirectory()).c_str());
		}
	}

	SwapResult Manager::GetSwapConditionalBase(TESObjectREFR* a_ref, TESForm* a_base)
	{
		if (const auto it = swapFormsConditional.find(static_cast<std::uint32_t>(a_base->refID)); it != swapFormsConditional.end()) {
			const ConditionalInput input(a_ref, a_base);
			const auto             result = std::ranges::find_if(it->second, [&](const auto& a_data) {
				return input.IsValid(a_data.first);
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
				return input.IsValid(a_data.first);
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
