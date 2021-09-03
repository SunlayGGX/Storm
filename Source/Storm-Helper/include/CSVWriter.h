#pragma once


namespace Storm
{
	class CSVElement;
	enum class CSVMode;

	class CSVWriter
	{
	public:
		CSVWriter(const std::string_view filePath);
		CSVWriter(const std::string_view filePath, const Storm::CSVMode mode);
		~CSVWriter();

	private:
		void append(const std::string_view keyName, const std::string &value);

	public:
		void operator()(const std::string_view keyName, const std::string &value);

		template<class Type>
		void operator()(const std::string_view keyName, Type &&value)
		{
			this->append(keyName, Storm::toStdString(std::forward<Type>(value)));
		}

	public:
		void clear(const bool keepKeys = false);
		void reserve(const std::size_t count);
		bool empty() const;

	private:
		std::map<std::string_view, std::vector<std::string>> _elements;
		const std::string _filePath;
		const Storm::CSVMode _mode;
	};
}
