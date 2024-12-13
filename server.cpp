#include <iostream>
#include <string>
#include <filesystem>
#include "./headers/json.hpp"
#include "./headers/httplib.h"

using namespace std;

struct fileInfo {
    string name;
    double fSize; // File size in KB
    string type;  // "file" or "directory"
    string path;  // Full file path
};

string convertToJSON(const fileInfo& file) {
    nlohmann::json j;
    j["name"] = file.name;
    j["size"] = file.fSize;
    j["type"] = file.type;
    j["path"] = file.path;
    return j.dump();
}

int findLast(const string& str) {
    size_t lastSlash = str.find_last_of('/');
    return lastSlash;
}

string emptyDirectory(string inp)
{
    int counter = 0;
    string ret = "";
    fileInfo file = {inp, 0, "", ""};
    return convertToJSON(file) + "COUNT:" + to_string(counter);
}

string listDirectories(string dir)
{
    if (filesystem::is_empty(dir)) {
        return emptyDirectory("");
    }
    
    int counter = 1;
    string ret = "";

    for (const filesystem::directory_entry& entry : 
    filesystem::directory_iterator(dir)) {
        string filePath = entry.path();
        string fileName = filePath.substr(findLast(filePath) + 1);
        double fileSizeKB = 0;
        string fileType = "directory";

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

vector<string> returnFileContent(string filepath) 
{
    ifstream inputStream(filepath);
    vector<string> data;
    string line;

    while(getline(inputStream, line)) {
        data.push_back(line);
    }

    if (!inputStream) {
        data.push_back("error opening file");
        return data;
    }

    return data;
}

int main(int argc, char* argv[]) 
{
    string baseDir = "/home/isda/Documents/coding/project-file/project-file";

    cout << "Server is online" << endl;

    int counter = 0;
    httplib::Server server;

    // get request for modified path 
    // #TODO: check for permission
    server.Get(R"(/list(.*))", [&counter, &baseDir](const httplib::Request& req, httplib::Response& res) 
    {
    cout << "Request for connection sent" << endl;

    string pathParam = req.matches[1];
    if (!pathParam.empty() && pathParam[0] == '/') {
        pathParam.erase(0, 1); 
    }

    string fullPath = baseDir + "/" + pathParam;
    cout << "Resolved fullPath: " << fullPath << endl;

    if (!filesystem::exists(fullPath)) {
        cerr << "Error: Path does not exist - " << fullPath << endl;
        res.set_content(emptyDirectory("not found"), "application/json");
        return;
    }

    counter++;
    cout << "Accessing: " << fullPath << endl;
    cout << "Connections: " << counter << endl;

    res.set_content(listDirectories(fullPath), "application/json");
});

    server.listen("127.0.0.1", 8000);

    return 0;
}
