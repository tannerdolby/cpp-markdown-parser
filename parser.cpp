//============================================================================
// Name        : markdown-parser.cpp
// Author      : Tanner Dolby
// Version     : 0.0.1
// Copyright   : MIT License
// Description : A utility for parsing a Markdown file to HTML.
//============================================================================

#include <iostream> // std::cout and std::endl
#include <fstream> // std::ifstream and std::ofstream
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <unordered_map> // std::unordered_map
#include <map> // std::map
#include <string> // std::string
#include <regex> // std::regex and std::regex_search
#include <vector> // std::vector
#include <sstream> // std::istringstream

using std::cout;
using std::endl;

const char *trim_reg_str = " \t\n\r\f\v";

// Class representing input Markdown files
class InputFile {
	char *path;
	std::string raw, html;
	int numberOfLines;
	std::vector<std::string> fileRawVec;
	std::vector<std::string> fileHtmlVec;
	public:
		InputFile(char *f) { path = f; numberOfLines = 0; };
		void read();
		std::string getFilePath() { return path; };
		std::string getRawText() { return raw; };
		std::string getHTML() { return html; };
		std::vector<std::string> getRawVector() { return fileRawVec; };
		std::vector<std::string> getHtmlVector() { return fileHtmlVec; };
		int getNumberOfLines() { return numberOfLines; };
		void writeHTML(std::string line) { fileHtmlVec.push_back(line); };
};

struct FileData {
	std::string raw;
	int numberOfLines;
	std::vector<std::string> fvec;
	FileData* next;
	FileData() : raw(""), numberOfLines(0), next(nullptr) {};
};

class FileReader {
	public:
		FileData readFile(char *f);
		void readFiles(std::vector<char*> vec);
};

FileData FileReader::readFile(char *filePath) {
	FileData fileData;
	std::ifstream file;
	std::string rawText = "";
	std::string line;
	int numberOfLines = 0;

	// Open file at given file path for reading
	// using ios_base::in as the stream open mode type
	file.open(filePath, std::ios_base::in);

	if (file.is_open()) {
		// begin reading file line by line
		while (std::getline(file, line)) {
			cout << line << endl;
			rawText += line + "\n";
			numberOfLines++;
			fileData.fvec.push_back(line);
		}
		// close the input file stream
		file.close();
	} else {
		cout << "Error opening file at path: " << filePath << endl;
	}

	fileData.raw = rawText;
	fileData.numberOfLines = numberOfLines;

	return fileData;
}

void FileReader::readFiles(std::vector<char*> files) {
	// todo: ["path1", "path2"] -> std::vector<FileData> and then write each output file
}

void InputFile::read() {
	FileReader fr;
	FileData fd = fr.readFile(path);
	raw = fd.raw;
	fileRawVec = fd.fvec;
	numberOfLines = fd.numberOfLines;
};

// function prototypes
void cwd();
std::string& ltrim(std::string&, const char* = trim_reg_str);
std::string& rtrim(std::string&, const char* = trim_reg_str);
std::string& trim(std::string&, const char* = trim_reg_str);
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
	std::regex anchor_element_regex("^\\[.*\\)|[^!]\\[.*\\)");

	// todo: accept file path from std::in or from command line args
	char path[] = "./src/test-file.md";
	InputFile f(path);
	// Read the file and store contents in the
	// class fields of InputFile instance
	f.read();
	std::cout << "Raw file content: " << f.getRawText() << std::endl;
	std::cout << "Number of Lines: " << f.getNumberOfLines() << std::endl;

	// Get current working directory
	cwd();

	// Open the output file and truncate previous files contents if it already exists
	std::ofstream ofs;
	ofs.open("./src/output.html", std::ios_base::ate);


	// Regular expressions for converting Markdown to HTML
	std::cout << "Starting regex " << std::endl;

	// use lineNum to keep track of replaced lines to ensure
	// proper order is preserved in HTML output
	// use a std::map which is ordered based on key
	std::map<int, std::string> lineMap;
	std::unordered_map<std::string, std::string> elemMap;
	int lineNum = 0;

	std::regex img_element_regex("(!\\[.*\\].+\\))");

	// Iterate each line of raw text read from the input file
	// and match lines that need to undergo transformation
	for (const auto &line : f.getRawVector()) {
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

	}
	cout << "Printing Ordered Map" << endl;
	std::map<int, std::string>::iterator mapItr;
	for (int i = 1; i <= lineMap.size(); i++) {
		cout << i << ": " << lineMap[i] << endl;
		bool isPrevLineCompleted = false;

		if (i > 1) {
			if (i+1 <= lineMap.size()) {

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

	}

	cout << "Printing Updated Map" << endl;
	for (std::map<int, std::string>::iterator it = lineMap.begin(); it != lineMap.end(); ++it) {
		cout << it -> first;
		cout << ": " << it -> second << endl;
		// write to output file stream e.g. resultant HTML file
		ofs << it -> second << endl;
	}

	cout << "HTML: " << endl;
	for (const auto sv : f.getHtmlVector()) {
		cout << "L: " << sv << endl;
	}

	return 0;
}

void handleElemMatch(std::string line, int lineNum, std::smatch match, std::regex re, std::unordered_map<std::string, std::string>& elemMap, std::map<int, std::string>& lineMap, std::string htmlTag) {
	cout << "LINED UP: " << line << endl;
	if (std::regex_search(line, match, re)) {
		// check what kind of element is matched and handle accordingly
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

			// determine heading level based on number of hashtags e.g. ## = <h2> and ### = <h3>
			setHeadingLevel(elemMap);
			lineMap[lineNum] = createElement(elemMap, "h" + headingLevel);
			elemMap["attributes"] = "";
		}
		else if (htmlTag == "p") {
			matchStr = match[0];
			elemMap["textContent"] = matchStr;
			lineMap[lineNum] = createElement(elemMap, htmlTag);
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
			std::regex element_link_href_regex("\\(([^.].*\\w)\\)");
			std::string linkName = "";
			std::string linkHref = "";
			if (std::regex_search(line, match, element_attr_regex)) {
				linkName = match[1];
			}
			if (std::regex_search(line, match, element_link_href_regex)) {
				linkHref = match[1];
			}
			matchStr = linkName + " [href=" + linkHref + "]";
			elemMap["textContent"] = matchStr;
			lineMap[lineNum] = createElement(elemMap, htmlTag);
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
    if (std::regex_search(matchStr, attr_match, element_attr_regex)) {
        // handle any element attribute definitions
        // match everything inside square brackets
        elemMap["attributes"] = attr_match[1];
    }
    if (elemMap["attributes"].empty()) {
        if (htmlTag != "li") {
        	std::regex text_no_attr_regex("([^#].*)");
            if (std::regex_search(elemMap["textContent"], attr_match, text_no_attr_regex)) {
                elemMap["textContent"] = attr_match[0];
            }
        }
        // todo: look for <a> or <img> within <p> tags
    } else {
    	std::regex paragraph_text_regex("(^[\\w].*\\[)");
        if (std::regex_search(elemMap["textContent"], attr_match, paragraph_text_regex)) {
            std::string s = attr_match[0];
            s.pop_back();
            elemMap["textContent"] = s;
        }
        // todo: look for <a> or <img> within <p> tags
    }
}

std::string createElement(std::unordered_map<std::string, std::string>& elemMap, std::string htmlTag, bool isPrevLineParagraphText) {
	checkForAttributes(elemMap, htmlTag);
    // Use optional delimeter parameter of getline()
    // to mimic a String.split("=") call for handling attrr definitions
    // if there is multiple attribute definitions e.g.
    // ## some title [class=foo,id=bar] first split
    // by comma then handle specific attrs
    std::vector<std::string> vecElemAttr;
    std::istringstream ss(elemMap["attributes"]);
    std::vector<std::string> elementFields;
    std::string sk, sl, ln;
    while (std::getline(ss, sl, ',')) {
        vecElemAttr.push_back(sl);
        std::istringstream sm(sl);
        // split each name=value pair and store values
        while (std::getline(sm, sk, '=')) {
            elementFields.push_back(sk);
        }
    }

    std::vector<std::string> elemAttrKeys;
    std::vector<std::string> elemAttrVals;
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
	std::string element = "";
	if (elemMap["attributes"].empty()) {
        element = "<" + htmlTag + ">" + trim(elemMap["textContent"]) + "</" + htmlTag + ">";
    } else {
        if (elemAttrKeys.size() != elemAttrVals.size()) {
        	perror("Attribute mismatch! A name=value pair is incomplete.");
        };
        std::string attrStr = "";
        for (int j = 0; j < elemAttrKeys.size(); j++) {
        	cout << "elem keys size: " << elemAttrKeys.size() << endl;
            if (elemAttrKeys.size() >= 2 || (j == 0 || j % 2 == 0)) {
                // add space at the end
                attrStr += elemAttrKeys[j] + "=" + '"' + elemAttrVals[j] + '"' + " ";
            } else {
            	attrStr += elemAttrKeys[j] + "=" + '"' + elemAttrVals[j] + '"';
            };
        }

        // Construct tag with attributes
        if (htmlTag == "img") {
        	element = "<" + htmlTag + " " + attrStr + " />";
        } else if (htmlTag == "p") {
        	// todo: look for <a> or <img> within <p> tags
        	std::regex anchor_element_regex("^\\[.*\\)|[^!]\\[.*\\)");
        	std::smatch match;
        	if (std::regex_search(elemMap["textContent"], match, anchor_element_regex)) {
        		elemMap["textContent"] = std::regex_replace(static_cast<std::string>(match[0]), anchor_element_regex, "<a>" + static_cast<std::string>(match[0]) + "</a>");
        	}
        	cout << "TEXT: " << elemMap["textContent"] << endl;
        	element = "<" + htmlTag + " " + attrStr + ">" + trim(elemMap["textContent"]) + "</" + htmlTag + ">";
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
