#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

class File {
	char *path;
	char *outputPath;
	string raw, html;
	int numLines;
	vector<string> linesRaw;
	vector<string> linesHtml;
public:
	File(char *f, char *op) { path = f; outputPath = op; numLines = 0; };

	void read() {
		ifstream f;
		string rawText = "", line = "";
		int lineCount = 0;

		f.open(path, ios_base::in);

		if (f.is_open()) {
			while (getline(f, line)) {
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

	vector<File> readFiles(vector<pair<char*, char*> > files) {
		// todo: ["path1", "path2"] -> vector<File> and then write each output file
		vector<File> fileVec;
		for (auto fp : files) {
			// create a new file instance then read file by file
			File f(fp.first, fp.second);
	//		f.read();

			// todo: do transformations

			// Open the output file and truncate previous files contents if it already exists
			ofstream ofs;
			char* outputPath = "";
			ofs.open(f.outputPath, ios_base::ate);
		}

		return fileVec;
	};

	char* getFilePath() {
		return path;
	}

	char* getFileOutPath() {
		return outputPath;
	}

	string getRawText() {
		return raw;
	}

	string getHTML() {
		return html;
	}

	vector<string> getRawVec() {
		return linesRaw;
	}

	vector<string> getHtmlVec() {
		return linesHtml;
	}

	int getNumberOfLines() {
		return numLines;
	}

	// wip
	void writeHTML(unordered_map<string, string> &lineMap, ofstream &ofs) {
		for (auto it = lineMap.begin(); it != lineMap.end(); ++it) {
			cout << it -> first;
			cout << ": " << it -> second << endl;
			// write to output file stream e.g. resultant HTML file
			ofs << it -> second << endl;
		}
	}
};