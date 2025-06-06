#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#include <windows.h>
#include <string>
#include <unordered_map>

class FileWatcher {
public:
    explicit FileWatcher(const std::string& path = "");
    ~FileWatcher();

    void setWatchPath(const std::string& path);
    bool checkChanges();  // Devuelve true si hubo cambios

private:
    std::string watch_path_;
    std::unordered_map<std::string, FILETIME> file_times_;

    bool scanDirectoryRecursive(const std::string& path);
    static FILETIME getFileTime(const std::string& filepath);
};

#endif // FILE_WATCHER_H