#include "InternalConfigHolder.h"

#include "InternalConfig.h"

#include "InternalReferenceConfig.h"

#include "GeneratedWrapper.h"
#include "XmlReader.h"
#include "StringAlgo.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


namespace
{
	void validateInternalConfig(const std::filesystem::path &internalConfigPath)
	{
		if (internalConfigPath.empty())
		{
			Storm::throwException<Storm::Exception>("Internal config path shouldn't be empty!");
		}

		if (!std::filesystem::is_regular_file(internalConfigPath))
		{
			Storm::throwException<Storm::Exception>("Internal config path should point to a valid regular file!");
		}
	}

	// Internal config shouldn't be exposed and modified unless truly necessary. It gives infos to the application, not parametrize things.
	// Therefore we shouldn't encounter mistakes and errors.
	void throwUnknownTagException(const std::string &tagPath, const std::string_view parentTagPath)
	{
		Storm::throwException<Storm::Exception>("tag '" + tagPath + "' (inside " + parentTagPath + ") is unknown, therefore it cannot be handled");
	}

	void legalizeBibTexLocationIfNeeded(std::string &inOutBibTex, const std::filesystem::path &bibTexLibraryLocations)
	{
		if (!inOutBibTex.empty())
		{
			const std::filesystem::path currentBibTexLocation = bibTexLibraryLocations / inOutBibTex;
			inOutBibTex = currentBibTexLocation.string();

			if (!std::filesystem::is_regular_file(currentBibTexLocation))
			{
				Storm::throwException<Storm::Exception>(inOutBibTex + " is not a text file containing the bibTex reference!");
			}
		}
	}

	// ---------------- Parsers --------------- //
	
	void parseArticle(const boost::property_tree::ptree &tree, Storm::InternalReferenceConfig &article)
	{
		article._type = Storm::PaperType::Article;

		// Mandatory
		Storm::XmlReader::sureReadXmlAttribute(tree, article._authors, "authors");
		Storm::XmlReader::sureReadXmlAttribute(tree, article._name, "name");
		Storm::XmlReader::sureReadXmlAttribute(tree, article._date, "date");
		Storm::XmlReader::sureReadXmlAttribute(tree, article._serialNumber, "serialNumber");

		// optional
		Storm::XmlReader::readXmlAttribute(tree, article._url, "url");
		Storm::XmlReader::readXmlAttribute(tree, article._bibTexLink, "bibTex");
	}

	void parseMisc(const boost::property_tree::ptree &tree, Storm::InternalReferenceConfig &article)
	{
		article._type = Storm::PaperType::Misc;

		// Mandatory
		Storm::XmlReader::sureReadXmlAttribute(tree, article._authors, "authors");
		Storm::XmlReader::sureReadXmlAttribute(tree, article._name, "name");

		// optional
		Storm::XmlReader::readXmlAttribute(tree, article._date, "date");
		Storm::XmlReader::readXmlAttribute(tree, article._serialNumber, "serialNumber");
		Storm::XmlReader::readXmlAttribute(tree, article._url, "url");
		Storm::XmlReader::readXmlAttribute(tree, article._bibTexLink, "bibTex");
	}
}


Storm::InternalConfigHolder::InternalConfigHolder() :
	_internalConfig{ std::make_unique<Storm::InternalConfig>() }
{

}

Storm::InternalConfigHolder::~InternalConfigHolder() = default;

void Storm::InternalConfigHolder::init()
{
	Storm::initGitGeneratedConfig(*_internalConfig->_generatedGitConfig);
}

void Storm::InternalConfigHolder::read(const std::filesystem::path &internalConfigPath, const Storm::MacroConfig &macroConfig)
{
	validateInternalConfig(internalConfigPath);

	const std::filesystem::path bibTexLibraryLocations = internalConfigPath.parent_path() / "BibTex";

	boost::property_tree::ptree xmlTree;
	boost::property_tree::read_xml(internalConfigPath.string(), xmlTree, boost::property_tree::xml_parser::no_comments);

	const auto &srcTree = xmlTree.get_child("Internal");

	/* Papers */
	const auto &referencesTree = srcTree.get_child("References");
	
	std::vector<Storm::InternalReferenceConfig> &reference = _internalConfig->_internalReferencesConfig;
	reference.reserve(referencesTree.size());

	for (const auto &referenceXmlElement : referencesTree)
	{
		Storm::InternalReferenceConfig &newReference = reference.emplace_back();
		newReference._id = reference.size();

		if (
			!Storm::XmlReader::handleXml(referenceXmlElement, "article", newReference, parseArticle) &&
			!Storm::XmlReader::handleXml(referenceXmlElement, "misc", newReference, parseMisc)
			)
		{
			throwUnknownTagException(referenceXmlElement.first, "Internal.Papers");
		}

		legalizeBibTexLocationIfNeeded(newReference._bibTexLink, bibTexLibraryLocations);

		if (newReference._authors.empty())
		{
			Storm::throwException<Storm::Exception>("Authors of a reference should be specified (you legally MUST cite the authors)!");
		}
		else if (newReference._name.empty())
		{
			Storm::throwException<Storm::Exception>("A reference should have a name!");
		}

		Storm::StringAlgo::replaceAll(newReference._name, '\n', Storm::StringAlgo::makeReplacePredicate("\\n"));
		Storm::StringAlgo::replaceAll(newReference._authors, ',', Storm::StringAlgo::makeReplacePredicate(", ", ",\t"));

		switch (newReference._type)
		{
		case Storm::PaperType::Article:
			if (newReference._date.empty())
			{
				Storm::throwException<Storm::Exception>("An article should have a date, or at least a year, of publication!");
			}
			else if (newReference._serialNumber.empty())
			{
				Storm::throwException<Storm::Exception>("An article should have a something to uniquely identifies it like a serial number, DOI, ISBN, ... !");
			}
			break;

		case Storm::PaperType::Misc:
			break;

		case Storm::PaperType::Unknown:
		default:
			Storm::throwException<Storm::Exception>("Unknown reference type!");
		}
	}
}

const Storm::InternalConfig& Storm::InternalConfigHolder::getInternalConfig() const
{
	return *_internalConfig;
}
