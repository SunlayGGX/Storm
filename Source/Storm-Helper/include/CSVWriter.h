#pragma once


namespace Storm
{
	enum class CSVMode;
	enum class CSVFormulaType;
	enum class Language;

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
		CSVWriter(const std::string_view filePath, const Storm::Language language);
		CSVWriter(const std::string_view filePath, const Storm::Language language, const Storm::CSVMode mode);
		~CSVWriter();

	private:
		void append(const std::string_view keyName, const std::string &value);
		void append(const std::string &keyName, const std::string &value);

		template<class KeyType, std::size_t count>
		void append(const KeyType(&keyName)[count], const std::string &value)
		{
			this->append(std::string_view{ keyName, count }, value);
		}

	public:
		template<class KeyType, class Type, class TransferFunctor = decltype(CSVWriter::makeDefaultTransferFunctor<Type>())>
		void operator()(const KeyType &keyName, Type &&value, const TransferFunctor &functor = CSVWriter::makeDefaultTransferFunctor<Type>())
		{
			this->append(keyName, Storm::toStdString(functor(std::forward<Type>(value))));
		}

		template<class KeyType, class Type, class TransferFunctor = decltype(CSVWriter::makeDefaultTransferFunctor<Type>())>
		void operator()(const KeyType &keyName, const std::vector<Type> &values, const TransferFunctor &functor = CSVWriter::makeDefaultTransferFunctor<Type>())
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

	public:
		std::string_view fetchAxeName() const noexcept;

		template<class Type, class TransferFunctor = decltype(CSVWriter::makeDefaultTransferFunctor<Type>())>
		void defineRowAxeValue(Type &&value, const TransferFunctor &functor = CSVWriter::makeDefaultTransferFunctor<Type>())
		{
			(*this)(this->fetchAxeName(), std::forward<Type>(value), functor);
		}

		void setNumeric(bool isNumeric);

	private:
		std::map<std::string, std::vector<std::string>, std::less<void>> _elements;
		const std::string _filePath;
		const Storm::CSVMode _mode;
		std::map<std::string, Storm::CSVFormulaType, std::less<void>> _formulas;
		Storm::Language _language;
		bool _shouldSortNumerically; // Only for 3D
	};
}
