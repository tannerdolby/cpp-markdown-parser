#include <iostream>
#include <unordered_map>
#include <regex>
#include "helpers.h"

std::string& ltrim(std::string&, const char* = " \t\n\r\f\v");
std::string& rtrim(std::string&, const char* = " \t\n\r\f\v");
std::string& trim(std::string&, const char* = " \t\n\r\f\v");

class Element {
public:
	std::string textContent;
	std::string element;
	std::string tagName;
	std::string attrStr;
	std::unordered_map<std::string, std::string> elemMap;

	Element(std::unordered_map<std::string, std::string> mp, std::string tag) {
		textContent = "";
		element = "";
		attrStr = "";
		tagName = tag;
		elemMap = mp;
	}

	std::string create() {

		if (tagName == "img") {
			// no text content
			checkForAttribute(elemMap, tagName);
		} else if (tagName == "h1" || tagName == "h2" || tagName == "h3"
				|| tagName == "h4" || tagName == "h5" || tagName == "h6") {
			checkForAttribute(elemMap, tagName);
		} else {
			elemMap["matchStr"] = elemMap["textContent"];
			checkForAttribute(elemMap, tagName);
		}

		std::vector<std::string> vecElemAttr, elemAttrKeys, elemAttrVals,
				elementFields;
		std::istringstream attributesLine(elemMap["attributes"]);
		std::string l1, l2;
		std::string element = "";

		// todo: be more selective when grabbing commas
		// e.g. [class=foo, alt=hey, there] leave 'hey, there' alone
		while (getline(attributesLine, l1, ',')) {
			l1 = trim(l1);
			vecElemAttr.push_back(l1);
			std::istringstream attrStr(l1);
			// split each name=value pair and store values
			while (getline(attrStr, l2, '=')) {
				elementFields.push_back(l2);
			}
		}

		for (int i = 0; i < elementFields.size(); i++) {
			if (i == 0 || i % 2 == 0) {
				std::string key = elementFields[i];
				elemMap[key] = elementFields[i + 1];
				elemAttrKeys.push_back(key);
				elemAttrVals.push_back(elementFields[i + 1]);
			}
		}

		if (elemMap["attributes"].empty()) {
			element = "<" + tagName + ">" + trim(elemMap["textContent"]) + "</"
					+ tagName + ">";
		} else {

			if (elemAttrKeys.size() != elemAttrVals.size()) {
				perror("Attribute mismatch! A name=value pair is incomplete.");
			}

			for (int j = 0; j < elemAttrKeys.size(); j++) {
				if (elemAttrKeys.size() >= 2 || (j == 0 || j % 2 == 0)) {
					attrStr += elemAttrKeys[j] + "=" + '"' + elemAttrVals[j]
							+ '"' + " ";
				} else {
					attrStr += elemAttrKeys[j] + "=" + '"' + elemAttrVals[j]
							+ '"';
				};
			}

			if (tagName == "img") {
				element = "<" + tagName + " " + trim(attrStr) + " />";
			} else {
				element = "<" + tagName + " " + trim(attrStr) + ">"
						+ trim(elemMap["textContent"]) + "</" + tagName + ">";
			}
		}

		return element;
	}

	void checkForAttribute(
			std::unordered_map<std::string, std::string> &elemMap,
			std::string htmlTag) {
		std::regex element_attr_regex("\\[([^\\[]*)\\]");
		std::regex element_link_href_regex("\\(([^.].*\\w)\\)");
		std::smatch attr_match;

		// handle any element attribute definitions
		// match everything inside square brackets
		if (regex_search(elemMap["matchStr"], attr_match, element_attr_regex)) {
			elemMap["attributes"] = attr_match[1];
		}

		if (elemMap["attributes"].empty()) {
			if (htmlTag != "li") {
				std::regex text_no_attr_regex("([^#].*)");
				if (regex_search(elemMap["textContent"], attr_match,
						text_no_attr_regex)) {
					elemMap["textContent"] = attr_match[0];
				}
			}
		} else {
			std::regex text_content_regex("([\\w].*\\[)");
			// extract text content before the attribute definitions
			if (regex_search(elemMap["textContent"], attr_match,
					text_content_regex)) {
				std::string content = attr_match[0];
				content.pop_back();
				elemMap["textContent"] = content;
			}
		}
	}
};