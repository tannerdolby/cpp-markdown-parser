#include <iostream>
using namespace std;

// trim from left
string& ltrim(string& s, const char* t)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
string& rtrim(string& s, const char* t)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
string& trim(string& s, const char* t)
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