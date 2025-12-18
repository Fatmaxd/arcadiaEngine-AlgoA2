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
    static const int TABLE_SIZE = 101;

    struct Entry {
        int playerID;
        string name;
        bool occupied;

        Entry() : playerID(-1), name(""), occupied(false) {}
    };

    vector<Entry> table;

    int h1(int key) const {
        return key % TABLE_SIZE;
    }

    int h2(int key) const {
        return 1 + (key % (TABLE_SIZE - 1));
    }
public:
    ConcretePlayerTable() : table(TABLE_SIZE) {
    }

    void insert(int playerID, string name) override {
        int index = h1(playerID);
        int step = h2(playerID);

        int baseIndex = index;

        for (int i = 0; i < TABLE_SIZE; i++) {

            if (!table[index].occupied) {
                table[index].playerID = playerID;
                table[index].name = name;
                table[index].occupied = true;
                return;
            }
            if (table[index].occupied && table[index].playerID == playerID) {
                table[index].name = name;
                return;
            }

            index = (baseIndex + i * step) % TABLE_SIZE;
        }

        cout << "Table is Full" << endl;
    }

    string search(int playerID) override {
        int index = h1(playerID);
        int step = h2(playerID);

        int baseIndex = index;

        for (int i = 0; i < TABLE_SIZE; i++) {

            if (!table[index].occupied)
                return "";

            if (table[index].playerID == playerID)
                return table[index].name;

            index = (baseIndex + i * step) % TABLE_SIZE;
        }
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
    enum Color { RED, BLACK };

    struct RBNode {
        int itemID;
        int price;
        Color color;
        RBNode* left;
        RBNode* right;
        RBNode* parent;

        RBNode(int id, int p) : itemID(id), price(p), color(RED), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    RBNode* root;
    int size;

    // Helper to get uncle
    RBNode* getUncle(RBNode* node) {
        RBNode* parent = node->parent;
        if (!parent || !parent->parent) return nullptr;
        if (parent == parent->parent->left) {
            return parent->parent->right;
        } else {
            return parent->parent->left;
        }
    }

    // Rotations
    void rotateLeft(RBNode* node) {
        RBNode* parent = node->parent;
        RBNode* rightChild = node->right;
        node->right = rightChild->left;
        if (rightChild->left) rightChild->left->parent = node;
        rightChild->parent = parent;
        if (!parent) {
            root = rightChild;
        } else if (node == parent->left) {
            parent->left = rightChild;
        } else {
            parent->right = rightChild;
        }
        rightChild->left = node;
        node->parent = rightChild;
    }

    void rotateRight(RBNode* node) {
        RBNode* parent = node->parent;
        RBNode* leftChild = node->left;
        node->left = leftChild->right;
        if (leftChild->right) leftChild->right->parent = node;
        leftChild->parent = parent;
        if (!parent) {
            root = leftChild;
        } else if (node == parent->left) {
            parent->left = leftChild;
        } else {
            parent->right = leftChild;
        }
        leftChild->right = node;
        node->parent = leftChild;
    }

    // Fix insert violations
    void fixInsert(RBNode* node) {
        while (node->parent && node->parent->color == RED) {
            RBNode* uncle = getUncle(node);
            RBNode* parent = node->parent;
            RBNode* grandparent = parent->parent;

            if (uncle && uncle->color == RED) {
                parent->color = BLACK;
                uncle->color = BLACK;
                grandparent->color = RED;
                node = grandparent;
            } else {
                if (parent == grandparent->left) {
                    if (node == parent->right) {
                        rotateLeft(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    parent->color = BLACK;
                    grandparent->color = RED;
                    rotateRight(grandparent);
                } else {
                    if (node == parent->left) {
                        rotateRight(parent);
                        node = parent;
                        parent = node->parent;
                    }
                    parent->color = BLACK;
                    grandparent->color = RED;
                    rotateLeft(grandparent);
                }
            }
        }
        root->color = BLACK;
    }

    // BST insert without fix
    RBNode* insertBST(RBNode* node, int id, int price) {
        if (!node) {
            return new RBNode(id, price);
        }

        // Composite key: price asc, then id asc
        if (price < node->price || (price == node->price && id < node->itemID)) {
            node->left = insertBST(node->left, id, price);
            node->left->parent = node;
        } else if (price > node->price || (price == node->price && id > node->itemID)) {
            node->right = insertBST(node->right, id, price);
            node->right->parent = node;
        } else {
            // Duplicate key: update price if same id? But ids unique, assume no duplicate id
            node->price = price;
            return node;
        }
        return node;
    }

    // Find min in subtree
    RBNode* findMin(RBNode* node) {
        while (node && node->left) node = node->left;
        return node;
    }

    // Transplant
    void transplant(RBNode* u, RBNode* v) {
        if (!u->parent) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        if (v) v->parent = u->parent;
    }

    // Fix delete violations
    void fixDelete(RBNode* x) {
        while (x != root && (x == nullptr || x->color == BLACK)) {
            if (x == x->parent->left) {
                RBNode* sibling = x->parent->right;
                if (sibling->color == RED) {
                    sibling->color = BLACK;
                    x->parent->color = RED;
                    rotateLeft(x->parent);
                    sibling = x->parent->right;
                }
                if ((sibling->left == nullptr || sibling->left->color == BLACK) &&
                    (sibling->right == nullptr || sibling->right->color == BLACK)) {
                    sibling->color = RED;
                    x = x->parent;
                } else {
                    if (sibling->right == nullptr || sibling->right->color == BLACK) {
                        sibling->left->color = BLACK;
                        sibling->color = RED;
                        rotateRight(sibling);
                        sibling = x->parent->right;
                    }
                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    sibling->right->color = BLACK;
                    rotateLeft(x->parent);
                    x = root;
                }
            } else {
                RBNode* sibling = x->parent->left;
                if (sibling->color == RED) {
                    sibling->color = BLACK;
                    x->parent->color = RED;
                    rotateRight(x->parent);
                    sibling = x->parent->left;
                }
                if ((sibling->right == nullptr || sibling->right->color == BLACK) &&
                    (sibling->left == nullptr || sibling->left->color == BLACK)) {
                    sibling->color = RED;
                    x = x->parent;
                } else {
                    if (sibling->left == nullptr || sibling->left->color == BLACK) {
                        sibling->right->color = BLACK;
                        sibling->color = RED;
                        rotateLeft(sibling);
                        sibling = x->parent->left;
                    }
                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    sibling->left->color = BLACK;
                    rotateRight(x->parent);
                    x = root;
                }
            }
        }
        if (x) x->color = BLACK;
    }

    // Delete node by pointer
    void deleteNode(RBNode* z) {
        RBNode* y = z;
        Color yOriginalColor = y->color;
        RBNode* x;

        if (!z->left) {
            x = z->right;
            transplant(z, z->right);
        } else if (!z->right) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = findMin(z->right);
            yOriginalColor = y->color;
            x = y->right;
            if (y->parent == z) {
                if (x) x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        if (yOriginalColor == BLACK) {
            fixDelete(x);
        }
        delete z;
    }

    // Linear search for node by ID
    RBNode* findByID(RBNode* node, int id) {
        if (!node) return nullptr;
        RBNode* leftRes = findByID(node->left, id);
        if (leftRes) return leftRes;
        if (node->itemID == id) return node;
        return findByID(node->right, id);
    }

public:
    ConcreteAuctionTree() : root(nullptr), size(0) {}

    ~ConcreteAuctionTree() {
        // Inorder deletion
        std::function<void(RBNode*)> del = [&](RBNode* node) {
            if (!node) return;
            del(node->left);
            del(node->right);
            delete node;
        };
        del(root);
    }

    void insertItem(int itemID, int price) override {
        root = insertBST(root, itemID, price);
        fixInsert(root);
        size++;
    }

    void deleteItem(int itemID) override {
        RBNode* node = findByID(root, itemID);
        if (node) {
            deleteNode(node);
            size--;
        }
    }
};

// =========================================================
// PART B: INVENTORY SYSTEM (Dynamic Programming)
// =========================================================

int InventorySystem::optimizeLootSplit(int n, vector<int>& coins) {
    // Partition problem: min |s1 - s2| = min |2*s1 - total| where s1 closest to total/2
    int total = 0;
    for (int c : coins) total += c;
    int half = total / 2;

    // DP: can we achieve sum j with first i items?
    vector<bool> dp(half + 1, false);
    dp[0] = true;

    for (int coin : coins) {
        for (int j = half; j >= coin; --j) {
            if (dp[j - coin]) dp[j] = true;
        }
    }

    // Find largest achievable <= half
    int closest = 0;
    for (int j = half; j >= 0; --j) {
        if (dp[j]) {
            closest = j;
            break;
        }
    }

    return total - 2 * closest;
}

int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int>>& items) {
    // 0/1 Knapsack: max value
    int m = items.size();
    vector<vector<int>> dp(m + 1, vector<int>(capacity + 1, 0));

    for (int i = 1; i <= m; ++i) {
        int w = items[i-1].first;
        int v = items[i-1].second;
        for (int c = 0; c <= capacity; ++c) {
            dp[i][c] = dp[i-1][c];
            if (c >= w) {
                dp[i][c] = max(dp[i][c], dp[i-1][c - w] + v);
            }
        }
    }

    return dp[m][capacity];
}

long long InventorySystem::countStringPossibilities(string s) {
    // Decode ways: 'u'/'n' single, "uu"->w, "nn"->m
    // If 'w' or 'm' present, 0
    // Empty: 1
    int len = s.length();
    if (len == 0) return 1;
    for (char c : s) {
        if (c == 'w' || c == 'm') return 0;
    }

    vector<long long> dp(len + 1, 0);
    dp[0] = 1;
    dp[1] = (s[0] == 'u' || s[0] == 'n') ? 1 : 0;

    for (int i = 2; i <= len; ++i) {
        // Single
        if (s[i-1] == 'u' || s[i-1] == 'n') {
            dp[i] += dp[i-1];
        }
        // Double
        if (i >= 2) {
            string sub = s.substr(i-2, 2);
            if (sub == "uu" || sub == "nn") {
                dp[i] += dp[i-2];
            }
        }
    }

    return dp[len];
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
    if (tasks.empty()) return 0;
    if (n == 0) return tasks.size();

    map<char, int> freq;
    for (char t : tasks) freq[t]++;

    int maxFreq = 0;
    int countMax = 0;
    for (auto& p : freq) {
        if (p.second > maxFreq) {
            maxFreq = p.second;
            countMax = 1;
        } else if (p.second == maxFreq) {
            countMax++;
        }
    }

    int res = (maxFreq - 1) * (n + 1) + countMax;
    return max(res, (int)tasks.size());
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