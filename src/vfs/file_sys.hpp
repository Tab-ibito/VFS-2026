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
    std::vector<bool> completed; // 打印tree时是否已经完成某一层遍历
    public:
    std::stack<Operation*> history;
    std::vector<Operation*> redo_list;
    Directory* cwd;
    Directory* root;
    bool switch_path(std::string& path, bool is_recorded, bool is_redo);
    void add_history(Operation* op);
    bool undo();
    bool redo();
    FileSys();
    ~FileSys(){};
    void print_tree(Directory* dir);
};

class Operation {
    public:
    Directory* dir;
    std::string name, type, file_type;
    bool is_end;
    int diff;
    std::string old_content;
    Directory* old_dir;
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
    Operation(Directory* dir, std::string& name, std::string& type, std::string& file_type, int diff, bool is_end, std::string& old_content) {
        this->dir = dir;
        this->name = name;
        this->type = type;
        this->file_type = file_type;
        this->diff = diff;
        this->is_end = is_end;
        this->old_content = old_content;
    }
    Operation(Directory* dir, std::string& name, std::string& type, Directory* old_dir){
        this->dir = dir;
        this->name = name;
        this->type = type;
        this->old_dir = old_dir;
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
            std::cout << this->content.at(i);
            cnt ++;
        }
        std::cout << '\n';
        return true;
    }
    void write_bin(int offset, std::string& hexdata) {
        int start = offset * 2;
        int end = start + hexdata.size();
        std::string buf = ""; // 设置隔离缓冲
        // 源数据有尾回填
        if (content.size() < end) content.resize(end, '0');
        // 复制覆盖
        std::copy(hexdata.begin(), hexdata.end(), content.begin() + start);
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
    
    public:
    FileSys* filesys; // 便于撤销操作回传
    bool root = false; // 是否是根目录
    std::string name; // 文件夹名
    Directory* parent; // 父文件夹指针
    std::unordered_map<std::string, std::variant<Directory*, File*>> content; // 子文件指针，键值是文件夹名
    std::set<std::string> seq_dict; // 按照字典序输出文件
    std::set<std::pair<std::string, int>, CompareSize> seq_size; // 按照字典序输出文件

    Directory(FileSys* filesys) {
        this->name = "";
        root = true;
        this->filesys = filesys;
    }

    Directory(std::string& name, Directory* parent, FileSys* filesys) {
        this->name = name;
        this->parent = parent;
        this->filesys = filesys;
    }

    bool is_valid_file(std::string& name) {
        return !(this->content.find(name) == this->content.end() || std::get_if<File*>(&(this->content.at(name))) == nullptr);
    }

    bool is_valid_dir(std::string& name) {
        return !(this->content.find(name) == this->content.end() || std::get_if<Directory*>(&(this->content.at(name))) == nullptr);
    }

    bool make_dir(std::string& name, bool is_recorded = true, bool is_redo = false) {
        // 创建子目录 mkdir
        std::string order = "mkdir";
        if (this->content.find(name) != this->content.end()) return false;
        Directory* new_dir = new Directory(name, this, this->filesys);
        this->content.insert({name, new_dir});
        if (is_recorded) {
            this->filesys->add_history(new Operation(this, name, order));
            if (!is_redo) this->filesys->redo_list.clear();
        }
        this->seq_dict.insert(name);
        return true;
    }

    bool create_file(std::string& type, std::string& name, std::string& content, bool is_recorded = true, bool is_redo = false) {
        // 创建文件
        std::string order = "create";
        if (this->content.find(name) != this->content.end()) return false;
        File* new_file = new File(type, name, content);
        this->content.insert({name, new_file});
        if (is_recorded) {
            this->filesys->add_history(new Operation(this, name, order));
            if (!is_redo) this->filesys->redo_list.clear();
        }
        this->seq_dict.insert(name);
        this->seq_size.insert({name, new_file->get_size()});
        return true;
    }

    bool append(std::string& name, std::string& content, bool is_recorded = true, int diff = 0, bool is_redo = false) {
        // 追加内容
        std::string order = "append";
        if (!is_valid_file(name)) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type == "bin") return false;
        this->seq_size.erase({name, file->get_size()});
        if(is_recorded) {
            this->filesys->add_history(new Operation(this, name, order, file->type, file->get_size()));
            if (!is_redo) this->filesys->redo_list.clear();
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

    bool write_bin(std::string& name, int offset, std::string& hexdata, bool is_recorded = true, bool is_end = false, bool is_redo = false) {
        // 写二进制文件内容
        std::string order = "write";
        if (!is_valid_file(name)) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (file->type != "bin") return false;
        int diff = offset < file->get_size() ? offset : file->get_size(); // 开始出现差异的字节位置
        int end = offset + hexdata.size() / 2; // 结束修改的字节位置
        std::string old_content = "";

        this->seq_size.erase({name, file->get_size()});
        if (is_recorded) {
            if (file->get_size() <= end){ // 扩展了文件的长度，那么原信息没有尾部，撤销时需要恢复尾部，is_end 为 true
                old_content = offset > file->get_size() ? "" : file->content.substr(offset*2);
                this->filesys->add_history(new Operation(this, name, order, file->type, diff, true, old_content));
            } else {
                old_content = offset > file->get_size() ? "" : file->content.substr(offset*2, hexdata.size());
                this->filesys->add_history(new Operation(this, name, order, file->type, diff, false, old_content));   
            }
            file->write_bin(offset, hexdata);
            if (!is_redo) this->filesys->redo_list.clear();
        } else {
            if (is_end) file->content.resize(offset*2 + hexdata.size()); 
            std::copy(hexdata.begin(), hexdata.end(), file->content.begin() + offset*2);
        } 
        this->seq_size.insert({name, file->get_size()});
        return true;
    }

    bool remove_file(std::string& name, bool is_recorded = true, bool is_redo = false) {
        std::string order = "rm";
        if (!is_valid_file(name)) return false;
        File* file = *std::get_if<File*>(&(this->content.at(name)));
        if (is_recorded){
            this->filesys->add_history(new Operation(this, name, order, file->type, file->content));
            if (!is_redo) this->filesys->redo_list.clear();
        } 
        this->seq_size.erase({name, file->get_size()});
        this->content.erase(name); // 清除content的文件名
        this->seq_dict.erase(name);
        return true;
    }

    bool remove_dir(std::string name, bool is_recorded = true, bool is_redo = false) {
        std::string order = "rm_dir";
        if (!is_valid_dir(name)) return false;
        Directory* dir = *std::get_if<Directory*>(&(this->content.at(name)));
        if (is_recorded) {
            this->filesys->add_history(new Operation(this, name, order, dir));
            if (!is_redo) this->filesys->redo_list.clear();
        }
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

bool FileSys::switch_path(std::string& path, bool is_recorded = true, bool is_redo = false) {
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
    if(is_recorded) {
        this->add_history(new Operation(this->cwd, order));
        if (!is_redo) this->redo_list.clear();
    }
    cwd = new_path;
    return true;
}

bool FileSys::undo() {
    // TODO for bonus
    if (history.empty()) return false;
    Operation* op = history.top();
    if (op->type == "cd") {
        // 恢复旧的内容
        redo_list.push_back(new Operation(this->cwd, op->type));
        this->cwd = op->dir;
    } else if (op->type == "mkdir") {
        redo_list.push_back(new Operation(this->cwd, op->name, op->type));
        this->cwd->content.erase(op->name);
        this->cwd->seq_dict.erase(op->name);
    } else if (op->type == "create") {
        // 逆操作删掉文件
        File* file = *std::get_if<File*>(&(this->cwd->content.at(op->name)));
        redo_list.push_back(new Operation(this->cwd, op->name, op->type, file->type, file->content));
        this->cwd->remove_file(op->name, false);
    } else if (op->type == "append") {
        // 可以先删再建
        File* file = *std::get_if<File*>(&(this->cwd->content.at(op->name)));
        std::string old_content = file->content.substr(op->diff);
        redo_list.push_back(new Operation(this->cwd, op->name, op->type, file->type, old_content));
        this->cwd->append(op->name, op->old_content, false, op->diff);
    } else if (op->type == "write") {
        File* file = *std::get_if<File*>(&(this->cwd->content.at(op->name)));
        std::string old_content = file->content.substr(op->diff * 2);
        redo_list.push_back(new Operation(this->cwd, op->name, op->type, file->type, op->diff, false, old_content)); // TODO: 需要优化
        this->cwd->write_bin(op->name, op->diff, op->old_content, false, op->is_end);
    } else if (op->type == "rm") {
        // 重建文件
        redo_list.push_back(new Operation(this->cwd, op->name, op->type));
        this->cwd->create_file(op->file_type, op->name, op->old_content, false);
    } else if (op->type == "rm_dir") {
        redo_list.push_back(new Operation(this->cwd, op->name, op->type));
        this->cwd->content.insert({op->name, op->old_dir});
        this->cwd->seq_dict.insert(op->name);
    }
    this->history.pop();
    return true;
}

bool FileSys::redo() {
    // 对于redo复原原本指令内容
    if (this->redo_list.empty()) return false;
    Operation* todo = this->redo_list.back();
    std::string op = this->redo_list.back()->type;
    this->redo_list.pop_back();
    if (op == "cd") {
        this->add_history(new Operation(this->cwd, op));
        this->cwd = todo->dir;
    } else if (op == "mkdir") {
        this->cwd->make_dir(todo->name, true, true);
    } else if (op == "create") {
        this->cwd->create_file(todo->file_type, todo->name, todo->old_content, true, true);
    } else if (op == "append") {
        this->cwd->append(todo->name, todo->old_content, true, 0, true);
    } else if (op == "write") {
        this->cwd->write_bin(todo->name, todo->diff, todo->old_content, true, false, true);
    } else if (op == "rm") {
        this->cwd->remove_file(todo->name, true, true);
    } else if (op == "rm_dir") {
        this->cwd->remove_dir(todo->name, true, true);
    }
    return true;
}

void FileSys::add_history(Operation* new_history) {
    this->history.push(new_history);
}

void FileSys::print_tree(Directory* dir) {
    const std::string branch = "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 "; // ├── 后面一个空格
    const std::string last_branch = "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 "; // └── 后面一个空格
    const std::string continuation = "\xE2\x94\x82   "; // │ 后面三个空格
    const std::string blank = "    "; // 四个空格
    if (dir == this->cwd) {
        this->completed.clear(); // 清空记录
        std::cout << dir->name << '/' << '\n';
    }
    this->completed.push_back(false);
    for (auto it = dir->seq_dict.begin(); it != dir->seq_dict.end(); it++) {
        for (auto status = this->completed.begin(); std::next(status) != this->completed.end(); status++) {
            if (!*status) std::cout << continuation;
            else std::cout << blank;
        }
        if (std::next(it) != dir->seq_dict.end()) {
            std::cout << branch;
        } else {
            std::cout << last_branch;
            this->completed.back() = true;
        }
        std::string name = *it;
        std::cout << name;
        if (!dir->is_valid_file(name)) {
            std::cout << '/';
            std::cout << '\n';
            Directory* next_dir = *std::get_if<Directory*>(&(dir->content.at(name)));
            print_tree(next_dir);
        } else std::cout << '\n';
    }
    this->completed.pop_back();
}

FileSys::FileSys() {
    // 构造函数
    this->root = new Directory(this);
    this->cwd = this->root;
}