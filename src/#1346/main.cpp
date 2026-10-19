#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>

using namespace std;

struct StudentInfo {
    string name;
    string gender;
    int cls, ranking, avg_score;
    vector<int> scores;
    bool operator<(const StudentInfo& right) const {
        if (this->avg_score > right.avg_score) return true;
        if (this->avg_score < right.avg_score) return false;
        for (int i=0; i<=8; i++) {
            if (this->scores[i] > right.scores[i]) return true;
            if (this->scores[i] < right.scores[i]) return false;
        }
        if (this->name < right.name) return true;
        else return false;
    }
};

unordered_map<string, StudentInfo> m;
struct Compare {
    bool operator()(const string& a, const string& b) const {
        return m[a] < m[b];
    }
};

set<string, Compare> ranking;
vector<string> published;

int main() {
    string op;
    bool start = false;
    while (true) {
        cin >> op;
        if (op == "START") {
            start = true;
            int i=1;
            for (auto it = ranking.begin(); it != ranking.end(); it++) {
                published[i-1] = *it;
                m[*it].ranking = i;
                i++;
            }
        }
        else if (op == "END") break;
        else if (op == "UPDATE") {
            string name;
            int code, score;
            cin >> name >> code >> score;
            
            auto tgt = m.find(name);
            if (tgt != m.end()) {
                ranking.erase(name);
                tgt->second.scores[code] = score;
                int total = 0;
                for (int i=0; i<=8; i++) {
                    total += tgt->second.scores[i];
                }
                tgt->second.avg_score = total / 9;
                ranking.insert(name);
            }
            else cout << "[Error]Update failed." << '\n';
        }
        else if (op == "QUERY") {
            string name;
            cin >> name;
            auto tgt = m.find(name);
            if (tgt != m.end()) cout << "STUDENT " << name << " NOW AT RANKING " << tgt->second.ranking << '\n';
            else cout << "[Error]Query failed." << '\n';
        }
        else if (op == "ADD") {
            if (start) {
                cout << "[Error]Cannot add student now." << '\n';
                continue;
            }
            StudentInfo new_info;
            cin >> new_info.name;
            if (m.find(new_info.name) != m.end()) {
                cout << "[Error]Add failed." << '\n';
                continue;
            }
            char gender;
            cin >> gender;
            if (gender == 'F') new_info.gender = "female";
            else if (gender == 'M') new_info.gender = "male";
            cin >> new_info.cls;
            int total = 0;
            for (int i=0; i<=8; i++) {
                int score;
                cin >> score;
                total += score;
                new_info.scores.push_back(score);
                new_info.avg_score = total / 9;
            }
            new_info.ranking = ranking.size() + 1;
            m.insert({new_info.name, new_info});
            ranking.insert(new_info.name);
            published.push_back(new_info.name);
        }
        else if (op == "FLUSH") {
            int i=1;
            for (auto it = ranking.begin(); it != ranking.end(); it++) {
                published[i-1] = *it;
                m[*it].ranking = i;
                i++;
            }
        }
        else if (op == "PRINTLIST") {
            int i=1;
            for (auto name : published) {
                cout << i << " " << m[name].name << " " << m[name].gender << " " << m[name].cls << " " << m[name].avg_score << " " << '\n';
                i++;
            }
        }
    }
    return 0;
}

// 问题：每次改操作只改单个，相比来说 log(n) 的堆操作更加适合这样的单个原子操作，一次flush的 nlogn 排序开销太大
// ranking需要在内部外部都维护更新，方便及时改查
// 自定义运算符重载，更改 Set 的排序规则