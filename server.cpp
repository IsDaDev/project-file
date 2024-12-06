#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "./headers/json.hpp"
#include "./headers/httplib.h"

using namespace std;

struct fileInfo {
    string name;
    double fSize;
    string type;
    string path;
};

string convertToJSON(fileInfo& fileInfo)
{
    nlohmann::json j;
    j["name"] = fileInfo.name;
    j["size"] = fileInfo.fSize;
    j["type"] = fileInfo.type;
    j["path"] = fileInfo.path;
    return j.dump();
}
int findLast(const string& str) {
    size_t lastSlash = str.find_last_of('/');
    return lastSlash;
}

string listDirectories(string dir)
{
    int counter = 1;
    string ret = "";

    vector<string> directories;

    for (const filesystem::directory_entry& entry : 
    filesystem::directory_iterator(dir)) {
        string filePath = entry.path();
        string fileName = filePath.substr(findLast(filePath) + 1);
        double fileSizeKB = 0;
        string fileType = "directory";

        if (entry.is_directory()) {
            directories.push_back(fileName);
        }

        if (!entry.is_directory()) {
            fileSizeKB = entry.file_size() / 1000;
            fileType = "file";
        };

        fileInfo file = {fileName, fileSizeKB, fileType, filePath};
        ret = ret + convertToJSON(file);
        counter++;
    };

    string returnString = ret + "COUNT:" + to_string(counter);

    return returnString;
}

int main(int argc, char *argv[])
{
    cout << "Server is online" << endl;

    string dir = argv[1];

    int counter = 0;
    httplib::Server server;

    server.Get("/list", [&counter, &dir](const httplib::Request&, httplib::Response& res) {
        counter = counter + 1;
        cout << "Connections: ";
        cout << counter << endl;
        res.set_content(listDirectories(dir), "application/json");
    });

    server.listen("127.0.0.1", 8000);

    return 0;
}
