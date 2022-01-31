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
#include <limits.h>
#include <unordered_map>
#include <map>
#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <queue>
#include <utility>

using std::cout;
using std::endl;

class File {
	char *path;
	char *outputPath;
	std::string raw, html;
	int numLines;
	std::vector<std::string> linesRaw;
	std::vector<std::string> linesHtml;
public:
	File(char *f, char *op) { path = f; outputPath = op; numLines = 0; };
	void read();
	std::vector<File> readFiles(std::vector<std::pair<char*, char*> > files);
	char* getFilePath() { return path; };
	char* getFileOutPath() { return outputPath; };
	std::string getRawText() { return raw; };
	std::string getHTML() { return html; };
	std::vector<std::string> getRawVec() { return linesRaw; };
	std::vector<std::string> getHtmlVec() { return linesHtml; };
	int getNumberOfLines() { return numLines; };
	void writeHTML(std::string html);
};

// function prototypes
void cwd();
std::string& ltrim(std::string&, const char* = " \t\n\r\f\v");
std::string& rtrim(std::string&, const char* = " \t\n\r\f\v");
std::string& trim(std::string&, const char* = " \t\n\r\f\v");
void setHeadingLevel(std::unordered_map<std::string, std::string>&);
std::string createElement(std::unordered_map<std::string, std::string>&, std::string, bool = false);
void handleElemMatch(std::string, int, std::smatch, std::regex, std::unordered_map<std::string, std::string>&, std::map<int, std::string>&, std::string);
void checkForAttributes(std::unordered_map<std::string, std::string>& elemMap, std::string htmlTag);

std::string headingLevel = "";
std::string matchStr = "";

int main() {

	// regular expressions for matching Markdown syntax
	std::regex heading_regex("^#{1,6}\\s\\w+.*");
	std::regex heading_text_regex("\\#([^#]*)\\[");
	std::regex paragraph_regex("^\\w.*");
	std::regex text_attr_regex("([^#].*\\[)");
	std::regex text_no_attr_regex("([^#].*)");
	std::regex single_line_code_regex("`([^`.][^`.]*)`");
	std::regex paragraph_text_regex("(^[\\w].*\\[)");;
	std::regex empty_line_regex("^\\s*$");
	std::regex element_sol_regex("\\<\\w+\\>");
	std::regex element_eol_regex("\\<\\/.*\\>$");
	std::regex element_h_sol_regex("<h\\d>");
	std::regex element_h_eol_regex("<\\/h\\d>$");
	std::regex element_p_sol_regex("^\\<p\\>");
	std::regex element_p_eol_regex("\\<\\/p\\>$");
	std::regex ordered_list_regex("^\\d\\.\\s\\w+");
	std::regex unordered_list_regex("^\\-\\s\\w+");
	std::regex ol_or_ul_regex("^\\<ul\\>|^\\<ol\\>");
	std::regex blockquote_regex("^\\>.*");
	std::regex li_sol_regex("^\\t\\<li\\>");
	std::regex li_wattr_regex("^\\t\\<li\\s\\w+\\=");
	std::regex anchor_element_regex("([^!|^\\w]\\[.+?\\))|^\\[.+?\\)");
	std::regex img_element_regex("(!\\[.+?\\])\\(.+?\\)");

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
	std::ofstream ofs;
	ofs.open(f.getFileOutPath(), std::ios_base::ate);

	std::map<int, std::string> lineMap;
	std::unordered_map<std::string, std::string> elemMap;
	int lineNum = 0;

	// Iterate each line of raw text read from the input file
	// and match lines that need to undergo transformation
	for (const auto &line : f.getRawVec()) {
		lineNum += 1;
		std::smatch match;
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
	std::map<int, std::string>::iterator mapItr;
	for (int i = 1; i < lineMap.size()-1; i++) {
		cout << i << ": " << lineMap[i] << endl;
		bool isPrevLineCompleted = false;
		std::string prevLine = lineMap[i-1];
		std::string currLine = lineMap[i];
		std::string nextLine = lineMap[i+1];
		std::smatch match;
		bool isCurrLineFullPTag = (std::regex_search(currLine, match, element_p_sol_regex) &&
		                            std::regex_search(currLine, match, element_p_eol_regex));

		// handle formatting ordered/unordered lists
		if (std::regex_search(currLine, match, ol_or_ul_regex) && std::regex_search(nextLine, match, ol_or_ul_regex)) {
		    elemMap["textContent"] =  match.suffix();
		}
		// insert closing list elements
		if (std::regex_search(currLine, match, ol_or_ul_regex)) {
		    elemMap["listType"] = static_cast<std::string>(match[0]).insert(1, 1, '/');
		}
		// check if the current line is the last <li>
		// element in a unordered or ordered list and close the tag
		if ((std::regex_search(currLine, match, li_wattr_regex) && nextLine == "") ||
		    (std::regex_search(currLine, match, li_sol_regex) && nextLine == "")) {
		    lineMap[i] += "\n" + elemMap["listType"];
		}
		// single list item ul or ol
		if (std::regex_search(currLine, match, ol_or_ul_regex) && nextLine == "") {
		    lineMap[i] += "\n" + elemMap["listType"];
		}
		// Remove opening and closing <p>,</p> tags for text
		// in between the start and end in a multi-line paragraph block
		if (std::regex_search(nextLine, match, element_p_sol_regex)&& isCurrLineFullPTag && nextLine != "") {
		    lineMap[i] = std::regex_replace(std::regex_replace(currLine, element_p_eol_regex, ""), element_p_sol_regex, "");
		}
		// Remove closing </p> tag on first line in multi-line paragraph block
		if (std::regex_search(currLine, match, element_p_sol_regex) && prevLine == "" && nextLine != "") {
		    lineMap[i] = std::regex_replace(currLine, element_p_eol_regex, "");
		}
		// Remove starting <p> tag on last line in multi-line parapgraph block
		if (std::regex_search(currLine, match, element_p_sol_regex) && prevLine != "" && nextLine == "") {
		    lineMap[i] = std::regex_replace(currLine, element_p_sol_regex, "");
		}

		// if line-1 is a completed line e.g. <h2 class="foo">hi there</h2>
		// its not involved in a multi-line <p> tag
		std::smatch match_helper;

		if (std::regex_search(lineMap[i-1], match_helper, element_sol_regex) &&
		    std::regex_search(lineMap[i-1], match_helper, element_eol_regex)
		) {
		    isPrevLineCompleted = true;
		}

		if (!isPrevLineCompleted && std::regex_search(lineMap[i-1], match_helper, paragraph_regex)) {
		    std::string prevLineText = match_helper[0];
		    std::string currLineText = lineMap[i];
		    elemMap["textContent"] = prevLineText + currLineText;
		}
	}

	cout << "Printing Updated Map" << endl;
	for (std::map<int, std::string>::iterator it = lineMap.begin(); it != lineMap.end(); ++it) {
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

void File::read() {
	std::ifstream f;
	std::string rawText = "", line = "";
	int lineCount = 0;

	f.open(path, std::ios_base::in);

	if (f.is_open()) {
		while (std::getline(f, line)) {
			cout << line << endl; // todo: remove
			rawText += line + "\n";
			lineCount++;
			linesRaw.push_back(line);
		}
		f.close();
	} else {
		cout << "Error opening file at path: " << path << endl;
	}

	raw = rawText;
	numLines = lineCount;
}

// accepts an array of pairs i.e. [inputPath, outputPath]
std::vector<File> File::readFiles(std::vector<std::pair<char*, char*> > files) {
	// todo: ["path1", "path2"] -> std::vector<File> and then write each output file
	std::vector<File> fileVec;
	for (auto fp : files) {
		// create a new file instance then read file by file
		File f(fp.first, fp.second);
//		f.read();

		// todo: do transformations

		// Open the output file and truncate previous files contents if it already exists
		std::ofstream ofs;
		char* outputPath = "";
		ofs.open(f.outputPath, std::ios_base::ate);
	}

	return fileVec;
}

void handleElemMatch(std::string line, int lineNum, std::smatch match, std::regex re, std::unordered_map<std::string, std::string>& elemMap, std::map<int, std::string>& lineMap, std::string htmlTag) {
	// check what kind of element is matched and handle accordingly
	if (std::regex_search(line, match, re)) {
		if (htmlTag == "heading") {
			std::regex heading_level_regex("^[\\#{1-6}]+");
			std::regex heading_text_wattr_regex("\\#([^#]*)\\[");
			std::regex heading_text_wout_attr_re("^\\#{1,6}.*");
			matchStr = match[0];
			if (std::regex_search(line, match, heading_level_regex)) {
				elemMap["headingLevel"] = match[0];
			}
			// search the text between ## and [] e.g. ## Title [class=foo]
			if (std::regex_search(line, match, heading_text_wattr_regex)) {
				elemMap["textContent"] = match[1];
			}
			else if (std::regex_search(line, match, heading_text_wout_attr_re)) {
				// no attributes
				elemMap["textContent"] = match[0];
			}

			// determine heading level based on number of hashes e.g. ## = <h2> and ### = <h3>
			setHeadingLevel(elemMap);
			lineMap[lineNum] = createElement(elemMap, "h" + headingLevel);
			elemMap["attributes"] = "";
		}
		else if (htmlTag == "p") {
			std::regex paragraph_regex("^\\w.*");
			matchStr = match[0];
			elemMap["textContent"] = matchStr;
			line = std::regex_replace(line, paragraph_regex, createElement(elemMap, htmlTag));
			lineMap[lineNum] = line;
			elemMap["attributes"] = "";

		}
		else if (htmlTag == "ol") {
			std::regex re("\\d\\.\\s");
			matchStr = std::regex_replace(line, re, "");
			elemMap["textContent"] = matchStr;
			elemMap["listType"] = htmlTag;
			if (lineMap[lineNum-1] == "") {
				lineMap[lineNum] =  "<ol>\n\t" + createElement(elemMap, "li");
			} else {
				lineMap[lineNum] = "\t" + createElement(elemMap, "li");
			}
			elemMap["attributes"] = "";
		}
		else if (htmlTag == "ul") {
			std::regex re("\\-\\s");
			matchStr = std::regex_replace(line, re, "");
			elemMap["textContent"] = matchStr;
			elemMap["listType"] = htmlTag;
			if (lineMap[lineNum-1] == "") {
				lineMap[lineNum] = "<ul>\n\t" + createElement(elemMap, "li");
			} else {
				lineMap[lineNum] = "\t" + createElement(elemMap, "li");
			}
			elemMap["attributes"] = "";
		}
		else if (htmlTag == "a") {
			std::regex element_attr_regex("\\[([^\\[]*)\\]");
			std::regex element_link_href_regex("\\((.+?)\\)");
			std::regex anchor_element_regex("\\[.+?\\w\\].+?.\\)");
			std::string linkName = "";
			std::string linkHref = "";
			std::string::const_iterator searchStart(line.cbegin());
			std::smatch matched;
			std::vector<std::string> links;
			std::regex bracket_or_paren_reg("[\\[|\\]|\\(|\\)]");

			std::queue<std::string> q;

			int i = 0;
			// iterate over the line while regex_search'ing (account for a line having multiple links)
			while (std::regex_search(searchStart, line.cend(), matched, anchor_element_regex)) {
				cout << (searchStart == line.cbegin() ? "" : " ") << "MATcH: " << matched[0] << endl;
				searchStart = matched.suffix().first;
				std::string anchor = matched[0];
				q.push(anchor);

				while (!q.empty()) {
					std::string a = q.front();
					q.pop();

					if (std::regex_search(anchor, match, element_attr_regex)) {
						linkName = match[1];
					}
					if (std::regex_search(anchor, match, element_link_href_regex)) {
						linkHref = match[1];
					}
					matchStr = linkName + " [href=" + linkHref + "]";
					elemMap["textContent"] = matchStr;

					// add constructed <a> elements to list of links to use in replacements
					links.push_back(createElement(elemMap, htmlTag));
				}

				for (auto l : links) {
					cout << "link: " << l << endl;

				}
			}
			//
			std::string tempLine = line;
			// todo: replace hard coded <p>
			lineMap[lineNum] = "<p>" + std::regex_replace(line, anchor_element_regex, createElement(elemMap, htmlTag)) + "</p>";
			elemMap["attributes"] = "";
		}
		else if (htmlTag == "img") {
			std::regex element_attr_regex("\\[([^\\[]*)\\]");
			std::regex element_link_href_regex("\\(([^.].*\\w)\\)");
			std::string altText = "";
			std::string imgSrc = "";
			if (std::regex_search(line, match, element_attr_regex)) {
				altText = match[1];
			}
			if (std::regex_search(line, match, element_link_href_regex)) {
				imgSrc = match[1];
			}
			matchStr = "[src=" + imgSrc + ", alt=" + altText + "]";
			elemMap["textContent"] = "";
			lineMap[lineNum] = createElement(elemMap, htmlTag);
			elemMap["attributes"] = "";
		}
		else if (htmlTag == "blockquote") {
			std::regex re("\\>\\s");
			matchStr = std::regex_replace(static_cast<std::string>(match[0]), re, "");
			elemMap["textContent"] = matchStr;
			lineMap[lineNum] = createElement(elemMap, htmlTag);
			elemMap["attributes"] = "";
		}
		else if (htmlTag == "code") {
			matchStr = match[1];
			elemMap["textContent"] = matchStr;
			lineMap[lineNum] = createElement(elemMap, htmlTag);
		}
		else if (htmlTag == "") {
			matchStr = "";
		}
	}
}

void checkForAttributes(std::unordered_map<std::string, std::string>& elemMap, std::string htmlTag) {
	std::regex element_attr_regex("\\[([^\\[]*)\\]");
	std::regex element_link_href_regex("\\(([^.].*\\w)\\)");
	std::smatch attr_match;

    // handle any element attribute definitions
    // match everything inside square brackets
    if (std::regex_search(matchStr, attr_match, element_attr_regex)) {
        elemMap["attributes"] = attr_match[1];
    }

    if (elemMap["attributes"].empty()) {
        if (htmlTag != "li") {
        	std::regex text_no_attr_regex("([^#].*)");
            if (std::regex_search(elemMap["textContent"], attr_match, text_no_attr_regex)) {
                elemMap["textContent"] = attr_match[0];
            }
        }
    } else {
    	std::regex paragraph_text_regex("(^[\\w].*\\[)");
        if (std::regex_search(elemMap["textContent"], attr_match, paragraph_text_regex)) {
            std::string s = attr_match[0];
            s.pop_back();
            elemMap["textContent"] = s;
        }
    }
}

std::string createElement(std::unordered_map<std::string, std::string>& elemMap, std::string htmlTag, bool isPrevLineParagraphText) {

	// check for attributes defined for the
	// current element to be processed
	checkForAttributes(elemMap, htmlTag);

    std::vector<std::string> vecElemAttr;
    std::vector<std::string> elemAttrKeys;
    std::vector<std::string> elemAttrVals;
    std::istringstream ss(elemMap["attributes"]);
    std::vector<std::string> elementFields;
    std::string sk, sl, ln;
	std::string element = "";

    // iterate the line and collect those attributes
    // O(n^2) time
	// todo: be more selective when grabbing commas e.g. [class=fuzz, alt=hey, there] leave 'hey, there' alone
    while (std::getline(ss, sl, ',')) {
    	sl = trim(sl);
        vecElemAttr.push_back(sl);
        std::istringstream sm(sl);
        // split each name=value pair and store values
        while (std::getline(sm, sk, '=')) {
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

    // if element doesn't have any attributes construct element as is
	if (elemMap["attributes"].empty()) {
        element = "<" + htmlTag + ">" + trim(elemMap["textContent"]) + "</" + htmlTag + ">";
    }
	// otherwise construct element with defined attributes
	else {
        if (elemAttrKeys.size() != elemAttrVals.size()) {
        	perror("Attribute mismatch! A name=value pair is incomplete.");
        };

        std::string attrStr = "";

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

void setHeadingLevel(std::unordered_map<std::string, std::string>& elemMap) {
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
}

// trim from left
std::string& ltrim(std::string& s, const char* t)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
std::string& rtrim(std::string& s, const char* t)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
std::string& trim(std::string& s, const char* t)
{
    return ltrim(rtrim(s, t), t);
}

// get current working directory printed to std out
void cwd() {
	char *getcwd(char *buf, size_t size);
	char cwd[256]; // 256 character limit max
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("Current working dir: %s\n", cwd);
	} else {
		perror("getcwd() error");
	}
}