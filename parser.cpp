//============================================================================
// Name        : markdown-parser.cpp
// Author      : Tanner Dolby
// Version     : 0.0.1
// Copyright   : MIT License
// Description : A utility for parsing Markdown files to HTML.
//============================================================================

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <climits>
#include <unordered_map>
#include <map>
#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <queue>
#include <utility>
#include <unordered_set>

#include "Element.h"
#include "File.h"

using namespace std;

// function prototypes
void cwd();
string getHeadingLevel(unordered_map<string, string>&);
string createElement(unordered_map<string,string>&, string);
void handleElemMatch(string, int, smatch, regex, unordered_map<string, string>&, map<int, string>&, string);
void checkForAttributes(unordered_map<string, string>& elemMap, string htmlTag);

// todo: create a DOM tree representation of created elements

string matchStr = "";

int main() {

	// regular expressions for matching Markdown syntax
	regex heading_regex("^#{1,6}\\s\\w+.*");
	regex heading_text_regex("\\#([^#]*)\\[");
	regex paragraph_regex("^\\w.*");
	regex text_attr_regex("([^#].*\\[)");
	regex text_no_attr_regex("([^#].*)");
	regex single_line_code_regex("`([^`.][^`.]*)`");
	regex paragraph_text_regex("(^[\\w].*\\[)");;
	regex empty_line_regex("^\\s*$");
	regex element_sol_regex("\\<\\w+\\>");
	regex element_eol_regex("\\<\\/.*\\>$");
	regex element_h_sol_regex("<h\\d>");
	regex element_h_eol_regex("<\\/h\\d>$");
	regex element_p_sol_regex("^\\<p\\>");
	regex element_p_eol_regex("\\<\\/p\\>$");
	regex ordered_list_regex("^\\d\\.\\s\\w+");
	regex unordered_list_regex("^\\-\\s\\w+");
	regex ol_or_ul_regex("^\\<ul\\>|^\\<ol\\>");
	regex blockquote_regex("^\\>.*");
	regex li_sol_regex("^\\t\\<li\\>");
	regex li_wattr_regex("^\\t\\<li\\s\\w+\\=");
	regex anchor_element_regex("([^!|^\\w]\\[.+?\\))|^\\[.+?\\)");
	regex img_element_regex("(!\\[.+?\\])\\(.+?\\)");

	char path[] = "./src/test-file.md";
	char outPath[] = "./src/test-file.html";

	// construct a new input file instance
	File f(path, outPath);

	// Read the file and store contents in the
	// class fields of File instance
	f.read();
	cout << "Raw file content: " << f.getRawText() << endl;
	cout << "Number of Lines: " << f.getNumberOfLines() << endl;

	// Get current working directory
	cwd();

	// Open the output file for appending and truncate previous files contents if they already exist
	ofstream ofs;
	ofs.open(f.getFileOutPath(), ios_base::ate);

	map<int, string> lineMap;
	unordered_map<string, string> elemMap;
	int lineNum = 0;

	// Iterate each line of raw text read from the input file
	// and match lines that need to undergo transformation
	for (const auto &line : f.getRawVec()) {
		lineNum += 1;
		smatch match;
		handleElemMatch(line, lineNum, match, empty_line_regex, elemMap, lineMap, "");
		handleElemMatch(line, lineNum, match, heading_regex, elemMap, lineMap, "heading");
		handleElemMatch(line, lineNum, match, paragraph_regex, elemMap, lineMap, "p");
		handleElemMatch(line, lineNum, match, anchor_element_regex, elemMap, lineMap, "a");
		handleElemMatch(line, lineNum, match, blockquote_regex, elemMap, lineMap, "blockquote");
		handleElemMatch(line, lineNum, match, single_line_code_regex, elemMap, lineMap, "code");
		handleElemMatch(line, lineNum, match, ordered_list_regex, elemMap, lineMap, "ol");
		handleElemMatch(line, lineNum, match, unordered_list_regex, elemMap, lineMap, "ul");
		handleElemMatch(line, lineNum, match, img_element_regex, elemMap, lineMap, "img");
		// todo: multi-line code snippets (<pre> + nested <code>)
		// todo: reference links [1]: https://google.com or [google]: https://google.com
	}

	cout << "Printing Ordered Map" << endl;
	map<int, string>::iterator mapItr;
	for (int i = 1; i < lineMap.size()-1; i++) {
		cout << i << ": " << lineMap[i] << endl;
		bool isPrevLineCompleted = false;
		string prevLine = lineMap[i-1];
		string currLine = lineMap[i];
		string nextLine = lineMap[i+1];
		smatch match;
		bool isCurrLineFullPTag = (regex_search(currLine, match, element_p_sol_regex) &&
		                            regex_search(currLine, match, element_p_eol_regex));

		// handle formatting ordered/unordered lists
		if (regex_search(currLine, match, ol_or_ul_regex) && regex_search(nextLine, match, ol_or_ul_regex)) {
		    elemMap["textContent"] =  match.suffix();
		}
		// insert closing list elements
		if (regex_search(currLine, match, ol_or_ul_regex)) {
		    elemMap["listType"] = static_cast<string>(match[0]).insert(1, 1, '/');
		}
		// check if the current line is the last <li>
		// element in a unordered or ordered list and close the tag
		if ((regex_search(currLine, match, li_wattr_regex) && nextLine == "") ||
		    (regex_search(currLine, match, li_sol_regex) && nextLine == "")) {
		    lineMap[i] += "\n" + elemMap["listType"];
		}
		// single list item ul or ol
		if (regex_search(currLine, match, ol_or_ul_regex) && nextLine == "") {
		    lineMap[i] += "\n" + elemMap["listType"];
		}
		// Remove opening and closing <p>,</p> tags for text
		// in between the start and end in a multi-line paragraph block
		if (regex_search(nextLine, match, element_p_sol_regex)&& isCurrLineFullPTag && nextLine != "") {
		    lineMap[i] = regex_replace(regex_replace(currLine, element_p_eol_regex, ""), element_p_sol_regex, "");
		}
		// Remove closing </p> tag on first line in multi-line paragraph block
		if (regex_search(currLine, match, element_p_sol_regex) && prevLine == "" && nextLine != "") {
		    lineMap[i] = regex_replace(currLine, element_p_eol_regex, "");
		}
		// Remove starting <p> tag on last line in multi-line parapgraph block
		if (regex_search(currLine, match, element_p_sol_regex) && prevLine != "" && nextLine == "") {
		    lineMap[i] = regex_replace(currLine, element_p_sol_regex, "");
		}

		// if line-1 is a completed line e.g. <h2 class="foo">hi there</h2>
		// its not involved in a multi-line <p> tag
		smatch match_helper;

		if (regex_search(lineMap[i-1], match_helper, element_sol_regex) &&
		    regex_search(lineMap[i-1], match_helper, element_eol_regex)
		) {
		    isPrevLineCompleted = true;
		}

		if (!isPrevLineCompleted && regex_search(lineMap[i-1], match_helper, paragraph_regex)) {
		    string prevLineText = match_helper[0];
		    string currLineText = lineMap[i];
		    elemMap["textContent"] = prevLineText + currLineText;
		}
	}

	cout << "Printing Updated Map" << endl;
	for (auto it = lineMap.begin(); it != lineMap.end(); ++it) {
		cout << it -> first;
		cout << ": " << it -> second << endl;
		// write to output file stream e.g. resultant HTML file
		ofs << it -> second << endl;
	}

	cout << "HTML: " << endl;
	for (const auto sv : f.getHtmlVec()) {
		cout << "L: " << sv << endl;
	}

	return 0;
}

void handleElemMatch(string line, int lineNum, std::smatch match, regex re, unordered_map<string, string>& elemMap, map<int, string>& lineMap, string htmlTag) {
	// check what kind of element is matched and handle accordingly
	if (regex_search(line, match, re)) {
		if (htmlTag == "heading") {
			regex heading_level_regex("^[\\#{1-6}]+");
			regex heading_text_wattr_regex("\\#([^#]*)\\[");
			regex heading_text_wout_attr_re("^\\#{1,6}.*");
			elemMap["matchStr"] = match[0];

			if (regex_search(line, match, heading_level_regex)) {
				elemMap["headingLevel"] = match[0];
			}
			// has attributes
			// search the text between ## and [] e.g. ## Title [class=foo]
			if (regex_search(line, match, heading_text_wattr_regex)) {
				elemMap["textContent"] = match[0];
			}

			// no attributes
			if (regex_search(line, match, heading_text_wout_attr_re)) {
				// no attributes
				elemMap["textContent"] = match[0];
			}

			Element heading(elemMap, getHeadingLevel(elemMap));
			lineMap[lineNum] = heading.create();
		}
		else if (htmlTag == "p") {
			regex paragraph_regex("^\\w.*");
			elemMap["textContent"] = match[0];
			Element paragraph(elemMap, "p");
			line = regex_replace(line, paragraph_regex, paragraph.create());
			lineMap[lineNum] = line;
		}
		else if (htmlTag == "ol") {
			regex re("\\d\\.\\s");
			elemMap["textContent"] = regex_replace(line, re, "");
			elemMap["listType"] = htmlTag;
			if (lineMap[lineNum-1] == "") {
				Element li(elemMap, "li");
				lineMap[lineNum] =  "<ol>\n\t" + li.create();
			} else {
				Element li(elemMap, "li");
				lineMap[lineNum] = "\t" + li.create();
			}
		}
		else if (htmlTag == "ul") {
			regex re("\\-\\s");
			matchStr = regex_replace(line, re, "");
			elemMap["textContent"] = matchStr;
			elemMap["listType"] = htmlTag;
			if (lineMap[lineNum-1] == "") {
				Element li(elemMap, "li");
				lineMap[lineNum] = "<ul>\n\t" + li.create();
			} else {
				Element li(elemMap, "li");
				lineMap[lineNum] = "\t" + li.create();
			}
		}
		else if (htmlTag == "a") {
			regex element_attr_regex("\\[([^\\[]*)\\]");
			regex element_link_href_regex("\\((.+?)\\)");
			regex anchor_element_regex("\\[.+?\\w\\].+?.\\)");
			string linkName = "";
			string linkHref = "";
			string::const_iterator searchStart(line.cbegin());
			smatch matched;
			vector<string> links;
			regex bracket_or_paren_reg("[\\[|\\]|\\(|\\)]");

			queue<string> q;

			// iterate over the line while regex_search'ing (account for a line having multiple links)
			while (regex_search(searchStart, line.cend(), matched, anchor_element_regex)) {
				cout << (searchStart == line.cbegin() ? "" : " ") << "MATcH: " << matched[0] << endl;
				searchStart = matched.suffix().first;
				string anchor = matched[0];
				q.push(anchor);

				while (!q.empty()) {
					string a = q.front();
					q.pop();

					if (regex_search(anchor, match, element_attr_regex)) {
						linkName = match[1];
					}

					if (regex_search(anchor, match, element_link_href_regex)) {
						linkHref = match[1];
					}

					elemMap["textContent"] = linkName + " [href=" + linkHref + "]";;

					// add constructed <a> elements to list of links to use in replacements
					links.push_back(createElement(elemMap, htmlTag));
				}

				for (auto l : links) {
					cout << "link: " << l << endl;

				}
			}
			lineMap[lineNum] = "<p>" + regex_replace(line, anchor_element_regex, createElement(elemMap, htmlTag)) + "</p>";
		}
		else if (htmlTag == "img") {
			regex element_attr_regex("\\[([^\\[]*)\\]");
			regex element_link_href_regex("\\(([^.].*\\w)\\)");
			string altText = "", imgSrc = "";
			if (regex_search(line, match, element_attr_regex)) {
				altText = match[1];
			}
			if (regex_search(line, match, element_link_href_regex)) {
				imgSrc = match[1];
			}
			elemMap["matchStr"] = "[src=" + imgSrc + ", alt=" + altText + "]";
			elemMap["textContent"] = "";
			Element img(elemMap, htmlTag);
			lineMap[lineNum] = createElement(elemMap, htmlTag);
			elemMap["attributes"] = "";
		}
		else if (htmlTag == "blockquote") {
			regex re("\\>\\s");
			elemMap["textContent"] = regex_replace(static_cast<string>(match[0]), re, "");
			lineMap[lineNum] = createElement(elemMap, htmlTag);
			elemMap["attributes"] = "";
		}
		else if (htmlTag == "code") {
			matchStr = match[1];
			elemMap["textContent"] = matchStr;
			string code =
			lineMap[lineNum] = createElement(elemMap, htmlTag);
		}
		else if (htmlTag == "") {
			matchStr = "";
		}
	}
}

void checkForAttributes(unordered_map<string, string>& elemMap, string htmlTag) {
	regex element_attr_regex("\\[([^\\[]*)\\]");
	regex element_link_href_regex("\\(([^.].*\\w)\\)");
	smatch attr_match;

    // handle any element attribute definitions
    // match everything inside square brackets
    if (regex_search(matchStr, attr_match, element_attr_regex)) {
        elemMap["attributes"] = attr_match[1];
    }

    if (elemMap["attributes"].empty()) {
        if (htmlTag != "li") {
        	regex text_no_attr_regex("([^#].*)");
            if (regex_search(elemMap["textContent"], attr_match, text_no_attr_regex)) {
                elemMap["textContent"] = attr_match[0];
            }
        }
    } else {
    	regex paragraph_text_regex("(^[\\w].*\\[)");
        if (regex_search(elemMap["textContent"], attr_match, paragraph_text_regex)) {
            string s = attr_match[0];
            s.pop_back();
            elemMap["textContent"] = s;
        }
    }
}

string createElement(unordered_map<string,string>& elemMap, string htmlTag) {

		// check for attributes defined for the
		// current element to be processed
		checkForAttributes(elemMap, htmlTag);

	    vector<string> vecElemAttr, elemAttrKeys, elemAttrVals, elementFields;
	    istringstream ss(elemMap["attributes"]);
	    string sk, sl, ln;
		string element = "";

	    // iterate the line and collect those attributes
	    // O(n^2) time
		// todo: be more selective when grabbing commas e.g. [class=fuzz, alt=hey, there] leave 'hey, there' alone
	    while (getline(ss, sl, ',')) {
	    	sl = trim(sl);
	        vecElemAttr.push_back(sl);
	        istringstream sm(sl);
	        // split each name=value pair and store values
	        while (getline(sm, sk, '=')) {
	            elementFields.push_back(sk);
	        }
	    }

	    // O(n) time iterating the element fields vector
	    for (int i = 0; i < elementFields.size(); i++) {
	        if (i == 0 || i % 2 == 0) {
	            // create key/value pair with attr name and value in unordered map
	            string key = elementFields[i];
	            // add pair to map
	            elemMap[key] = elementFields[i+1];
	            elemAttrKeys.push_back(key);
	            elemAttrVals.push_back(elementFields[i+1]);
	        }
	    }


	    // CONSTRUCT ELEMENT
	    // if element doesn't have any attributes construct element as is
		if (elemMap["attributes"].empty()) {
	        element = "<" + htmlTag + ">" + trim(elemMap["textContent"]) + "</" + htmlTag + ">";
	    }
		// otherwise construct element with defined attributes
		else {
	        if (elemAttrKeys.size() != elemAttrVals.size()) {
	        	perror("Attribute mismatch! A name=value pair is incomplete.");
	        };

	        string attrStr = "";

	        for (int j = 0; j < elemAttrKeys.size(); j++) {
	            if (elemAttrKeys.size() >= 2 || (j == 0 || j % 2 == 0)) {
	                // add space at the end
	                attrStr += elemAttrKeys[j] + "=" + '"' + elemAttrVals[j] + '"' + " ";
	            } else {
	            	attrStr += elemAttrKeys[j] + "=" + '"' + elemAttrVals[j] + '"';
	            };
	        }

	        attrStr = trim(attrStr);

	        if (htmlTag == "img") {
	        	element = "<" + htmlTag + " " + attrStr + " />";
	        }
	        else {
	        	element = "<" + htmlTag + " " + attrStr + ">" + trim(elemMap["textContent"]) + "</" + htmlTag + ">";
	        }
	    }

		return element;
	}



string getHeadingLevel(unordered_map<string, string> &elemMap) {
	string headingLevel = "";
	switch (elemMap["headingLevel"].size()) {
		case 1:
			headingLevel = "1";
			break;
	    case 2:
	        headingLevel = "2";
	        break;
	    case 3:
	        headingLevel = "3";
	        break;
	    case 4:
	        headingLevel = "4";
	        break;
	    case 5:
	        headingLevel = "5";
	        break;
	    case 6:
	        headingLevel = "6";
	        break;
	}
	return "h" + headingLevel;
}