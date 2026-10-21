#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <variant>
#include <stack>

class File {
    public:
    std::string type;
    std::string name;
    std::string content;
    File(std::string& type, std::string& name, std::string content) {
        this->type = type;
        this->name = name;
        this->content = content;
    }

    // 追加内容
    void append(std::string content) {
        this->content.append(content);
    }
    // 输出
    void output() {
        std::cout << this->name << ':' << this->content << std::endl;
    }
    // 二进制文件读取
    bool read_bin(int offset, int length) {
        int b = this->content.size() / 2; // 计算字节长度
        if (offset + length > b) return false;
        int start = offset * 2;
        int end = (offset + length) * 2;
        int cnt = 0;
        std::cout << this->name << ':';
        for (int i = start; i < end; i++) {
            if (cnt > 0 && cnt % 2 == 0) std::cout << ' ';
            if (this->content.at(i) != ' ') {
                std::cout << this->content.at(i);
                cnt ++;
            }
        }
        std::cout << std::endl;
        return true;
    }
    void write_bin(int offset, std::string hexdata) {
        int start = offset * 2;
        std::stack<char> buf; // 设置隔离缓冲
        while (this->content.size() > start) {
            buf.push(content.back());
            this->content.pop_back();
        }
        this->content += hexdata;
        int cnt = 0, len = hexdata.size();
        // 覆写
        while (!buf.empty()) {
            if (cnt >= hexdata.size()) this->content.push_back(buf.top());
            buf.pop();
            cnt ++;
        }
    }
    int get_size() {
        if (this->type == "bin") return this->content.size() / 2;
        else return this->content.size();
    }
    ~File(){}
};

class Directory {
    public:
    bool root = false; // 是否是根目录
    std::string name; // 文件夹名
    Directory* parent; // 父文件夹指针
    std::map<std::string, std::variant<Directory*, File*>> content; // 子文件指针，键值是文件夹名
    std::string get_path(); // 获取路径

    Directory() {
        root = true;
    }

    Directory(std::string& name, Directory* parent) {
        this->name = name;
        this->parent = parent;
    }

    bool make_dir(std::string& name) {
        // 创建子目录
        if (this->content.find(name) != this->content.end()) return false;
        Directory* new_dir = new Directory(name, this);
        this->content.insert({name, new_dir});
        return true;
    }

    bool create_file(std::string& type, std::string& name, std::string& content) {
        // 创建文件
        if (this->content.find(name) != this->content.end()) return false;
        File* new_file = new File(type, name, content);
        this->content.insert({name, new_file});
        return true;
    }

    bool append(std::string& name, std::string& content) {
        // 追加内容
        if (this->content.find(name) == this->content.end() || std::get_if<File*>(&(this->content.at(name))) == nullptr) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type == "bin") return false;
        file->append(content);
        return true;
    }

    bool output(std::string& name) {
        // 输出文本文档内容
        if (this->content.find(name) == this->content.end() || std::get_if<File*>(&(this->content.at(name))) == nullptr) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type == "bin") return false;
        file->output();
        return true;
    }

    bool read_bin(std::string& name, int offset, int length) {
        // 读二进制文件内容
        if (this->content.find(name) == this->content.end() || std::get_if<File*>(&(this->content.at(name))) == nullptr) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type != "bin") return false;
        return file->read_bin(offset, length);
    }

    bool write_bin(std::string& name, int offset, std::string& hexdata) {
        // 写二进制文件内容
        if (this->content.find(name) == this->content.end() || std::get_if<File*>(&(this->content.at(name))) == nullptr) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type != "bin") return false;
        file->write_bin(offset, hexdata);
        return true;
    }

    bool remove_file(std::string& name) {
        if (this->content.find(name) == this->content.end() || std::get_if<File*>(&(this->content.at(name))) == nullptr) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        this->content.erase(name); // 清除content的文件名
        delete file;
        return true;
    }

    bool get_stat(std::string name) {
        if (this->content.find(name) == this->content.end()) return false;
        auto ptr = std::get_if<File*>(&(this->content.at(name)));
        if (ptr == nullptr) {
            Directory* dir = *std::get_if<Directory*>(&(this->content.at(name)));
            std::cout << name << ':' << "dir" << ':' << 0 << std::endl;
        } else {
            File* file = *ptr;
            std::cout << name << ':' << file->type << ':' <<file->get_size() << std::endl;
        }
        return true;
    }

    void print_dir() {
        // ls 指令打印当前目录，带空格
        std::string output;
        for (auto item : this->content) {
            output += item.first;
            if (std::get_if<Directory*>(&item.second) != nullptr) output += '/';
            output += '\n';
        }
        std::cout << output;
    }
    ~Directory();
};

class FileSys {
    private:
    std::vector<std::string> split_path(std::string& path) {
        // 分割路径
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
};