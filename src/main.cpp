#include <iostream>
#include <thread>
#include <barrier>
#include <syncstream>
#include <array>
#include <vector>

using namespace std;

struct Config {
    static constexpr int nt = 4;
    static constexpr int kinds = 10;
    static constexpr int steps = 16;

    static constexpr array<pair<char,int>, kinds> work_amount{{
        {'a', 7}, {'b', 8}, {'c', 6},
        {'d', 7}, {'e', 6}, {'f', 8}, {'g', 6},
        {'h', 4}, {'i', 5}, {'j', 5}
    }};

    static constexpr array<const char*, steps> rows{{
        "abcb",
        "abca",
        "abcc",
        "abcb",
        "abca",
        "bddd",
        "eeff",
        "ffdd",
        "eeff",
        "ffgg",
        "gggg",
        "eedd",
        "hhhh",
        "iiii",
        "ijjj",
        "jj--"
    }};
};

struct Task {
    char act;
    int  idx;
};

array<array<Task, Config::nt>, Config::steps> build_plan() {
    array<array<Task, Config::nt>, Config::steps> plan{};
    int used[26] = {0};

    for (int s = 0; s < Config::steps; ++s) {
        const char* row = Config::rows[s];
        for (int t = 0; t < Config::nt; ++t) {
            char ch = row[t];
            if (ch == '-') {
                plan[s][t] = Task{'-', 0};
            } else {
                int idx = ++used[ch - 'a'];
                plan[s][t] = Task{ch, idx};
            }
        }
    }
    return plan;
}

void f(char x, int i) {
    osyncstream(cout) << "З набору " << x << " виконано дію " << i << "." << endl;
}

void worker(int tid, barrier<> &sync_point, const array<array<Task, Config::nt>, Config::steps> &plan) {
    for (int s = 0; s < Config::steps; ++s) {
        Task task = plan[s][tid];

        if (task.act != '-') {
            f(task.act, task.idx);
        }
        sync_point.arrive_and_wait();
    }
}

void start_calc() {
    auto plan = build_plan();

    barrier sync_point(Config::nt);

    vector<jthread> workers;
    workers.reserve(Config::nt);

    for (int tid = 0; tid < Config::nt; ++tid) {
        workers.emplace_back(worker, tid, ref(sync_point), ref(plan));
    }
}

int main() {
    osyncstream(cout) << "Обчислення розпочато." << endl;
    start_calc();
    osyncstream(cout) << "Обчислення завершено." << endl;
    return 0;
}