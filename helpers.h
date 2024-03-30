#include <string>

// trim from left
std::string& ltrim(std::string &s, const char *t) {
	s.erase(0, s.find_first_not_of(t));
	return s;
}

// trim from right
std::string& rtrim(std::string &s, const char *t) {
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

// trim from left & right
std::string& trim(std::string &s, const char *t) {
	return ltrim(rtrim(s, t), t);
}

// print current working directory to std out
void printcwd() {
	char* getcwd(char *buf, size_t size);
	char cwd[256]; // 256 character limit max
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("Current working dir: %s\n", cwd);
	} else {
		perror("getcwd() error");
	}
}
