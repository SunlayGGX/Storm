#pragma once


namespace Storm
{
	template<class SettingType>
	struct CorrectSettingChecker
	{
	private:
		template<class SettingType, SettingType expected, SettingType ... others>
		struct CorrectSettingCheckerImpl
		{
		public:
			static inline bool check(const SettingType currentSetting)
			{
				return
					Storm::CorrectSettingChecker<SettingType>::CorrectSettingCheckerImpl<SettingType, expected>::check(currentSetting) ||
					Storm::CorrectSettingChecker<SettingType>::CorrectSettingCheckerImpl<SettingType, others...>::check(currentSetting);
			}
		};

		template<class SettingType, SettingType expected>
		struct CorrectSettingCheckerImpl<SettingType, expected>
		{
		public:
			static inline bool check(const SettingType currentSetting)
			{
				return currentSetting == expected;
			}
		};

	public:
		template<SettingType ... expected>
		static inline bool check(const SettingType currentSetting)
		{
			return CorrectSettingCheckerImpl<SettingType, expected...>::check(currentSetting);
		}
	};
}
