#pragma once
#include "GInclude.h"


static std::vector<std::string> getlistFilesWithExtension(const char* path, const char* extension) {
    std::vector<std::string> filenames;

    char command[256];
    sprintf_s(command, "dir \"%s\\*%s\"", path, extension);

    FILE* pipe = _popen(command, "r");
    if (!pipe) {
        std::cerr << "Error opening pipe." << std::endl;
        return filenames;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // Remove newline character
        buffer[strcspn(buffer, "\n")] = '\0';
        // Check if the file has the specified extension


        if (strstr(buffer, extension) != nullptr) {
            string str_buf = string(buffer);
            if (str_buf[str_buf.length() - 1] == ' ') //Remove the last space
                str_buf = str_buf.substr(0, str_buf.length() - 1);

            int found = (int)str_buf.rfind(" ");
            if (found != std::string::npos)
                str_buf = str_buf.substr(found + 1, str_buf.length() - (found + 1));

            filenames.push_back(str_buf);
        }
    }

    _pclose(pipe);

    return filenames;
}


