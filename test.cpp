#include <string>
#include <iostream>
#include <filesystem>

using namespace std;

int main()
{
    string path = "/home/isda/Documents/coding/project-file/project-file/server";
    std::cout << filesystem::path(path).extension() << '\n';
}