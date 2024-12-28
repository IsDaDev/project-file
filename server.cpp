#include <iostream>
#include <string>
#include <filesystem>
#include "./headers/json.hpp"
#include "./headers/httplib.h"
#include "./headers/base64.h"

using namespace std;

struct fileInfo 
{
    string name;
    double fSize; // File size in KB
    string type;  // "file" or "directory"
    string path;  // Full file path
    string content;
    int flag;
};

string convertToJSON(const fileInfo& file) 
{
    nlohmann::json j;
    j["name"] = file.name;
    j["size"] = file.fSize;
    j["type"] = file.type;
    j["path"] = file.path;
    j["content"] = file.content;
    j["flag"] = file.flag;
    return j.dump();
}

int findLast(const string& str) 
{
    size_t lastSlash = str.find_last_of('/');
    return lastSlash;
}

string returnFileContent(string filepath) 
{
    ifstream inputStream(filepath);
    string data = "";
    string line;

    while(getline(inputStream, line)) {
        data = data + line + "\n";
    }

    string base64Data = base64_encode(reinterpret_cast<const unsigned char*>(data.c_str()), data.length());

    return base64Data;
}

string emptyDirectory(string inp)
{
    int counter = 0;
    string ret = "";
    fileInfo file = {inp, 0, "", "", "", 0};
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
        string fileContent = "";

        if (!entry.is_directory()) {
            fileSizeKB = entry.file_size() / 1000;
            fileType = "file";

            if (filesystem::path(filePath).extension() != "") {
                fileContent = returnFileContent(filePath);
            } else {
                fileContent = "binary file";
            }
        };

        fileInfo file = {fileName, fileSizeKB, fileType, filePath, fileContent, 0};
        ret = ret + convertToJSON(file);
        counter++;
    };

    string returnString = ret + "COUNT:" + to_string(counter);

    return returnString;
}

string getFile(string path)
{
    string ret;

    filesystem::directory_entry entry(path);

    string filePath = entry.path();
    string fileName = filePath.substr(findLast(filePath) + 1);
    string fileType = "file";
    string fileContent = "";

    double fileSizeKB = entry.file_size() / 1000;
    if (filesystem::path(filePath).extension() != "") {
        fileContent = returnFileContent(filePath);
    } else {
        fileContent = "binary file";
    }

    fileInfo file = {fileName, fileSizeKB, fileType, filePath, fileContent, 1};
    return convertToJSON(file);
}

int main(int argc, char* argv[]) 
{
    string baseDir = "/home/isda/Documents/coding/project-file/project-file";

    cout << "Server is online" << endl;

    int counter = 0;
    httplib::Server server;

    // get request for modified path 
    server.Get(R"(/list(.*))", [&counter, &baseDir](const httplib::Request& req, httplib::Response& res) 
    {
        cout << "Request for connection sent" << endl;

        string pathParam = req.matches[1];
        if (!pathParam.empty() && pathParam[0] == '/') {
            pathParam.erase(0, 1); 
        }

        string fullPath = baseDir + "/" + pathParam;
        cout << "GET_REQUEST: " << fullPath << endl;

        if (!filesystem::exists(fullPath)) {
            cerr << "Error: Path does not exist - " << fullPath << endl;
            res.set_content(emptyDirectory("not found"), "application/json");
            return;
        }

        if (filesystem::is_regular_file(fullPath)) {
            res.set_content(getFile(fullPath), "application/json");
            return;
        }

        counter++;
        cout << "Accessing: " << fullPath << endl;
        cout << "Connections: " << counter << endl;

        res.set_content(listDirectories(fullPath), "application/json");
    });

    server.POST(R"(/list(.*))", [&counter, &baseDir](const httplib::Request& req, httplib::Response& res))
    {
        string pathParam = req.matches[1];
        if (!pathParam.empty() && pathParam[0] == '/') {
            pathParam.erase(0, 1); 
        }

        string fullPath = baseDir + "/" + pathParam;
        cout << "POST_REQUEST: " << fullPath << endl;

        if (!filesystem::exists(fullPath)) {
            cerr << "Error: Path does not exist - " << fullPath << endl;
            res.set_content(emptyDirectory("not found"), "application/json");
            return;
        }
    }

    server.listen("127.0.0.1", 8000);

    return 0;
}
