#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <variant>

class Directory {
    public:
    bool root = false; // 是否是根目录
    std::string name; // 文件夹名
    Directory* parent; // 父文件夹指针
    std::map<std::string, std::variant<Directory*>> content; // 子文件指针，键值是文件夹名
    std::string get_path(); // 获取路径

    Directory() {
        root = true;
    }

    Directory(std::string& name, Directory* parent) {
        this->name = name;
        this->parent = parent;
    }

    bool make_dir(std::string name) {
        // 创建子目录
        if (this->content.find(name) != this->content.end()) return false;
        Directory* new_dir = new Directory(name, this);
        this->content.insert({name, new_dir});
        return true;
    }
};

class FileSys {
    private:
    std::vector<std::string> split_path(std::string& path) {
        std::vector<std::string> res;
        std::string token;
        for (auto& c : path) {
            if (c == '/') {
                if (!token.empty()) res.push_back(token);
                token = "";
            }
            token.push_back(c);
        }
        res.push_back(token);
        return res;
    }
    public:
    Directory* cwd; // 现在工作文件位置
    Directory* root; // 根目录
    FileSys() {
        // 构造函数
        root = new Directory();
        cwd = root;
    }

    bool switch_path(std::string& path) {
        // 更改路径
        if (path.empty()) return false;
        Directory* new_path;
        std::vector<std::string> parsed = split_path(path);
        if (path[0] == '/') {
            new_path = root;
            for (auto& token : parsed) {
                // 访问到不存在的文件夹，静默处理
                if (new_path->content.find(token) != new_path->content.end() || std::get_if<Directory*>(&(new_path->content.at(token))) == nullptr) return true; 
                new_path = *std::get_if<Directory*>(&(new_path->content.at(token)));
            }
        } else {
            new_path = cwd;
            for (auto& token : parsed) {
                if (token == ".") continue;
                else if (token == "..") {
                    if (new_path->root) return false; // 检测根目录越界
                    new_path = new_path->parent;
                } else {
                // 访问到不存在的文件夹，静默处理
                    if (new_path->content.find(token) == new_path->content.end() || std::get_if<Directory*>(&(new_path->content.at(token))) == nullptr) return true; 
                    new_path = *std::get_if<Directory*>(&(new_path->content.at(token)));
                }
            }
        }
        cwd = new_path;
        return true;
    }

    std::string print_cwd() {
        // ls 指令打印当前目录
        std::string output;
        for (auto item : cwd->content) {
            output += item.first;
            output += '\n';
        }
        return output;
    }
};