#pragma once


namespace Storm
{
	enum class CSVMode;
	enum class CSVFormulaType;

	class CSVWriter
	{
	private:
		template<class Type>
		static auto makeDefaultTransferFunctor()
		{
			return []<class Type2 = Type>(Type2 &&val) -> decltype(auto)
			{
				return std::forward<Type2>(val);
			};
		}

	public:
		CSVWriter(const std::string_view filePath);
		CSVWriter(const std::string_view filePath, const Storm::CSVMode mode);
		~CSVWriter();

	private:
		void append(const std::string &keyName, const std::string &value);

	public:
		template<class Type, class TransferFunctor = decltype(CSVWriter::makeDefaultTransferFunctor<Type>())>
		void operator()(const std::string &keyName, Type &&value, const TransferFunctor &functor = CSVWriter::makeDefaultTransferFunctor<Type>())
		{
			this->append(keyName, Storm::toStdString(functor(std::forward<Type>(value))));
		}

		template<class Type, class TransferFunctor = decltype(CSVWriter::makeDefaultTransferFunctor<Type>())>
		void operator()(const std::string &keyName, const std::vector<Type> &values, const TransferFunctor &functor = CSVWriter::makeDefaultTransferFunctor<Type>())
		{
			for (const Type &element : values)
			{
				this->operator()(keyName, element, functor);
			}
		}

	public:
		void addFormula(const std::string &keyName, Storm::CSVFormulaType formula);

	public:
		void clear(const bool keepKeys = false);
		void reserve(const std::size_t count);
		bool empty() const;

	private:
		std::map<std::string, std::vector<std::string>> _elements;
		const std::string _filePath;
		const Storm::CSVMode _mode;
		std::map<std::string, Storm::CSVFormulaType> _formulas;
	};
}
