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

// selects class=foo from some text [class=foo]
//std::regex element_attr_regex("\\[([^.]*)\\]");
std::regex element_attr_regex("\\[([^\\[]*)\\]");
std::smatch element_attr_match;

std::regex element_link_href_regex("\\(([^.].*\\w)\\)");
std::smatch element_link_href_match;

std::regex text_attr_regex("([^#].*\\[)");
std::regex text_no_attr_regex("([^#].*)");
std::smatch t_match;

std::regex single_line_code_regex("`([^`.][^`.]*)`");
std::smatch sl_code_match;

std::string headingLevel = "";
std::string matchStr = "";

std::regex paragraph_text_regex("(^[\\w].*\\[)");
std::smatch paragraph_text_match;

std::regex empty_line_regex("^\\s*$");
std::smatch empty_line_match;

std::regex element_sol_regex("\\<\\w+\\>");
std::smatch element_sol_match;

std::regex element_eol_regex("\\<\\/.*\\>$");
std::smatch element_eol_match;

std::regex element_h_sol_regex("<h\\d>");
std::smatch element_h_sol_match;

std::regex element_h_eol_regex("<\\/h\\d>$");
std::smatch element_h_eol_match;

std::regex element_p_sol_regex("^\\<p\\>");
std::smatch element_p_sol_match;

std::regex element_p_eol_regex("\\<\\/p\\>$"); // ^\\<\/p\\>
std::smatch element_p_eol_match;

std::regex ordered_list_regex("^\\d\\.\\s\\w+");
std::smatch ordered_list_match;

std::regex unordered_list_regex("^\\-\\s\\w+");
std::smatch unordered_list_match;

std::regex ol_or_ul_regex("^\\<ul\\>|^\\<ol\\>");
std::smatch ol_or_ul_match;

std::regex blockquote_regex("^\\>.*");
std::smatch blockquote_match;

// handle closing </ol> and </ul>
std::regex li_sol_regex("^\\t\\<li\\>");
std::smatch li_sol_match;

// handle formatting for multi-line <p> elements
std::regex li_wattr_regex("^\\t\\<li\\s\\w+\\=");
std::smatch li_wattr_match;

std::regex anchor_element_regex("^\\[.*\\)|[^!]\\[.*\\)");
std::smatch anchor_match;

//std::regex img_element_regex("[\!]\\[.*\\].+\\)");
std::regex img_element_regex("(!\\[.*\\].+\\))");
std::smatch img_match;

const char *trim_reg_str = " \t\n\r\f\v";

// links
// todo: [link name](url) or [link name][link definition name]

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
		void writeHTML(std::string line) {
			fileHtmlVec.push_back(line);
		};

};

struct FileData {
	std::string raw;
	std::vector<std::string> fvec;
	int numberOfLines;
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
			std::cout << line << std::endl;
			rawText += line + "\n";
			numberOfLines++;
			fileData.fvec.push_back(line);
		}
		// close the input file stream
		file.close();
	} else {
		std::cout << "Error opening file at path: " << filePath << std::endl;
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
std::string& ltrim(std::string& s, const char* t = trim_reg_str);
std::string& rtrim(std::string& s, const char* = trim_reg_str);
std::string& trim(std::string&, const char* = trim_reg_str);
void setHeadingLevel(std::unordered_map<std::string, std::string>);
std::string createElement(std::unordered_map<std::string, std::string>, std::string, bool isPrevLineParagraphText = false);


int main() {

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

	// heading elements <h1>-<h6>
	// assuming the headings are placed at the start of each line
	// e.g. ## Heading level 2
	std::regex heading_regex("^#{1,6}\\s\\w+.*");
	std::smatch heading_match;

	std::regex heading_text_regex("\\#([^#]*)\\[");
	std::smatch heading_text_match;
	std::string matchText = "";

	std::regex heading_level_regex("^[\\#{1-6}]+");
	std::smatch heading_level_match;

	std::regex paragraph_regex("^\\w.*");
	std::smatch p_match;
	std::regex elem_line_regex("^[<]");
	std::smatch p_match_ml;

	// use lineNum to keep track of replaced lines to ensure
	// proper order is preserved in HTML output
	// use a std::map which is ordered based on key
	std::map<int, std::string> lineMap;
	std::unordered_map<std::string, std::string> elemMap;
	int lineNum = 0;

	// Iterate each line of raw text read from the input file
	// and match lines that need to undergo transformation
	for (const auto &line : f.getRawVector()) {
		lineNum += 1;

		// <h1>-<h6> matching
		// if there is a match for heading element e.g. #, ##, ### to h6
		if (std::regex_search(line, heading_match, heading_regex)) {
			// iterate the matches
			for (int i = 0; i < heading_match.size(); i++) {
				// convert smatch[i] to a std::string to use with regex_search
				matchStr = heading_match[i];

				if (std::regex_search(matchStr, heading_level_match, heading_level_regex)) {
					elemMap["headingLevel"] = heading_level_match[0];
				}

				// search the text between ## and [] e.g. ## Title [class=foo]
				if (std::regex_search(matchStr, heading_text_match, heading_text_regex)) {
					elemMap["textContent"] = heading_text_match[1];
				}

				// determine heading level based on number of hashtags e.g. ## = <h2> and ### = <h3>
				setHeadingLevel(elemMap);
				lineMap[lineNum] = createElement(elemMap, "h" + headingLevel);
			}
		}

		// match blank lines
		if (std::regex_search(line, empty_line_match, empty_line_regex)) {
			lineMap[lineNum] = "";
		}

		// <p> matching
		if (std::regex_search(line, p_match, paragraph_regex)) {
			matchStr = p_match[0];
			elemMap["textContent"] = matchStr;
			lineMap[lineNum] = createElement(elemMap, "p");
		}

		// <ol> matching
		if (std::regex_search(line, ordered_list_match, ordered_list_regex)) {
			std::regex re("\\d\\.\\s");
			matchStr = std::regex_replace(line, re, "");
			elemMap["textContent"] = matchStr;
			if (lineMap[lineNum-1] == "") {
				lineMap[lineNum] = "<ol>\n\t" + createElement(elemMap, "li");
			} else {
				lineMap[lineNum] = "\t" + createElement(elemMap, "li");
			}
		}

		// <ul> matching
		if (std::regex_search(line, unordered_list_match, unordered_list_regex)) {
			std::regex re("\\-\\s");
			matchStr = std::regex_replace(line, re, "");
			elemMap["textContent"] = matchStr;
			if (lineMap[lineNum-1] == "") {
				lineMap[lineNum] = "<ul>\n\t" + createElement(elemMap, "li");
			} else {
				lineMap[lineNum] = "\t" + createElement(elemMap, "li");
			}
		}

		// <a> matching e.g. [link name](href)
		if (std::regex_search(line, anchor_match, anchor_element_regex)) {
			std::string linkName = "";
			std::string linkHref = "";
			if (std::regex_search(line, element_attr_match, element_attr_regex)) {
				linkName = element_attr_match[1];
			}
			if (std::regex_search(line, element_link_href_match, element_link_href_regex)) {
				linkHref = element_link_href_match[1];
			}
			matchStr = linkName + " [href=" + linkHref + "]";
			elemMap["textContent"] = matchStr;
			lineMap[lineNum] = createElement(elemMap, "a");
		}

		// <img> matching e.g. ![alt text](src)
		if (std::regex_search(line, img_match, img_element_regex)) {
			std::string altText = "";
			std::string imgSrc = "";
			if (std::regex_search(line, element_attr_match, element_attr_regex)) {
				altText = element_attr_match[1];
			}
			if (std::regex_search(line, element_link_href_match, element_link_href_regex)) {
				imgSrc = element_link_href_match[1];
			}
			matchStr = "[src=" + imgSrc + ", alt=" + altText + "]";
			elemMap["textContent"] = "";
			lineMap[lineNum] = createElement(elemMap, "img");
		}

		// <blockquote> matching
		if (std::regex_search(line, blockquote_match, blockquote_regex)) {
			std::regex re("\\>\\s");
			matchStr = std::regex_replace(static_cast<std::string>(blockquote_match[0]), re, "");
			elemMap["textContent"] = matchStr;
			lineMap[lineNum] = createElement(elemMap, "blockquote");
		}

		// single line <code> matching
		if (std::regex_search(line, sl_code_match, single_line_code_regex)) {
			matchStr = sl_code_match[1];
			elemMap["textContent"] = matchStr;
			lineMap[lineNum] = createElement(elemMap, "code");
		}

		// todo: multi-line code snippets (<pre> + nested <code>)

	}
	cout << "Printing Ordered Map" << endl;
	std::map<int, std::string>::iterator mapItr;
	for (int i = 1; i <= lineMap.size(); i++) {
		cout << i << ": " << lineMap[i] << endl;
		bool isPrevLineCompleted = false;
		bool isNextLineCompleted = false;
		bool nextLineStartsWithTag = false;

		if (i > 1) {
			if (i+1 <= lineMap.size()) {

				std::string prevLine = lineMap[i-1];
				std::string currLine = lineMap[i];
				std::string nextLine = lineMap[i+1];

				// handle formatting ordered/unordered lists
				if (std::regex_search(currLine, ol_or_ul_match, ol_or_ul_regex) && std::regex_search(nextLine, ol_or_ul_match, ol_or_ul_regex)) {
					elemMap["textContent"] =  ol_or_ul_match.suffix();
				}

				// insert closing list elements
				if (std::regex_search(currLine, ol_or_ul_match, ol_or_ul_regex)) {
					elemMap["listType"] = static_cast<std::string>(ol_or_ul_match[0]).insert(1, 1, '/');
				}

				// check if the current line is the last <li>
				// element in a unordered or ordered list and close the tag
				if ((std::regex_search(currLine, li_wattr_match, li_wattr_regex) && nextLine == "") ||
					(std::regex_search(currLine, li_sol_match, li_sol_regex) && nextLine == "")) {
					lineMap[i] += "\n" + elemMap["listType"];
				}

				// single list item ul or ol
				if (std::regex_search(currLine, ol_or_ul_match, ol_or_ul_regex) && nextLine == "") {
					lineMap[i] += "\n" + elemMap["listType"];
				}

				bool isCurrLineFullPTag = (std::regex_search(currLine, element_p_sol_match, element_p_sol_regex)
				&& std::regex_search(currLine, element_p_eol_match, element_p_eol_regex));

				// Remove opening and closing <p>,</p> tags for text
				// in between the start and end in a multi-line paragraph block
				if (std::regex_search(nextLine, element_p_sol_match, element_p_sol_regex)&& isCurrLineFullPTag && nextLine != "") {
					lineMap[i] = std::regex_replace(std::regex_replace(currLine, element_p_eol_regex, ""), element_p_sol_regex, "");
				}

				// Remove closing </p> tag on first line in multi-line paragraph block
				if (std::regex_search(currLine, element_p_sol_match, element_p_sol_regex) && prevLine == "" && nextLine != "") {
					lineMap[i] = std::regex_replace(currLine, element_p_eol_regex, "");
				}

				// Remove starting <p> tag on last line in multi-line parapgraph block
				if (std::regex_search(currLine, element_p_sol_match, element_p_sol_regex) && prevLine != "" && nextLine == "") {
					lineMap[i] = std::regex_replace(currLine, element_p_sol_regex, "");
				}
			}

			// if line-1 is a completed line e.g. <h2 class="foo">hi there</h2>
			// its not involved in a multi-line <p> tag
			if (std::regex_search(lineMap[i-1], element_sol_match, element_sol_regex) &&
				std::regex_search(lineMap[i-1], element_eol_match, element_eol_regex)
			) {
				isPrevLineCompleted = true;
			}

			if (!isPrevLineCompleted && std::regex_search(lineMap[i-1], p_match, paragraph_regex)) {
				std::string prevLineText = p_match[0];
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

std::string createElement(std::unordered_map<std::string, std::string> elemMap, std::string htmlTag, bool isPrevLineParagraphText) {
	elemMap["attribues"] = "";
    if (std::regex_search(matchStr, element_attr_match, element_attr_regex)) {
        // handle any element attribute definitions
        // match everything inside square brackets
        elemMap["attributes"] = element_attr_match[1];
    }

    if (elemMap["attributes"].empty()) {
    	if (htmlTag != "li") {
    		if (std::regex_search(matchStr, t_match, text_no_attr_regex)) {
        		elemMap["textContent"] = t_match[0];
    		}
    	}
    	// todo: look for <a> or <img> within <p> tags
    } else {
		if (std::regex_search(matchStr, t_match, paragraph_text_regex)) {
			std::string s = t_match[0];
			s.pop_back();
			elemMap["textContent"] = s;
		}
    }

    // Use optional delimeter paramter of getline()
    // to mimic a String.split("=") call for handling class/id definitions
    // if there is multiple attribute definitions e.g.
    // ## some title [class=foo,id=bar] first split
    // by comma then handle specific attrs
    std::vector<std::string> vecElemAttr;
    std::istringstream ss(elemMap["attributes"]);
    std::vector<std::string> elementFields;
    std::string sk;
    std::string sl;
    std::string ln;
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
            if (elemAttrKeys.size() >= 2 && (j == 0 || j % 2 == 0)) {
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
        	if (std::regex_search(elemMap["textContent"], anchor_match, anchor_element_regex)) {
        		elemMap["textContent"] = std::regex_replace(static_cast<std::string>(anchor_match[0]), anchor_element_regex, "<a>" + static_cast<std::string>(anchor_match[0]) + "</a>");
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

void setHeadingLevel(std::unordered_map<std::string, std::string> elemMap) {
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

// Get current working directory printed to std out
void cwd() {
	char *getcwd(char *buf, size_t size);
	char cwd[256]; // 256 character limit max
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("Current working dir: %s\n", cwd);
	} else {
		perror("getcwd() error");
	}
}