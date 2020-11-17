#pragma once


namespace Storm
{
	class UIFieldContainer;

	class KernelHandler
	{
	public:
		KernelHandler();
		~KernelHandler();

	public:
		void initialize();
		void update(const float currentPhysicsTimeInSeconds);

	public:
		__forceinline float getKernelValue() const noexcept { return _currentKernelValue; }
		void setKernelValue(const float newValue);

	private:
		float _currentKernelValue;

		bool _shouldIncrementKernel;

		std::unique_ptr<Storm::UIFieldContainer> _uiFields;
	};
}
