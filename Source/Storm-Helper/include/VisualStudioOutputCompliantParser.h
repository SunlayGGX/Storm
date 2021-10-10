#pragma once


namespace Storm
{
	template<class IncrementalParentType>
	class VisualStudioOutputCompliantParser :
		private IncrementalParentType
	{
	private:
		template<int dummy = 0>
		constexpr static auto hasValidIncrementParent(int) -> decltype(IncrementalParentType::getValue(), bool())
		{
			return true;
		}

		template<int dummy = 0>
		constexpr static bool hasValidIncrementParent(void*)
		{
			return false;
		}

		enum : bool
		{
			k_enableIncrement = Storm::VisualStudioOutputCompliantParser<IncrementalParentType>::hasValidIncrementParent(0)
		};

	public:
		VisualStudioOutputCompliantParser(const std::string_view initialMsg, std::size_t expectedMsgCount)
		{
			enum
			{
				k_expectedFirstReserveMsgSize = 128,
				k_frameFirstReserveSize = 128,
				k_decoratorMsgSize = 6,
				k_firstReserveSizeSum = k_expectedFirstReserveMsgSize + k_frameFirstReserveSize + k_decoratorMsgSize
			};

			if constexpr (k_enableIncrement)
			{
				enum
				{
					k_incrementReserveSize = 5,
				};

				expectedMsgCount *= (k_firstReserveSizeSum + k_incrementReserveSize);
			}
			else
			{
				expectedMsgCount *= k_firstReserveSizeSum;
			}

			const std::size_t initialMsgSize = initialMsg.size();
			if (initialMsgSize > 0)
			{
				_output.reserve(initialMsgSize + 1 + expectedMsgCount);
				_output += initialMsg;
				_output += '\n';
			}
			else
			{
				_output.reserve(expectedMsgCount);
			}
		}

	public:
		void appendMessage(const std::string_view filePath, const std::size_t line, const std::string_view msg)
		{
			if constexpr (k_enableIncrement)
			{
				_output += ' ';
				_output += std::to_string(IncrementalParentType::getValue());
				_output += '>';
			}

			_output += filePath;
			_output += '(';
			_output += std::to_string(line);
			_output += ") : ";

			_output += msg;
			_output += '\n';
		}

		auto&& get() &&
		{
			return std::move(_output);
		}

	private:
		std::string _output;
	};
}
