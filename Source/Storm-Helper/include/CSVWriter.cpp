#include "CSVWriter.h"
#include "CSVMode.h"

#include <fstream>


namespace
{
	template<Storm::CSVMode mode, bool mismatch>
	void write(std::map<std::string_view, std::vector<std::string>> &allElements, const std::string &filePath, const std::size_t maxElementCount)
	{
		std::filesystem::create_directories(std::filesystem::path{ filePath }.parent_path());

		std::ofstream file{ filePath };

		const std::size_t allElementsCount = allElements.size();
		assert(allElementsCount > 0 && "All Elements count check for emptiness should have been made outside this method!");

		if constexpr (mode == Storm::CSVMode::Row)
		{
			if constexpr (mismatch)
			{
				LOG_WARNING << "Mismatch between row and columns in csv to be written. Some row won't have the same element count, therefore shifting could happen.";
			}

			for (const auto &elements : allElements)
			{
				file << elements.first;

				for (const auto &element : elements.second)
				{
					file << ',' << element;
				}

				file << '\n';
			}
		}
		else if constexpr (mode == Storm::CSVMode::Columns)
		{
			if constexpr (mismatch)
			{
				LOG_WARNING << "Mismatch between row and columns in csv to be written. Some columns won't have the same element count, therefore shifting could happen.";
			}

			std::vector<std::vector<std::string>> contiguousAllElements;
			contiguousAllElements.reserve(allElementsCount);

			for (auto &elements : allElements)
			{
				file << elements.first << ',';
				contiguousAllElements.emplace_back(std::move(elements.second));
			}

			for (std::size_t iter = 0; iter < maxElementCount; ++iter)
			{
				file << '\n';
				for (const auto &elements : contiguousAllElements)
				{
					if constexpr (mismatch)
					{
						if (elements.size() < maxElementCount)
						{
							file << elements[iter];
						}

						file << ',';
					}
					else
					{
						file << elements[iter] << ',';
					}
				}
			}
		}
		else
		{
			assert(false && "Unknown csv mode! We should have not come here!");
			Storm::throwException<Storm::Exception>("Unknown csv mode! We should not come here!");
		}

		LOG_DEBUG <<
			"We have successfully written csv file inside \"" << filePath << "\"\n" <<
			"Written " << maxElementCount << " elements into " << allElementsCount << " key headers.\n"
			"Total elements written : " << maxElementCount * allElementsCount << " elements.";

		allElements.clear();
	}
}


Storm::CSVWriter::CSVWriter(const std::string_view filePath) :
	Storm::CSVWriter{ filePath, Storm::CSVMode::Columns }
{

}

Storm::CSVWriter::CSVWriter(const std::string_view filePath, const Storm::CSVMode mode) :
	_filePath{ Storm::toStdString(std::filesystem::path{ filePath }.replace_extension(".csv")) },
	_mode{ mode }
{
	
}

Storm::CSVWriter::~CSVWriter()
{
	if (!_elements.empty())
	{
		std::size_t maxElementCount = _elements.begin()->second.size();
		bool mismatch = false;
		for (const auto &elements : _elements)
		{
			const std::size_t elementsCount = elements.second.size();
			if (elementsCount != maxElementCount)
			{
				mismatch = true;
				if (elementsCount > maxElementCount)
				{
					maxElementCount = elementsCount;
				}
			}
		}

		switch (_mode)
		{
		case Storm::CSVMode::Row:
			if (mismatch)
			{
				write<Storm::CSVMode::Row, true>(_elements, _filePath, maxElementCount);
			}
			else
			{
				write<Storm::CSVMode::Row, false>(_elements, _filePath, maxElementCount);
			}
			break;

		case Storm::CSVMode::Columns:
			if (mismatch)
			{
				write<Storm::CSVMode::Columns, true>(_elements, _filePath, maxElementCount);
			}
			else
			{
				write<Storm::CSVMode::Columns, false>(_elements, _filePath, maxElementCount);
			}
			break;

		default:
			assert(false && "Unknown CSV Mode!");
			LOG_FATAL << "Unknown CSV Mode !!";
		}
	}
}

void Storm::CSVWriter::append(const std::string_view keyName, const std::string &value)
{
	_elements[keyName].emplace_back('"' + value + '"');
}

void Storm::CSVWriter::operator()(const std::string_view keyName, const std::string &value)
{
	this->append(keyName, value);
}

void Storm::CSVWriter::clear(const bool keepKeys /*= false*/)
{
	if (keepKeys)
	{
		for (auto &elements : _elements)
		{
			elements.second.clear();
		}
	}
	else
	{
		_elements.clear();
	}
}

void Storm::CSVWriter::reserve(const std::size_t count)
{
	for (auto &elements : _elements)
	{
		elements.second.reserve(count);
	}
}

bool Storm::CSVWriter::empty() const
{
	for (const auto &elements : _elements)
	{
		if (!elements.second.empty())
		{
			return true;
		}
	}

	return false;
}
