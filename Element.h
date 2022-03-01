#include <iostream>
#include <unordered_map>
#include <regex>
#include "Helpers.h"

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

	// Create HTML element and return its HTML tag as string
	std::string create() {

		if (tagName == "img") {
			// no text content
			checkForAttribute(elemMap, tagName);
		} else {
			elemMap["matchStr"] = elemMap["textContent"];
			checkForAttribute(elemMap, tagName);
		}

	    std::vector<std::string> vecElemAttr, elemAttrKeys, elemAttrVals, elementFields;
	    std::istringstream ss(elemMap["attributes"]);
	    std::string sk, sl, ln;
		std::string element = "";

	    // iterate the attribute string and collect values
		// todo: be more selective when grabbing commas e.g. [class=fuzz, alt=hey, there] leave 'hey, there' alone
	    while (getline(ss, sl, ',')) {
	    	sl = trim(sl);
	        vecElemAttr.push_back(sl);
	        std::istringstream sm(sl);
	        // split each name=value pair and store values
	        while (getline(sm, sk, '=')) {
	            elementFields.push_back(sk);
	        }
	    }

	    // O(n) time iterating the element fields vector
	    for (int i = 0; i < elementFields.size(); i++) {
	        if (i == 0 || i % 2 == 0) {
	            // create key/value pair with attr name and value in unordered map
	            std::string key = elementFields[i];
	            // add pair to map
	            elemMap[key] = elementFields[i+1];
	            elemAttrKeys.push_back(key);
	            elemAttrVals.push_back(elementFields[i+1]);
	        }
	    }

	    // CONSTRUCT ELEMENT
	    // if element doesn't have any attributes construct element as is
		if (elemMap["attributes"].empty()) {
	        element = "<" + tagName + ">" + trim(elemMap["textContent"]) + "</" + tagName + ">";
	    }
		// otherwise construct element with defined attributes
		else {
	        if (elemAttrKeys.size() != elemAttrVals.size()) {
	        	perror("Attribute mismatch! A name=value pair is incomplete.");
	        };

	        for (int j = 0; j < elemAttrKeys.size(); j++) {
	            if (elemAttrKeys.size() >= 2 || (j == 0 || j % 2 == 0)) {
	                // add space at the end
	                attrStr += elemAttrKeys[j] + "=" + '"' + elemAttrVals[j] + '"' + " ";
	            } else {
	            	attrStr += elemAttrKeys[j] + "=" + '"' + elemAttrVals[j] + '"';
	            };
	        }

	        attrStr = trim(attrStr);

	        std::unordered_set<std::string> hLevels;
	        hLevels.insert("h1");
	        hLevels.insert("h2");
	        hLevels.insert("h3");
	        hLevels.insert("h4");
	        hLevels.insert("h5");
	        hLevels.insert("h6");

	        if (tagName == "img") {
	        	element = "<" + tagName + " " + attrStr + " />";
	        }
	        else if (hLevels.find(tagName) != hLevels.end()) {
	        	element = "<" + tagName + " " + attrStr + ">" + "hi there" + "</" + tagName + ">";
	        }
	        else {
	        	element = "<" + tagName + " " + attrStr + ">" + trim(elemMap["textContent"]) + "</" + tagName + ">";
	        }
	    }

		return element;
	}

	void checkForAttribute(std::unordered_map<std::string, std::string>& elemMap, std::string htmlTag) {
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
	            if (regex_search(elemMap["textContent"], attr_match, text_no_attr_regex)) {
	                elemMap["textContent"] = attr_match[0];
	            }
	        }
	    } else {
	    	std::regex paragraph_text_regex("(^[\\w].*\\[)");
	        if (regex_search(elemMap["textContent"], attr_match, paragraph_text_regex)) {
	            std::string s = attr_match[0];
	            s.pop_back();
	            elemMap["textContent"] = s;
	        }
	    }
	}
};