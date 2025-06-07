#include "FileWatcher.h"
#include "../include/log.h"
#include <iostream>
#include <format>

FileWatcher::FileWatcher(const std::string& path) {
    if (!path.empty()) {
        setWatchPath(path);
    }
}

FileWatcher::~FileWatcher() {}

void FileWatcher::setWatchPath(const std::string& path) {
    watch_path_ = path;
    file_times_.clear();
    scanDirectoryRecursive(path);
}

bool FileWatcher::scanDirectoryRecursive(const std::string& path) {
    WIN32_FIND_DATA find_data;
    std::string search_path = path + "\\*";
    HANDLE h_find = FindFirstFile(search_path.c_str(), &find_data);

    if (h_find == INVALID_HANDLE_VALUE) {
        Log::warn(std::format("Error leyendo el directorio por defecto [public] ",  GetLastError()));
        return false;
    }

    do {
        std::string file_name = find_data.cFileName;
        if (file_name == "." || file_name == "..") {
            continue;
        }

        std::string full_path = path + "\\" + file_name;

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Es un directorio, escanear recursivamente
            scanDirectoryRecursive(full_path);
        } else {
            // Es un archivo, registrar su tiempo de modificación
            file_times_[full_path] = find_data.ftLastWriteTime;
        }
    } while (FindNextFile(h_find, &find_data));

    FindClose(h_find);
    return true;
}

bool FileWatcher::checkChanges() {
    bool changed = false;
    WIN32_FIND_DATA find_data;

    for (auto it = file_times_.begin(); it != file_times_.end(); ) {
        std::string file_path = it->first;
        HANDLE h_file = FindFirstFile(file_path.c_str(), &find_data);
        
        if (h_file == INVALID_HANDLE_VALUE) {
            // Archivo eliminado
            it = file_times_.erase(it);
            changed = true;
        } else {
            if (CompareFileTime(&find_data.ftLastWriteTime, &it->second) != 0) {
                // Archivo modificado
                it->second = find_data.ftLastWriteTime;
                changed = true;
            }
            ++it;
            FindClose(h_file);
        }
    }

    return changed;
}