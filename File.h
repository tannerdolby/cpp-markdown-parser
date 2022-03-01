#include <iostream>
#include <vector>
#include <unordered_map>

class File {
	char *path;
	char *outputPath;
	std::string raw, html;
	int numLines;
	std::vector<std::string> linesRaw;
	std::vector<std::string> linesHtml;
public:
	File(char *f, char *op) { path = f; outputPath = op; numLines = 0; };

	void read() {
		std::ifstream f;
		std::string rawText = "", line = "";
		int lineCount = 0;

		f.open(path, std::ios_base::in);

		if (f.is_open()) {
			while (getline(f, line)) {
				std::cout << line << std::endl; // todo: remove
				rawText += line + "\n";
				lineCount++;
				linesRaw.push_back(line);
			}
			f.close();
		} else {
			std::cout << "Error opening file at path: " << path << std::endl;
		}

		raw = rawText;
		numLines = lineCount;
	}

	std::vector<File> readFiles(std::vector<std::pair<char*, char*> > files) {
		// todo: ["path1", "path2"] -> vector<File> and then write each output file
		std::vector<File> fileVec;
		for (auto fp : files) {
			// create a new file instance then read file by file
			File f(fp.first, fp.second);

			// todo: do transformations

			// Open the output file and truncate previous files contents if it already exists
			std::ofstream ofs;
			char* outputPath = "";
			ofs.open(f.outputPath, std::ios_base::ate);
		}

		return fileVec;
	};

	char* getFilePath() {
		return path;
	}

	char* getFileOutPath() {
		return outputPath;
	}

	std::string getRawText() {
		return raw;
	}

	std::string getHTML() {
		return html;
	}

	std::vector<std::string> getRawVec() {
		return linesRaw;
	}

	std::vector<std::string> getHtmlVec() {
		return linesHtml;
	}

	int getNumberOfLines() {
		return numLines;
	}

	// wip
	void writeHTML(std::unordered_map<std::string, std::string> &lineMap, std::ofstream &ofs) {
		for (auto it = lineMap.begin(); it != lineMap.end(); ++it) {
			std::cout << it -> first;
			std::cout << ": " << it -> second << std::endl;
			// write to output file stream e.g. resultant HTML file
			ofs << it -> second << std::endl;
		}
	}
};