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
                std::cout << "Invalid operation" << std::endl;
        } else if (order == "ls") {
            std::cout << filesys.print_cwd(); // 已自带换行
        } else if (order == "cd") {
            std::string path;
            std::cin >> path;
            if (!filesys.switch_path(path))
                std::cout << "Invalid operation" << std::endl;
        }
    }
    return 0;
}