#include "CSVWriter.h"

#include "CSVFormulaType.h"
#include "CSVHelpers.h"
#include "CSVMode.h"

#include "Language.h"
#include "StormMacro.h"

#include <fstream>


namespace
{
	constexpr std::string_view k_axeName = "Axe";

	struct NumericLessConvertor
	{
	public:
		using is_transparent = int;

	public:
		template<class StrType, class OtherStrType>
		bool operator()(StrType &&left, OtherStrType &&right) const
		{
			return std::strtod(left.data(), nullptr) < std::strtod(right.data(), nullptr);
		}
	};
	
	template<const char skipChar, const char returnChar>
	void csvBlankSkip(std::ofstream &file, std::size_t allElementsCount)
	{
		file << returnChar;
		while (allElementsCount != 0)
		{
			file << skipChar;
			--allElementsCount;
		}
		file << returnChar;
	}

	template<Storm::CSVFormulaType formula, Storm::Language language>
	constexpr std::string_view retrieveFormulaKeyword()
	{
		if constexpr (formula == Storm::CSVFormulaType::Sum)
		{
			if constexpr (language == Storm::Language::English)
			{
				return "SUM";
			}
			else if constexpr (language == Storm::Language::French)
			{
				return "SOMME";
			}
			else
			{
				Storm::throwException<Storm::Exception>("Unknown language requested!");
			}
		}
		else
		{
			Storm::throwException<Storm::Exception>("Unknown formula type requested!");
		}
	}

	template<Storm::CSVMode mode, Storm::Language language>
	void writeFormula(std::ofstream &file, const Storm::CSVFormulaType formula, const std::size_t valuesCount, const std::size_t position)
	{
		if constexpr (mode == Storm::CSVMode::Row)
		{
			switch (formula)
			{
			case Storm::CSVFormulaType::Sum:
				file <<
					"=" << retrieveFormulaKeyword<Storm::CSVFormulaType::Sum, language>() << "(A" << position << ':' <<
					Storm::CSVHelpers::transcriptLetterPosition(valuesCount) << position << ')'
					;
				return;

			default:
				break;
			}
		}
		else if constexpr (mode == Storm::CSVMode::Columns)
		{
			const std::string letter = Storm::CSVHelpers::transcriptLetterPosition(position);
			switch (formula)
			{
			case Storm::CSVFormulaType::Sum:
				file << "=" << retrieveFormulaKeyword<Storm::CSVFormulaType::Sum, language>() << "(" << letter << "1:" << letter << valuesCount << ')';
				return;

			default:
				break;
			}
		}
		else if constexpr (mode == Storm::CSVMode::ThreeDimensional)
		{
			const std::string letter = Storm::CSVHelpers::transcriptLetterPosition(position);
			switch (formula)
			{
			case Storm::CSVFormulaType::Sum:
				file << "=" << retrieveFormulaKeyword<Storm::CSVFormulaType::Sum, language>() << "(" << letter << "1:" << letter << valuesCount << ')';
				return;

			default:
				break;
			}
		}
		else
		{
			assert(false && "Unknown csv mode! We should have not come here!");
			Storm::throwException<Storm::Exception>("Unknown csv mode! We should not come here!");
		}

		assert(false && "Unhandled formula type! We should have not come here!");
		Storm::throwException<Storm::Exception>("Unhandled formula type! We should have not come here!");
	}

	template <Storm::CSVMode mode, Storm::Language language, bool hasFormula, bool noAxis, class ElementMapType, class FormulaMapType>
	void writeAsRow(STORM_MAY_BE_UNUSED FormulaMapType &allFormulasRequests, const std::size_t maxElementCount, std::ofstream &file, const ElementMapType &elementMap)
	{
		STORM_MAY_BE_UNUSED std::size_t rowIter = 0;
		for (const auto &elements : elementMap)
		{
			if constexpr (noAxis)
			{
				if (elements.first == k_axeName) STORM_UNLIKELY
				{
					continue;
				}
			}

			file << elements.first;

			for (const auto &element : elements.second)
			{
				file << ',' << element;
			}

			if constexpr (hasFormula)
			{
				csvBlankSkip<',', ','>(file, maxElementCount - elements.second.size());

				if (auto found = allFormulasRequests.find(elements.first); found != std::end(allFormulasRequests))
				{
					// Csv cells start at index 1 (isn't 0-based index)
					writeFormula<mode, language>(file, found->second, maxElementCount + 1, rowIter);
					allFormulasRequests.erase(found);
				}

				++rowIter;
			}

			file << '\n';
		}
	}

	template<Storm::CSVMode mode, Storm::Language language, bool mismatch, class ElementMapType, class FormulaMapType>
	void write(ElementMapType &allElements, FormulaMapType &allFormulasRequests, const std::string &filePath, const std::size_t maxElementCount, STORM_MAY_BE_UNUSED const bool shouldNumericallySort)
	{
		std::filesystem::create_directories(std::filesystem::path{ filePath }.parent_path());

		std::ofstream file{ filePath };

		file << "sep=,\n";

		const std::size_t allElementsCount = allElements.size();
		assert(allElementsCount > 0 && "All Elements count check for emptiness should have been made outside this method!");

		if constexpr (mismatch)
		{
			LOG_WARNING << "Mismatch between row and columns in csv to be written. Some columns/rows won't have the same element count, therefore shifting could happen.";
		}

		if constexpr (mode == Storm::CSVMode::Row)
		{
			if (allFormulasRequests.empty())
			{
				writeAsRow<mode, language, false, false>(allFormulasRequests, maxElementCount, file, allElements);
			}
			else
			{
				writeAsRow<mode, language, true, false>(allFormulasRequests, maxElementCount, file, allElements);
			}
		}
		else if constexpr (mode == Storm::CSVMode::Columns)
		{
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

			if (!allFormulasRequests.empty())
			{
				csvBlankSkip<',', '\n'>(file, allElementsCount);

				std::size_t columnIter = 0;
				for (auto &elements : allElements)
				{
					if (!allFormulasRequests.empty())
					{
						if (auto found = allFormulasRequests.find(elements.first); found != std::end(allFormulasRequests))
						{
							// Csv cells start at index 1 (isn't 0-based index)
							writeFormula<mode, language>(file, found->second, maxElementCount + 1, columnIter);
							allFormulasRequests.erase(found);
						}
						
						++columnIter;
					}

					file << ',';
				}
			}
		}
		else if constexpr (mode == Storm::CSVMode::ThreeDimensional)
		{
			if (const auto rowAxe = allElements.find(k_axeName);
				rowAxe != std::end(allElements))
			{
				// Fill the first line with axe value.
				for (const auto &axeValue : rowAxe->second)
				{
					file << ',' << axeValue;
				}

				file << '\n';

				if (!shouldNumericallySort)
				{
					if (allFormulasRequests.empty())
					{
						writeAsRow<mode, language, false, true>(allFormulasRequests, maxElementCount, file, allElements);
					}
					else
					{
						writeAsRow<mode, language, true, true>(allFormulasRequests, maxElementCount, file, allElements);
					}
				}
				else
				{
					std::map<std::string_view, std::vector<std::string>, NumericLessConvertor> resortedElement;

					for (auto &elements : allElements)
					{
						if (elements.first != k_axeName)
						{
							resortedElement.try_emplace(elements.first, std::move(elements.second));
						}
					}

					if (allFormulasRequests.empty())
					{
						writeAsRow<mode, language, false, false>(allFormulasRequests, maxElementCount, file, resortedElement);
					}
					else
					{
						writeAsRow<mode, language, true, false>(allFormulasRequests, maxElementCount, file, resortedElement);
					}
				}
			}
			else
			{
				Storm::throwException<Storm::Exception>("No axe were given.");
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
		allFormulasRequests.clear();
	}
}


Storm::CSVWriter::CSVWriter(const std::string_view filePath, const Storm::Language language) :
	Storm::CSVWriter{ filePath, language, Storm::CSVMode::Columns }
{

}

Storm::CSVWriter::CSVWriter(const std::string_view filePath, const Storm::Language language, const Storm::CSVMode mode) :
	_filePath{ Storm::toStdString(std::filesystem::path{ filePath }.replace_extension(".csv")) },
	_mode{ mode },
	_language{ language },
	_shouldSortNumerically{ false }
{
	
}

Storm::CSVWriter::~CSVWriter()
{
	if (!_elements.empty())
	{
		std::size_t maxElementCount = _elements.begin()->second.size();
		bool mismatch = false;

		if (_mode == Storm::CSVMode::ThreeDimensional)
		{
			if (const auto rowAxe = _elements.find(k_axeName);
				rowAxe != std::end(_elements))
			{
				const std::size_t axeElemFillCount = rowAxe->second.size() + 1;

				for (const auto &elements : _elements)
				{
					if (rowAxe->first != elements.first)
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

						mismatch = mismatch || elementsCount != axeElemFillCount;
					}
				}
			}
			else
			{
				Storm::throwException<Storm::Exception>("No axe were given.");
			}
		}
		else
		{
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
		}

#define STORM_WRITE_LANGUAGE_CASE(modeName, languageName, ...) case languageName: write<modeName, languageName, __VA_ARGS__>(_elements, _formulas, _filePath, maxElementCount, _shouldSortNumerically); break

#define STORM_WRITE_MODE_CASE(modeName)														\
	case modeName:																			\
		if (mismatch)																		\
		{																					\
			switch (_language)																\
			{																				\
				STORM_WRITE_LANGUAGE_CASE(modeName, Storm::Language::French, true);			\
			default:																		\
				STORM_WRITE_LANGUAGE_CASE(modeName, Storm::Language::English, true);		\
			}																				\
		}																					\
		else																				\
		{																					\
			switch (_language)																\
			{																				\
				STORM_WRITE_LANGUAGE_CASE(modeName, Storm::Language::French, false);		\
			default:																		\
				STORM_WRITE_LANGUAGE_CASE(modeName, Storm::Language::English, false);		\
			}																				\
		}																					\
		break

		switch (_mode)
		{
			STORM_WRITE_MODE_CASE(Storm::CSVMode::Row);
			STORM_WRITE_MODE_CASE(Storm::CSVMode::Columns);
			STORM_WRITE_MODE_CASE(Storm::CSVMode::ThreeDimensional);

		default:
			assert(false && "Unknown CSV Mode!");
			LOG_FATAL << "Unknown CSV Mode !!";
		}
	}
}

void Storm::CSVWriter::append(const std::string_view keyName, const std::string& value)
{
	auto found = _elements.find(keyName);
	if (found == std::end(_elements))
	{
		std::vector<std::string> tmp{};
		tmp.reserve(_elements.empty() ? 16 : std::begin(_elements)->second.capacity());

		found = _elements.try_emplace(found, std::string{ keyName }, std::move(tmp));
	}

	std::string &elem = found->second.emplace_back();

	const std::size_t valueStrLength = value.size();
	if (valueStrLength > 0)
	{
		elem.reserve(valueStrLength + 2);

		elem += '"';
		elem += value;
		elem += '"';
	}
}

void Storm::CSVWriter::append(const std::string &keyName, const std::string &value)
{
	this->append(std::string_view{ keyName }, value);
}

void Storm::CSVWriter::addFormula(const std::string &keyName, Storm::CSVFormulaType formula)
{
	_formulas[keyName] = formula;
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
	return std::any_of(std::begin(_elements), std::end(_elements), [](const auto &elements) { return !elements.second.empty(); });
}

std::string_view Storm::CSVWriter::fetchAxeName() const noexcept
{
	return k_axeName;
}

void Storm::CSVWriter::setNumeric(bool isNumeric)
{
	assert(_mode == CSVMode::ThreeDimensional && "Numerical sort is only valid when we're creating axes value for a csv, therefore in 3D mode.");
	_shouldSortNumerically = isNumeric;
}
