#include <map>
#include <unordered_map>
#include <set>
#include <vector>
#include <stack>
#include <queue>
#include <deque>
#include <utility>
#include <optional>
#include <tuple>
#include <variant>
#include <algorithm>
#include <iostream>
#include "file_sys.hpp"

std::string get_content(bool is_binary = false) {
    char c;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string content;
    std::cin >> c;
    do {
        c = std::cin.get();
        if (!is_binary || c != ' ') content.push_back(c);
    } while (c != '"');
    content.pop_back();
    return content;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n;
    std::cin >> n;
    FileSys filesys;

    for (int i=0; i<n; i++) {
        std::string order;
        std::cin >> order;
        if (order == "mkdir") {
            std::string name;
            std::cin >> name;
            if (!filesys.cwd->make_dir(name))
                std::cout << "Invalid operation" << '\n';
        } else if (order == "ls") {
            filesys.cwd->print_dir(); // 已自带换行
        } else if (order == "cd") {
            std::string path;
            std::cin >> path;
            if (!filesys.switch_path(path))
                std::cout << "Invalid operation" << '\n';
        } else if (order == "create") {
            std::string type, name, content;
            std::cin >> type >> name;
            content = get_content(type == "bin");
            if (!filesys.cwd->create_file(type, name, content))
                std::cout << "Invalid operation" << '\n';
        } else if (order == "append") {
            std::string name, content;
            std::cin >> name;
            content = get_content();
            if(!filesys.cwd->append(name, content))
                std::cout << "Invalid operation" << '\n';
        } else if (order == "cat") {
            std::string name;
            std::cin >> name;
            if (!filesys.cwd->output(name))
                std::cout << "Invalid operation" << '\n';
        } else if (order == "read") {
            std::string name;
            int length, offset;
            std::cin >> name;
            std::cin >> offset >> length;
            if (!filesys.cwd->read_bin(name, offset, length))
                std::cout << "Invalid operation" << '\n';
        } else if (order == "write") {
            std::string name, hexdata;
            int offset;
            std::cin >> name >> offset;
            hexdata = get_content(true);
            if (!filesys.cwd->write_bin(name, offset, hexdata))
                std::cout << "Invalid operation" << '\n';
        } else if (order == "rm") {
            std::string name;
            std::cin >> name;
            if (!filesys.cwd->remove_file(name))
                std::cout << "Invalid operation" << '\n';
        } else if (order == "stat") {
            std::string name;
            std::cin >> name;
            if (!filesys.cwd->get_stat(name))
                std::cout << "Invalid operation" << '\n';
        } else if (order == "top") {
            int k;
            std::cin >> k;
            filesys.cwd->print_topk(k);
        } else if (order == "undo") {
            if (!filesys.undo())
                std::cout << "Invalid operation" << '\n';
        } else if (order == "r") {
            std::cout << "---" <<std::endl;
        }
    }
    return 0;
}