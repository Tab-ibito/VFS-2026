#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <variant>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include <set>

class Operation;
class File;
class Directory;
class FileSys{
    private:
    void split_path(std::string& path, std::vector<std::string>& res);
    std::stack<Operation*> history;
    public:
    Directory* cwd;
    Directory* root;
    bool switch_path(std::string& path);
    void add_history(Operation* op);
    bool undo();
    FileSys();
    ~FileSys(){};
};

class Operation {
    public:
    Directory* dir;
    std::string name, type, file_type;
    bool is_created, is_removed;
    int diff;
    std::string old_content;
    // 增（文件夹 / 文件），删改（文件）
    Operation(Directory* dir, std::string& type) {
        this->dir = dir;
        this->type = type;
    }
    Operation(Directory* dir, std::string& name, std::string& type) {
        this->dir = dir;
        this->name = name;
        this->type = type;
    }
    Operation(Directory* dir, std::string& name, std::string& type, std::string& file_type, std::string& old_content) {
        this->dir = dir;
        this->name = name;
        this->type = type;
        this->file_type = file_type;
        this->old_content = old_content;
    }
        Operation(Directory* dir, std::string& name, std::string& type, std::string& file_type, int diff) {
        this->dir = dir;
        this->name = name;
        this->type = type;
        this->file_type = file_type;
        this->diff = diff;
    }
    Operation(Directory* dir, std::string& name, std::string& type, std::string& file_type, int diff, std::string& old_content) {
        this->dir = dir;
        this->name = name;
        this->type = type;
        this->file_type = file_type;
        this->diff = diff;
        this->old_content = old_content;
    }
};  

class File {
    public:
    std::string type;
    std::string name;
    std::string content;
    File(std::string& type, std::string& name, std::string& content) {
        this->type = type;
        this->name = name;
        this->content = content;
    }

    // 追加内容
    void append(std::string& content) {
        this->content.append(content);
    }
    // 输出
    void output() {
        std::cout << this->name << ':' << this->content << '\n';
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
        std::cout << '\n';
        return true;
    }
    void write_bin(int offset, std::string& hexdata) {
        int start = offset * 2;
        std::stack<char> buf; // 设置隔离缓冲

        while (this->content.size() > start) {
            buf.push(content.back());
            this->content.pop_back();
        }
        // 没到offset的地方先填充
        while (this->content.size() < start) {
            this->content.push_back('0');
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

struct CompareSize {
    bool operator()(const std::pair<std::string, int>& a,
                    const std::pair<std::string, int>& b) const {
        if (a.second != b.second)
            return a.second > b.second;
        return a.first < b.first;
    }
};

class Directory {
    private:
    bool is_valid_file(std::string& name) {
        return !(this->content.find(name) == this->content.end() || std::get_if<File*>(&(this->content.at(name))) == nullptr);
    }
    
    public:
    FileSys* filesys; // 便于撤销操作回传
    bool root = false; // 是否是根目录
    std::string name; // 文件夹名
    Directory* parent; // 父文件夹指针
    std::unordered_map<std::string, std::variant<Directory*, File*>> content; // 子文件指针，键值是文件夹名
    std::set<std::string> seq_dict; // 按照字典序输出文件
    std::set<std::pair<std::string, int>, CompareSize> seq_size; // 按照字典序输出文件

    Directory(FileSys* filesys) {
        root = true;
        this->filesys = filesys;
    }

    Directory(std::string& name, Directory* parent, FileSys* filesys) {
        this->name = name;
        this->parent = parent;
        this->filesys = filesys;
    }

    bool make_dir(std::string& name) {
        // 创建子目录 mkdir
        std::string order = "mkdir";
        if (this->content.find(name) != this->content.end()) return false;
        Directory* new_dir = new Directory(name, this, this->filesys);
        this->content.insert({name, new_dir});
        this->filesys->add_history(new Operation(this, name, order));
        this->seq_dict.insert(name);
        return true;
    }

    bool create_file(std::string& type, std::string& name, std::string& content, bool is_recorded = true) {
        // 创建文件
        std::string order = "create";
        if (this->content.find(name) != this->content.end()) return false;
        File* new_file = new File(type, name, content);
        this->content.insert({name, new_file});
        if (is_recorded) this->filesys->add_history(new Operation(this, name, order, type, content));
        this->seq_dict.insert(name);
        this->seq_size.insert({name, new_file->get_size()});
        return true;
    }

    bool append(std::string& name, std::string& content, bool is_recorded = true, int diff = 0) {
        // 追加内容
        std::string order = "append";
        if (!is_valid_file(name)) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type == "bin") return false;
        this->seq_size.erase({name, file->get_size()});
        if(is_recorded) {
            this->filesys->add_history(new Operation(this, name, order, file->type, file->get_size()));
            file->append(content);
        }
        else file->content = file->content.substr(0, diff);
        this->seq_size.insert({name, file->get_size()});
        return true;
    }

    bool output(std::string& name) {
        // 输出文本文档内容
        if (!is_valid_file(name)) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type == "bin") return false;
        file->output();
        return true;
    }

    bool read_bin(std::string& name, int offset, int length) {
        // 读二进制文件内容
        if (!is_valid_file(name)) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type != "bin") return false;
        return file->read_bin(offset, length);
    }

    bool write_bin(std::string& name, int offset, std::string& hexdata, bool is_recorded = true) {
        // 写二进制文件内容
        std::string order = "write";
        if (!is_valid_file(name)) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type != "bin") return false;
        int diff = offset < file->get_size() ? offset : file->get_size();
        std::string old_content = offset > file->get_size() ? "" : file->content.substr(offset*2);
        this->seq_size.erase({name, file->get_size()});
        if (is_recorded) {
            file->write_bin(offset, hexdata);
            this->filesys->add_history(new Operation(this, name, order, file->type, diff, old_content));
        } else {
            file->content = file->content.substr(0, offset*2) + hexdata;
        } 
        this->seq_size.insert({name, file->get_size()});
        return true;
    }

    bool remove_file(std::string& name, bool is_recorded = true) {
        std::string order = "rm";
        if (!is_valid_file(name)) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (is_recorded) this->filesys->add_history(new Operation(this, name, order, file->type, file->content));
        this->seq_size.erase({name, file->get_size()});
        this->content.erase(name); // 清除content的文件名
        this->seq_dict.erase(name);
        return true;
    }

    bool get_stat(std::string& name) {
        // 输出文件类型和大小
        if (this->content.find(name) == this->content.end()) return false;
        auto ptr = std::get_if<File*>(&(this->content.at(name)));
        if (ptr == nullptr) {
            Directory* dir = *std::get_if<Directory*>(&(this->content.at(name)));
            std::cout << name << ':' << "dir" << ':' << 0 << '\n';
        } else {
            File* file = *ptr;
            std::cout << name << ':' << file->type << ':' << file->get_size() << '\n';
        }
        return true;
    }

    void print_topk(int k) {
        int cnt = 0;
        auto it = seq_size.begin();
        while (cnt < k && it != seq_size.end()) {
            std::cout << it->first << ':' << it->second << '\n';
            cnt ++;
            it ++;
        }
    }

    void print_dir() {
        // ls 指令打印当前目录，带空格
        for (auto& item : this->seq_dict) {
            std::cout << item;
            if (std::get_if<Directory*>(&(this->content.at(item))) != nullptr) std::cout << '/';
            std::cout << '\n';
        }
    }
    ~Directory(){}
};

void FileSys::split_path(std::string& path, std::vector<std::string>& res) {
    // 分割路径
    std::string token;
    for (auto& c : path) {
        if (c == '/') {
            if (!token.empty()) res.push_back(token);
            token = "";
        } else token.push_back(c);
    }
    if (!token.empty()) res.push_back(token);
}

bool FileSys::switch_path(std::string& path) {
    // 更改路径
    std::string order = "cd";
    if (path.empty()) return false;
    Directory* new_path;
    std::vector<std::string> parsed;
    split_path(path, parsed);
    if (path[0] == '/') {
        new_path = root;
    } else {
        new_path = cwd;
    }
    for (auto& token : parsed) {
        if (token == ".") continue;
        else if (token == "..") {
            if (new_path->root) return false; // 检测根目录越界
            new_path = new_path->parent;
        } else {
        // 访问到不存在的文件夹
            if (new_path->content.find(token) == new_path->content.end() || std::get_if<Directory*>(&(new_path->content.at(token))) == nullptr) return false; 
            new_path = *std::get_if<Directory*>(&(new_path->content.at(token)));
        }
    }
    this->add_history(new Operation(this->cwd, order));
    cwd = new_path;
    return true;
}

bool FileSys::undo() {
    // TODO for bonus
    if (history.empty()) return false;
    Operation* op = history.top();
    if (op->type == "cd") {
        // 恢复旧的内容
        this->cwd = op->dir;
    } else if (op->type == "mkdir") {
        this->cwd->content.erase(op->name);
        this->cwd->seq_dict.erase(op->name);
    } else if (op->type == "create") {
        // 逆操作删掉文件
        this->cwd->remove_file(op->name, false);
    } else if (op->type == "append") {
        // 可以先删再建
        this->cwd->append(op->name, op->old_content, false, op->diff);
    } else if (op->type == "write") {
        this->cwd->write_bin(op->name, op->diff, op->old_content, false);
    } else if (op->type == "rm") {
        // 重建文件
        this->cwd->create_file(op->file_type, op->name, op->old_content, false);
    }
    this->history.pop();
    return true;
}

void FileSys::add_history(Operation* new_history) {
    this->history.push(new_history);
}

FileSys::FileSys() {
    // 构造函数
    this->root = new Directory(this);
    this->cwd = this->root;
}