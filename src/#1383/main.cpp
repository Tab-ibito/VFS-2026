#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
using namespace std;

// struct Activity {
//     int avg, level;
//     string str;
//     bool operator>(const Activity& right) {
//         if (this->avg > right.avg) return true;
//         if (this->avg == right.avg && this->level > right->level) return true;
//         if (this->level == right.level && this->str < right.str) return true; 
//         return false;
//     }
// };

int main () {
    string op;
    map<string, vector<int>> m;
    // set<Activity> s;

    while (true) {
        cin >> op;
        if (op == "end") return 0;
        string order, param;
        cin >> order;

        if (op == "map") {
            if (order == "size") {
                cout << m.size() << endl;
            } else if (order == "insert") {
                string name;
                int num;
                cin >> name;
                cin >> num;
                for (int i=0; i<num; i++) {
                    int k;
                    cin >> k;
                    m[name].push_back(k);
                }
            } else if (order == "erase") {
                string name;
                cin >> name;
                auto tgt = m.find(name);
                if (tgt != m.end()) m.erase(name);
                else cout << "Error" << endl;
            } else if (order == "count") {
                string name;
                cin >> name;
                auto tgt = m.find(name);
                if (tgt != m.end()) cout << 1 << endl;
                else cout << 0 << endl;
            } else if (order == "traverse") {
                for (auto& [key, vec] : m) {
                    cout << key << " ";
                    for (auto& num : vec) {
                        cout << num << " ";
                    }
                    cout << endl;
                }
            } else if (order == "visitVec") {
                string name;
                int index;
                cin >> name >> index;
                auto tgt = m.find(name);
                if (tgt != m.end()) cout << tgt->second[index] << endl; 
                else cout << "Error" << endl;
            } else if (order == "visit") {
                string name;
                cin >> name;
                auto tgt = m.find(name);
                if (tgt != m.end()) {
                    cout << tgt->second.size() << " ";
                    for (auto& num : tgt->second) {
                        cout << num << " ";
                    }
                    cout << endl;
                }
                else cout << "Error" << endl;
            } else if (order == "append"){
                string name;
                int elem;
                cin >> name >> elem;
                auto tgt = m.find(name);
                if (tgt != m.end()) tgt->second.push_back(elem);
                else cout << "Error" << endl;
            }
        } else if (op == "set") {

        }
    }
    return 0;
}