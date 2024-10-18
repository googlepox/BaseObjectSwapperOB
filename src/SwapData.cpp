#include "SwapData.h"

#define DEGTORAD 0.01745329252f

constexpr float PI = static_cast<float>(3.1415926535897932);;
constexpr float HALF_PI = 0.5F * PI;
constexpr float TWO_PI = 2.0F * PI;

namespace BaseObjectSwapper
{
	namespace detail
	{
		MinMax<float> get_min_max(const std::string& a_str)
		{
			constexpr auto get_float = [](const std::string& str) {
				return string::to_num<float>(str);
				};

			if (const auto splitNum = string::split(a_str, R"(/)"); splitNum.size() > 1) {
				return { get_float(splitNum[0]), get_float(splitNum[1]) };
			}
			else {
				auto num = get_float(a_str);
				return { num, num };
			}
		}
	}

	Traits::Traits(const std::string& a_str)
	{
		if (dist::is_valid_entry(a_str)) {
			if (a_str.contains("chance")) {
				if (a_str.contains("R")) {
					trueRandom = true;
				}
				if (srell::cmatch match; srell::regex_search(a_str.c_str(), match, genericRegex)) {
					chance = string::to_num<std::uint32_t>(match[1].str());
				}
			}
		}
	}

	RelData<NiPoint3> Transform::get_transform_from_string(const std::string& a_str, bool a_convertToRad)
	{
		MinMax<NiPoint3> transformData;

		const auto get_transform = [&](const std::string& b_str) -> NiPoint2 {
			auto [min, max] = detail::get_min_max(b_str);
			return {
				a_convertToRad ? DEGTORAD * min : min,
				a_convertToRad ? DEGTORAD * max : max
			};
			};

		if (srell::cmatch match; srell::regex_search(a_str.c_str(), match, transformRegex)) {
			auto [minX, maxX] = get_transform(match[1].str());  //match[0] gets the whole string
			transformData.first.x = minX;
			transformData.second.x = maxX;

			auto [minY, maxY] = get_transform(match[2].str());
			transformData.first.y = minY;
			transformData.second.y = maxY;

			const auto [minZ, maxZ] = get_transform(match[3].str());
			transformData.first.z = minZ;
			transformData.second.z = maxZ;
		}

		return { a_str.contains('R'), transformData };
	}

	MinMax<float> Transform::get_scale_from_string(const std::string& a_str)
	{
		srell::cmatch match;
		if (srell::regex_search(a_str.c_str(), match, genericRegex)) {
			return detail::get_min_max(match[1].str());
		}

		return MinMax<float>{ 0.0f, 0.0f };
	}

	std::string Transform::get_path_from_string(const std::string& a_str)
	{
		srell::cmatch match;
		if (srell::regex_search(a_str.c_str(), match, genericRegex)) {
			//_MESSAGE("original string: %s", a_str.c_str());
			//_MESSAGE("match string: %s", match[1].str().c_str());
			return match[1].str();
		}

		return "";
	}

	float Transform::get_random_value(const Input& a_input, float a_min, float a_max)
	{
		float value = a_min;

		if (!numeric::essentially_equal(a_min, a_max)) {
			value = a_input.trueRandom ? SeedRNG().Generate(a_min, a_max) :
				SeedRNG(a_input.refSeed).Generate(a_min, a_max);
		}

		if (a_input.clamp) {
			value = std::clamp(value, a_input.clampMin, a_input.clampMax);
		}

		return value;
	}

	NiPoint3 Transform::get_random_value(const Input& a_input, const std::pair<NiPoint3, NiPoint3>& a_minMax)
	{
		auto& [min, max] = a_minMax;

		if (min.FltEquals(&max)) {
			return min;
		}

		return NiPoint3{
			get_random_value(a_input, min.x, max.x),
			get_random_value(a_input, min.y, max.y),
			get_random_value(a_input, min.z, max.z)
		};
	}

	Transform::Transform(const std::string& a_str)
	{
		// ignore commas within parentheses
		const auto get_split_transform = [&]() -> std::vector<std::string> {
			srell::sregex_token_iterator iter(a_str.begin(),
				a_str.end(),
				stringRegex,
				-1);
			srell::sregex_token_iterator end{};
			return { iter, end };
			};

		if (dist::is_valid_entry(a_str)) {
			const auto transformStrs = get_split_transform();
			for (auto& transformStr : transformStrs) {
				if (transformStr.contains("pos")) {
					location = get_transform_from_string(transformStr);
				}
				else if (transformStr.contains("rot")) {
					rotation = get_transform_from_string(transformStr, true);
				}
				else if (transformStr.contains("scale")) {
					refScale = get_scale_from_string(transformStr);
				}
				else if (transformStr.contains("disable")) {
					refDisable = true;
				}
				else if (transformStr.contains("model")) {
					_MESSAGE("transformStr: %s", transformStr.c_str());
					modelSwap = true;
					modelPath = get_path_from_string(transformStr);
				}
			}
		}
	}

	void Transform::SetTransform(TESObjectREFR* a_refr) const
	{
		if (location || rotation || refScale || refDisable || modelSwap) {
			Input input(useTrueRandom, a_refr->refID);
			if (location) {
				auto& [relative, minMax] = *location;
				const auto randPos = get_random_value(input, minMax);
				if (relative) {
					a_refr->posX += randPos.x;
					a_refr->posY += randPos.y;
					a_refr->posZ += randPos.z;
				}
				else {
					a_refr->posX = randPos.x;
					a_refr->posY = randPos.y;
					a_refr->posZ = randPos.z;
				}
			}
			if (rotation) {
				input.clamp = true;
				input.clampMin = -TWO_PI;
				input.clampMax = TWO_PI;

				auto& [relative, minMax] = *rotation;
				const auto randRot = get_random_value(input, minMax);
				if (relative) {
					a_refr->rotX += randRot.x;
					a_refr->rotY += randRot.y;
					a_refr->rotZ += randRot.z;
				}
				else {
					a_refr->rotX = randRot.x;
					a_refr->rotY = randRot.y;
					a_refr->rotZ = randRot.z;
				}
			}
			if (refScale) {
				input.clamp = true;
				input.clampMin = 0.0f;
				input.clampMax = 1000.0f;

				auto& [min, max] = *refScale;
				a_refr->scale = a_refr->scale * get_random_value(input, min, max);
			}
			if (refDisable) {
				if (a_refr->baseExtraList.HasType(kExtraData_EnableStateParent) || a_refr->baseExtraList.HasType(kExtraData_EnableStateChildren)) {
					//_MESSAGE("skipping disable as reference has EnableStateParent flag");
				}
				else if (a_refr->IsPersistent()) {
					//_MESSAGE("skipping disable as reference is flagged as Persistent");
				}
				else {
					a_refr->SetDisabled(true);
				}
			}
			if (modelSwap) {
				TESModel* model = OBLIVION_CAST(a_refr, TESForm, TESModel);
				if (model && model->nifPath.m_data != modelPath.c_str()) {
					model->SetModelPath(modelPath.c_str());
				}
			}
		}
	}

	bool Transform::IsValid() const
	{
		return location || rotation || refScale || refDisable || modelSwap;
	}

	TransformData::TransformData(const Input& a_input) :
		transform(a_input.transformStr),
		traits(a_input.traitsStr),
		record(a_input.record),
		path(a_input.path)
	{
		if (traits.trueRandom) {
			transform.useTrueRandom = true;
		}
	}

	SwapData::SwapData(FormIDOrSet a_id, const Input& a_input) :
		TransformData(a_input),
		formIDSet(std::move(a_id))
	{}

	std::uint32_t TransformData::GetFormID(const std::string& a_str)
	{
		constexpr auto lookup_formID = [](std::uint32_t a_refID, const std::string& modName) -> std::uint32_t
			{
				const auto modIdx = (*g_dataHandler)->GetModIndex(modName.c_str());
				return modIdx == 0xFF ? 0 : (a_refID & 0xFFFFFF) | modIdx << 24;
			};

		if (const auto splitID = string::split(a_str, "~"); splitID.size() == 2) {
			const auto  formID = string::to_num<std::uint32_t>(splitID[0], true);
			const auto& modName = splitID[1];
			return lookup_formID(formID, modName);
		}
		if (string::is_only_hex(a_str, true))
		{
			if (const auto form = LookupFormByID(string::to_num<std::uint32_t>(a_str, true))) {
				return form->refID;
			}
		}
		/*if (const auto form = GetFormByID(a_str.c_str())) {
			return form->refID;
		} */
		return 0;
	}

	void TransformData::GetTransforms(const std::string& a_path, const std::string& a_str, std::function<void(std::uint32_t, TransformData&)> a_func)
	{
		const auto formPair = string::split(a_str, "|");

		if (const auto baseFormID = GetFormID(formPair[0]); baseFormID != 0) {
			const Input input(
				formPair[1],                                        // transform
				formPair.size() > 2 ? formPair[2] : std::string{},  // traits
				a_str,
				a_path);
			TransformData transformData(input);
			a_func(baseFormID, transformData);
		}
		else {
			_ERROR("\t\t\tfailed to process %s (BASE formID not found)", a_str.c_str());
		}
	}

	bool TransformData::IsTransformValid(const TESObjectREFR* a_ref) const
	{
		if (traits.chance != 100) {
			const auto rng = traits.trueRandom ? SeedRNG().Generate<std::uint32_t>(0, 100) :
				SeedRNG(static_cast<std::uint32_t>(a_ref->refID)).Generate<std::uint32_t>(0, 100);
			if (rng > traits.chance) {
				return false;
			}
		}

		return transform.IsValid();
	}

	FormIDOrSet SwapData::GetSwapFormID(const std::string& a_str)
	{
		if (a_str.contains(",")) {
			FormIDSet  set;
			const auto IDStrs = string::split(a_str, ",");
			set.reserve(IDStrs.size());
			for (auto& IDStr : IDStrs) {
				if (auto formID = GetFormID(IDStr); formID != 0) {
					set.emplace(formID);
				}
				else {
					_ERROR("\t\t\tfailed to process %s (SWAP formID not found)", IDStr.c_str());
				}
			}
			return set;
		}
		else {
			return GetFormID(a_str);
		}
	}

	TESBoundObject* SwapData::GetSwapBase(const TESObjectREFR* a_ref) const
	{
		auto seededRNG = SeedRNG(static_cast<std::uint32_t>(a_ref->refID));

		if (traits.chance != 100) {
			const auto rng = traits.trueRandom ? SeedRNG().Generate<std::uint32_t>(0, 100) :
				seededRNG.Generate<std::uint32_t>(0, 100);
			if (rng > traits.chance) {
				return nullptr;
			}
		}

		if (const auto formID = std::get_if<std::uint32_t>(&formIDSet); formID) {
			return static_cast<TESBoundObject*>(LookupFormByID(*formID));
		}
		else {  // return random element from set
			auto& set = std::get<FormIDSet>(formIDSet);

			const auto setEnd = std::distance(set.begin(), set.end()) - 1;
			const auto randIt = traits.trueRandom ? SeedRNG().Generate<std::int64_t>(0, setEnd) :
				seededRNG.Generate<std::int64_t>(0, setEnd);

			return static_cast<TESBoundObject*>(LookupFormByID(*std::next(set.begin(), randIt)));
		}
	}

	void SwapData::GetForms(const std::string& a_path, const std::string& a_str, std::function<void(std::uint32_t, SwapData&)> a_func)
	{
		constexpr auto swap_empty = [](const FormIDOrSet& a_set) {
			if (const auto formID = std::get_if<std::uint32_t>(&a_set); formID) {
				return *formID == 0;
			}
			else {
				return std::get<FormIDSet>(a_set).empty();
			}
			};

		const auto formPair = string::split(a_str, "|");

		if (const auto baseFormID = GetFormID(formPair[0]); baseFormID != 0) {
			if (const auto swapFormID = GetSwapFormID(formPair[1]); !swap_empty(swapFormID)) {
				const Input input(
					formPair.size() > 2 ? formPair[2] : std::string{},  // transform
					formPair.size() > 3 ? formPair[3] : std::string{},  // traits
					a_str,
					a_path);
				SwapData swapData(swapFormID, input);
				a_func(baseFormID, swapData);
			}
			else {
				_ERROR("\t\t\tfailed to process %s (SWAP formID not found)", a_str.c_str());
			}
		}
		else {
			_ERROR("\t\t\tfailed to process %s (BASE formID not found)", a_str.c_str());
		}
	}
}