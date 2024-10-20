#pragma once

#include "Defs.h"

namespace BaseObjectSwapper
{
	inline srell::regex genericRegex{ R"(\((.*?)\))" };

	struct Traits
	{
		Traits() = default;
		explicit Traits(const std::string& a_str);

		// members
		bool          trueRandom{ false };
		std::uint32_t chance{ 100 };
	};

	class Transform
	{
	public:
		Transform() = default;
		explicit Transform(const std::string& a_str);

		void SetTransform(TESObjectREFR* a_refr) const;
		bool IsValid() const;

		bool operator==(Transform const& a_rhs) const
		{
			return location && a_rhs.location || rotation && a_rhs.rotation || refScale && a_rhs.refScale || refDisable && a_rhs.refDisable || modelSwap && a_rhs.modelSwap;
		}

	private:
		[[nodiscard]] static RelData<NiPoint3> get_transform_from_string(const std::string& a_str, bool a_convertToRad = false);
		[[nodiscard]] static MinMax<float>         get_scale_from_string(const std::string& a_str);
		[[nodiscard]] static std::string         get_path_from_string(const std::string& a_str);

		struct Input
		{
			bool       trueRandom{ false };
			std::uint32_t refSeed{ 0 };

			bool  clamp{ false };
			float clampMin{ 0.0f };
			float clampMax{ 0.0f };
		};

		static float        get_random_value(const Input& a_input, float a_min, float a_max);
		static NiPoint3 get_random_value(const Input& a_input, const std::pair<NiPoint3, NiPoint3>& a_minMax);

		// members
		std::optional<RelData<NiPoint3>> location{ std::nullopt };
		std::optional<RelData<NiPoint3>> rotation{ std::nullopt };
		std::optional<MinMax<float>>         refScale{ std::nullopt };
		std::optional<bool>         refDisable{ std::nullopt };
		std::optional<bool>         modelSwap{ std::nullopt };
		std::optional<bool>         turnOffFire{ std::nullopt };
		std::string         modelPath;

		bool useTrueRandom{ false };

		static inline srell::regex transformRegex{ R"(\((.*?),(.*?),(.*?)\))" };
		static inline srell::regex stringRegex{ R"(,\s*(?![^()]*\)))" };

		friend class TransformData;
	};

	class TransformData
	{
	public:
		struct Input
		{
			std::string transformStr;
			std::string traitsStr;
			std::string record;
			std::string path;
		};

		TransformData() = delete;
		explicit TransformData(const Input& a_input);

		[[nodiscard]] static std::uint32_t GetFormID(const std::string& a_str);
		bool                            IsTransformValid(const TESObjectREFR* a_ref) const;

		static void GetTransforms(const std::string& a_path, const std::string& a_str, std::function<void(std::uint32_t, TransformData&)> a_func);

		// members
		Transform transform{};
		Traits    traits{};

		std::string record{};
		std::string path{};
	};

	class SwapData : public TransformData
	{
	public:
		SwapData() = delete;
		SwapData(FormIDOrSet a_id, const Input& a_input);

		[[nodiscard]] static FormIDOrSet GetSwapFormID(const std::string& a_str);
		TESBoundObject* GetSwapBase(const TESObjectREFR* a_ref) const;

		static void GetForms(const std::string& a_path, const std::string& a_str, std::function<void(std::uint32_t, SwapData&)> a_func);

		// members
		FormIDOrSet formIDSet{};
	};

	using SwapDataVec = std::vector<SwapData>;
	using TransformDataVec = std::vector<TransformData>;

	using SwapDataConditional = std::unordered_map<FormIDStr, SwapDataVec>;
	using TransformDataConditional = std::unordered_map<FormIDStr, TransformDataVec>;

	using TransformResult = std::optional<Transform>;
	using SwapResult = std::pair<TESBoundObject*, TransformResult>;
}