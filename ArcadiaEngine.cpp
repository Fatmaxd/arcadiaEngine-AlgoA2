// ArcadiaEngine.cpp - STUDENT TEMPLATE
// TODO: Implement all the functions below according to the assignment requirements

#include "ArcadiaEngine.h"
#include <algorithm>
#include <queue>
#include <numeric>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <functional>
#include <string>
#include <iostream>
#include <map>
#include <set>

using namespace std;


struct Key {
    int negScore;
    int id;
    Key(int ns, int i) : negScore(ns), id(i) {}
    bool operator<(const Key& other) const {
        if (negScore != other.negScore) return negScore < other.negScore;
        return id < other.id;
    }
    bool operator==(const Key& other) const {
        return negScore == other.negScore && id == other.id;
    }
};

struct Node {
    int id;
    int score;
    int height;
    Node* forward[16];
    Node(int i = -1, int s = INT_MAX) : id(i), score(s), height(0) {
        std::fill(forward, forward + 16, nullptr);
    }
};



// =========================================================
// PART A: DATA STRUCTURES (Concrete Implementations)
// =========================================================

// --- 1. PlayerTable (Double Hashing) ---

class ConcretePlayerTable : public PlayerTable {
private:
    // TODO: Define your data structures here
    // Hint: You'll need a hash table with double hashing collision resolution

public:
    ConcretePlayerTable() {
        // TODO: Initialize your hash table
    }

    void insert(int playerID, string name) override {
        // TODO: Implement double hashing insert
        // Remember to handle collisions using h1(key) + i * h2(key)
    }

    string search(int playerID) override {
        // TODO: Implement double hashing search
        // Return "" if player not found
        return "";
    }
};

// --- 2. Leaderboard (Skip List) ---

class ConcreteLeaderboard : public Leaderboard {
private:
    Node* head;
    int currentMaxLevel;
    static const int MAX_LEVEL = 16;

    Key getKey(Node* node) {
        return Key(-node->score, node->id);
    }

    int randomLevel() {
        int level = 1;
        while (level < MAX_LEVEL && (rand() % 2 == 0)) {
            level++;
        }
        return level;
    }

    Node* findNodeById(int playerID) {
        Node* curr = head->forward[0];
        while (curr) {
            if (curr->id == playerID) {
                return curr;
            }
            curr = curr->forward[0];
        }
        return nullptr;
    }

    void deleteNode(Node* nodeToDelete) {
        if (!nodeToDelete) return;

        Key targetKey = getKey(nodeToDelete);

        std::vector<Node*> pred(MAX_LEVEL + 1, nullptr);
        Node* current = head;
        int searchLevel = currentMaxLevel;

        for (int lvl = searchLevel; lvl >= 0; --lvl) {
            while (current->forward[lvl] && getKey(current->forward[lvl]) < targetKey) {
                current = current->forward[lvl];
            }
            pred[lvl] = current;
        }

        // Verify it's found at level 0
        if (pred[0]->forward[0] != nodeToDelete) {
            return; // Should not happen
        }

        // Perform deletion up to the node's height
        for (int lvl = 0; lvl <= nodeToDelete->height; ++lvl) {
            pred[lvl]->forward[lvl] = nodeToDelete->forward[lvl];
        }

        delete nodeToDelete;
    }

    void insertNew(int playerID, int score) {
        Key targetKey(-score, playerID);

        std::vector<Node*> pred(MAX_LEVEL + 1, nullptr);
        Node* current = head;
        int searchLevel = currentMaxLevel;

        for (int lvl = searchLevel; lvl >= 0; --lvl) {
            while (current->forward[lvl] && getKey(current->forward[lvl]) < targetKey) {
                current = current->forward[lvl];
            }
            pred[lvl] = current;
        }

        Node* nextNode = pred[0]->forward[0];
        if (nextNode && getKey(nextNode) == targetKey) {
            nextNode->score = score;
            return;
        }

        int newLevel = randomLevel();
        if (newLevel > currentMaxLevel) {
            for (int lvl = currentMaxLevel + 1; lvl < newLevel; ++lvl) {
                pred[lvl] = head;
            }
            currentMaxLevel = newLevel;
        }

        Node* newNode = new Node(playerID, score);
        newNode->height = newLevel;

        for (int lvl = 0; lvl < newLevel; ++lvl) {
            newNode->forward[lvl] = pred[lvl]->forward[lvl];
            pred[lvl]->forward[lvl] = newNode;
        }
    }

public:
    ConcreteLeaderboard() {
        head = new Node(-1, INT_MAX);
        currentMaxLevel = 0;
    }

    ~ConcreteLeaderboard() {
        Node* curr = head->forward[0];
        while (curr) {
            Node* next = curr->forward[0];
            delete curr;
            curr = next;
        }
        delete head;
    }

    void addScore(int playerID, int score) override {
        Node* existing = findNodeById(playerID);
        if (existing) {
            deleteNode(existing);
        }
        insertNew(playerID, score);
    }

    void removePlayer(int playerID) override {
        Node* toDelete = findNodeById(playerID);
        if (toDelete) {
            deleteNode(toDelete);
        }
    }

    vector<int> getTopN(int n) override {
        std::vector<int> topPlayers;
        Node* curr = head->forward[0];
        while (curr && topPlayers.size() < static_cast<size_t>(n)) {
            topPlayers.push_back(curr->id);
            curr = curr->forward[0];
        }
        return topPlayers;
    }
};

// --- 3. AuctionTree (Red-Black Tree) ---

class ConcreteAuctionTree : public AuctionTree {
private:
    // TODO: Define your Red-Black Tree node structure
    // Hint: Each node needs: id, price, color, left, right, parent pointers

public:
    ConcreteAuctionTree() {
        // TODO: Initialize your Red-Black Tree
    }

    void insertItem(int itemID, int price) override {
        // TODO: Implement Red-Black Tree insertion
        // Remember to maintain RB-Tree properties with rotations and recoloring
    }

    void deleteItem(int itemID) override {
        // TODO: Implement Red-Black Tree deletion
        // This is complex - handle all cases carefully
    }
};

// =========================================================
// PART B: INVENTORY SYSTEM (Dynamic Programming)
// =========================================================

int InventorySystem::optimizeLootSplit(int n, vector<int>& coins) {
    // TODO: Implement partition problem using DP
    // Goal: Minimize |sum(subset1) - sum(subset2)|
    // Hint: Use subset sum DP to find closest sum to total/2
    return 0;
}

int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int>>& items) {
    // TODO: Implement 0/1 Knapsack using DP
    // items = {weight, value} pairs
    // Return maximum value achievable within capacity
    return 0;
}

long long InventorySystem::countStringPossibilities(string s) {
    // TODO: Implement string decoding DP
    // Rules: "uu" can be decoded as "w" or "uu"
    //        "nn" can be decoded as "m" or "nn"
    // Count total possible decodings
    return 0;
}

// =========================================================
// PART C: WORLD NAVIGATOR (Graphs)
// =========================================================

bool WorldNavigator::pathExists(int n, vector<vector<int>>& edges, int source, int dest) {
    // Simple BFS over an undirected graph
    if (n <= 0) return false;
    if (source < 0 || source >= n || dest < 0 || dest >= n) return false;
    if (source == dest) return true;

    vector<vector<int>> adj(n);
    for (const auto& e : edges) {
        if (e.size() < 2) continue;
        int u = e[0], v = e[1];
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    vector<bool> visited(n, false);
    queue<int> q;
    visited[source] = true;
    q.push(source);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (u == dest) return true;
        for (int v : adj[u]) {
            if (!visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }

    return false;
}

long long WorldNavigator::minBribeCost(int n, int m, long long goldRate, long long silverRate,
                                       vector<vector<int>>& roadData) {
    // Kruskal's algorithm for MST on an undirected graph
    if (n <= 0) return 0;

    struct Edge {
        int u, v;
        long long w;
    };

    vector<Edge> edges;
    edges.reserve(roadData.size());
    for (const auto& rd : roadData) {
        if (rd.size() < 4) continue;
        int u = rd[0], v = rd[1];
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        long long goldCost = rd[2];
        long long silverCost = rd[3];
        long long w = goldCost * goldRate + silverCost * silverRate;
        edges.push_back({u, v, w});
    }

    if ((int)edges.size() < n - 1) {
        // Not enough edges to possibly connect all cities
        // still need to check connectivity in case of multi-edges, but size check is a quick fail
    }

    sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.w < b.w;
    });

    // Disjoint Set Union (Union-Find)
    vector<int> parent(n), rankv(n, 0);
    iota(parent.begin(), parent.end(), 0);

    function<int(int)> find = [&](int x) -> int {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    };

    auto unite = [&](int a, int b) -> bool {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (rankv[a] < rankv[b]) swap(a, b);
        parent[b] = a;
        if (rankv[a] == rankv[b]) rankv[a]++;
        return true;
    };

    long long totalCost = 0;
    int usedEdges = 0;

    for (const auto& e : edges) {
        if (unite(e.u, e.v)) {
            totalCost += e.w;
            usedEdges++;
            if (usedEdges == n - 1) break;
        }
    }

    if (usedEdges != n - 1) {
        // Graph is not fully connected
        return -1;
    }

    return totalCost;
}

string WorldNavigator::sumMinDistancesBinary(int n, vector<vector<int>>& roads) {
    if (n <= 0) return "0";

    const long long INF = (long long)4e18;
    vector<vector<long long>> dist(n, vector<long long>(n, INF));
    for (int i = 0; i < n; ++i) dist[i][i] = 0;

    // Undirected edges
    for (const auto& r : roads) {
        if (r.size() < 3) continue;
        int u = r[0], v = r[1];
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        long long w = r[2];
        if (w < dist[u][v]) {
            dist[u][v] = w;
            dist[v][u] = w;
        }
    }

    // Floyd–Warshall
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < n; ++j) {
                if (dist[k][j] == INF) continue;
                long long nd = dist[i][k] + dist[k][j];
                if (nd < dist[i][j]) {
                    dist[i][j] = nd;
                }
            }
        }
    }

    long long sum = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (dist[i][j] != INF) {
                sum += dist[i][j];
            }
        }
    }

    if (sum == 0) return "0";

    string binary;
    while (sum > 0) {
        binary.push_back(char('0' + (sum & 1LL)));
        sum >>= 1LL;
    }
    reverse(binary.begin(), binary.end());
    return binary;
}

// =========================================================
// PART D: SERVER KERNEL (Greedy)
// =========================================================

int ServerKernel::minIntervals(vector<char>& tasks, int n) {
    // TODO: Implement task scheduler with cooling time
    // Same task must wait 'n' intervals before running again
    // Return minimum total intervals needed (including idle time)
    // Hint: Use greedy approach with frequency counting
    return 0;
}

// =========================================================
// FACTORY FUNCTIONS (Required for Testing)
// =========================================================

extern "C" {
    PlayerTable* createPlayerTable() { 
        return new ConcretePlayerTable(); 
    }

    Leaderboard* createLeaderboard() { 
        return new ConcreteLeaderboard(); 
    }

    AuctionTree* createAuctionTree() { 
        return new ConcreteAuctionTree(); 
    }
}
