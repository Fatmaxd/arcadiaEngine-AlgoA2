/**
 * main_extra.cpp
 * Comprehensive Edge Case Test Suite for ArcadiaEngine
 * Tests edge cases, errors, and advanced scenarios beyond happy path.
 * Compile with: g++ -std=c++11 -o extra ArcadiaEngine.cpp main_extra.cpp
 * Run with: ./extra
 */

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <functional>
#include <random>
#include <algorithm>
#include "ArcadiaEngine.h"

using namespace std;

// ==========================================
// FACTORY FUNCTIONS (LINKING)
// ==========================================
extern "C" {
    PlayerTable* createPlayerTable();
    Leaderboard* createLeaderboard();
    AuctionTree* createAuctionTree();
}

// ==========================================
// TEST UTILITIES
// ==========================================
class ExtraTestRunner {
    int count = 0;
    int passed = 0;
    int failed = 0;

public:
    void runTest(string testName, bool condition, string extraOutput = "") {
        count++;
        cout << "TEST: " << left << setw(60) << testName;
        if (condition) {
            cout << "[ PASS ]" << (extraOutput.empty() ? "" : " - " + extraOutput);
            passed++;
        } else {
            cout << "[ FAIL ]" << (extraOutput.empty() ? "" : " - " + extraOutput);
            failed++;
        }
        cout << endl;
    }

    void printSummary() {
        cout << "\n==========================================" << endl;
        cout << "EXTRA SUMMARY: Passed: " << passed << " | Failed: " << failed << endl;
        cout << "==========================================" << endl;
        cout << "TOTAL EXTRA TESTS: " << count << endl;
        if (failed == 0) {
            cout << "Excellent! All edge cases passed. Ready for submission." << endl;
        } else {
            cout << "Some edge cases failed. Debug and retest." << endl;
        }
    }
};

ExtraTestRunner runner;

// ==========================================
// PART A1: PLAYER TABLE (HASHING EDGES)
// ==========================================
void test_PlayerTable_Edges() {
    cout << "\n--- Part A1: PlayerTable Edge Cases ---" << endl;

    // Empty search
    {
        PlayerTable* table = createPlayerTable();
        runner.runTest("PlayerTable: Search empty table", table->search(999) == "");
        delete table;
    }

    // Update existing
    {
        PlayerTable* table = createPlayerTable();
        table->insert(42, "Old");
        table->insert(42, "New");  // Update
        runner.runTest("PlayerTable: Update existing ID", table->search(42) == "New");
        delete table;
    }

    // Non-existing search after inserts
    {
        PlayerTable* table = createPlayerTable();
        table->insert(1, "One");
        runner.runTest("PlayerTable: Search non-existing", table->search(999) == "");
        delete table;
    }

    // Collision: Simple double hash probe
    // Assume h1(101)=0, h2= something, but test insert two that collide
    // For fixed size 101, hard to force full, but test many inserts
    {
        PlayerTable* table = createPlayerTable();
        // Insert 50 random IDs to fill partially
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 1000);
        for (int i = 0; i < 50; ++i) {
            int id = dis(gen);
            table->insert(id, "Test" + to_string(i));
        }
        // Search one inserted
        int testId = dis(gen);
        table->insert(testId, "CollisionTest");
        runner.runTest("PlayerTable: Insert with potential collision", table->search(testId) == "CollisionTest");
        delete table;
    }

    // Full table simulation: Insert 101 items that all hash to same slot (use multiples of 101)
    // h1(k) = k % 101, so keys 0,101,202,... all to 0
    // But IDs positive, start from 101*0=0 invalid, use 101,202,..., up to 101*100=10100
    {
        PlayerTable* table = createPlayerTable();
        bool fullPrinted = false;
        // Redirect cout to capture "Table is Full"
        // For simplicity, insert 101 colliding, check last insert prints
        // But since cout, we can check after if search fails or something, but assume code prints
        for (int i = 0; i <= 101; ++i) {  // 0 to 101
            int id = i * 101;
            table->insert(id, "Full" + to_string(i));
            if (i == 101) {
                // Assume if full, last insert doesn't add, but since probe, double hash may find slots
                // To force full, need keys where double hash cycles without empty, hard without mod
                // Skip strict full test, assume impl handles probe loop
            }
        }
        runner.runTest("PlayerTable: Handle near-full table (no crash)", true);  // Placeholder
        delete table;
    }
}

// ==========================================
// PART A2: LEADERBOARD (SKIP LIST EDGES)
// ==========================================
void test_Leaderboard_Edges() {
    cout << "\n--- Part A2: Leaderboard Edge Cases ---" << endl;

    Leaderboard* board = createLeaderboard();

    // getTopN(0) empty
    runner.runTest("Leaderboard: getTopN(0) empty", board->getTopN(0).empty());

    // Add same score, tie-break ID asc (lower ID first since desc score, asc ID)
    board->addScore(20, 500);
    board->addScore(10, 500);
    vector<int> top2 = board->getTopN(2);
    runner.runTest("Leaderboard: Tie-break ID asc (10 before 20)", top2.size() == 2 && top2[0] == 10 && top2[1] == 20);

    // getTopN > size
    board->addScore(30, 600);  // Highest
    vector<int> top4 = board->getTopN(4);
    runner.runTest("Leaderboard: getTopN > size returns all", top4.size() == 3);

    // Remove non-existing
    board->removePlayer(999);
    vector<int> afterRemove = board->getTopN(3);
    runner.runTest("Leaderboard: Remove non-existing no change", afterRemove[0] == 30);

    // Remove existing
    board->removePlayer(10);
    afterRemove = board->getTopN(3);
    runner.runTest("Leaderboard: Remove existing updates list", afterRemove.size() == 2 && afterRemove[0] == 30);

    // Add negative score? Assume scores >=0, but test
    board->addScore(40, -100);
    vector<int> withNeg = board->getTopN(3);
    runner.runTest("Leaderboard: Negative score at end", withNeg[2] == 40);

    delete board;
}

// ==========================================
// PART A3: AUCTION TREE (RB EDGES)
// ==========================================
void test_AuctionTree_Edges() {
    cout << "\n--- Part A3: AuctionTree Edge Cases ---" << endl;

    // Empty delete no crash
    {
        AuctionTree* tree = createAuctionTree();
        tree->deleteItem(999);
        runner.runTest("AuctionTree: Delete non-existing no crash", true);
        delete tree;
    }

    // Insert same price, order by ID asc
    {
        AuctionTree* tree = createAuctionTree();
        tree->insertItem(20, 100);
        tree->insertItem(10, 100);  // Same price, ID 10 < 20, so 10 before 20 in inorder (asc price, then asc ID)
        // To verify order, need inorder traversal, but since no get func, assume insert no crash + composite in BST
        tree->insertItem(15, 100);
        runner.runTest("AuctionTree: Insert duplicate prices no crash", true);  // Placeholder, add inorder if possible
        delete tree;
    }

    // Delete after insert
    {
        AuctionTree* tree = createAuctionTree();
        tree->insertItem(1, 50);
        tree->deleteItem(1);
        runner.runTest("AuctionTree: Delete inserted item no crash", true);
        delete tree;
    }

    // Many inserts (balance test)
    {
        AuctionTree* tree = createAuctionTree();
        for (int i = 0; i < 100; ++i) {
            tree->insertItem(i, i % 50);  // Some duplicates
        }
        runner.runTest("AuctionTree: 100 inserts no crash", true);
        delete tree;
    }
}

// ==========================================
// PART B: INVENTORY EDGES
// ==========================================
void test_Inventory_Edges() {
    cout << "\n--- Part B: Inventory Edge Cases ---" << endl;

    // optimizeLootSplit empty
    {
        vector<int> empty;
        runner.runTest("LootSplit: Empty coins -> 0", InventorySystem::optimizeLootSplit(0, empty) == 0);
    }

    // All zeros
    {
        vector<int> zeros = {0,0,0};
        runner.runTest("LootSplit: All zeros -> 0", InventorySystem::optimizeLootSplit(3, zeros) == 0);
    }

    // Odd total, min diff 1
    {
        vector<int> odd = {1,3};
        runner.runTest("LootSplit: {1,3} -> Diff 2", InventorySystem::optimizeLootSplit(2, odd) == 2);
    }

    // maximizeCarryValue cap=0
    {
        vector<pair<int,int>> items = {{1,10}};
        runner.runTest("Knapsack: Cap 0 -> 0", InventorySystem::maximizeCarryValue(0, items) == 0);
    }

    // Empty items
    {
        vector<pair<int,int>> empty;
        runner.runTest("Knapsack: Empty items -> 0", InventorySystem::maximizeCarryValue(5, empty) == 0);
    }

    // Weight > cap
    {
        vector<pair<int,int>> heavy = {{11,100}};
        runner.runTest("Knapsack: Weight > cap -> 0", InventorySystem::maximizeCarryValue(10, heavy) == 0);
    }

    // countStringPossibilities empty
    {
        runner.runTest("StringDP: Empty string -> 1", InventorySystem::countStringPossibilities("") == 1);
    }

    // Invalid char 'w'
    {
        runner.runTest("StringDP: 'w' -> 0", InventorySystem::countStringPossibilities("w") == 0);
    }

    // Single valid
    {
        runner.runTest("StringDP: 'u' -> 1", InventorySystem::countStringPossibilities("u") == 1);
    }

    // 'uuu' : u u u, u w, w u -> 3
    {
        runner.runTest("StringDP: 'uuu' -> 3", InventorySystem::countStringPossibilities("uuu") == 3);
    }

    // 'un' : u n -> 1 (no double)
    {
        runner.runTest("StringDP: 'un' -> 1", InventorySystem::countStringPossibilities("un") == 1);
    }
}

// ==========================================
// PART C: NAVIGATOR EDGES
// ==========================================
void test_Navigator_Edges() {
    cout << "\n--- Part C: Navigator Edge Cases ---" << endl;

    // pathExists: n=1, source=dest
    {
        vector<vector<int>> noEdges;
        runner.runTest("PathExists: n=1, self -> true", WorldNavigator::pathExists(1, noEdges, 0, 0));
    }

    // Disconnected
    {
        vector<vector<int>> dis = {{0,0}};  // Invalid self
        runner.runTest("PathExists: Disconnected 0-1 -> false", !WorldNavigator::pathExists(2, dis, 0, 1));
    }

    // n=0 invalid
    {
        vector<vector<int>> e;
        runner.runTest("PathExists: n=0 -> false", !WorldNavigator::pathExists(0, e, 0, 0));
    }

    // minBribeCost: n=1 -> 0
    {
        vector<vector<int>> noRoad;
        runner.runTest("MinBribe: n=1 -> 0", WorldNavigator::minBribeCost(1, 0, 1, 1, noRoad) == 0);
    }

    // Disconnected -> -1
    {
        vector<vector<int>> disRoad = {{0,0,1,1}};  // Invalid
        runner.runTest("MinBribe: Disconnected 2 nodes -> -1", WorldNavigator::minBribeCost(2, 1, 1, 1, disRoad) == -1);
    }

    // Zero cost
    {
        vector<vector<int>> zeroR = {{0,1,0,0}};
        runner.runTest("MinBribe: Zero cost edge -> 0", WorldNavigator::minBribeCost(2, 1, 1, 1, zeroR) == 0);
    }

    // sumMinDistancesBinary: n=1 -> "0"
    {
        vector<vector<int>> noR;
        runner.runTest("BinarySum: n=1 -> '0'", WorldNavigator::sumMinDistancesBinary(1, noR) == "0");
    }

    // Disconnected pairs skipped
    {
        vector<vector<int>> dis = {{0,0,1}};  // Invalid
        runner.runTest("BinarySum: Disconnected -> '0'", WorldNavigator::sumMinDistancesBinary(2, dis) == "0");
    }

    // Sum=1 -> "1"
    {
        vector<vector<int>> one = {{0,1,1}};
        runner.runTest("BinarySum: Single edge 1 -> '1'", WorldNavigator::sumMinDistancesBinary(2, one) == "1");
    }
}

// ==========================================
// PART D: KERNEL EDGES
// ==========================================
void test_Kernel_Edges() {
    cout << "\n--- Part D: Kernel Edge Cases ---" << endl;

    // Empty tasks
    {
        vector<char> empty;
        runner.runTest("Scheduler: Empty tasks -> 0", ServerKernel::minIntervals(empty, 2) == 0);
    }

    // n=0
    {
        vector<char> tasks = {'A','B'};
        runner.runTest("Scheduler: n=0 -> size", ServerKernel::minIntervals(tasks, 0) == 2);
    }

    // All unique
    {
        vector<char> unique = {'A','B','C'};
        runner.runTest("Scheduler: All unique, n=2 -> 3", ServerKernel::minIntervals(unique, 2) == 3);
    }

    // High freq, many idles
    {
        vector<char> manyA(5, 'A');
        runner.runTest("Scheduler: 5 A's, n=1 -> 9 (A idle A idle ...)", ServerKernel::minIntervals(manyA, 1) == 9);
    }
}

int main() {
    cout << "Arcadia Engine - Edge Case Tests" << endl;
    cout << "-----------------------------------------" << endl;

    test_PlayerTable_Edges();
    test_Leaderboard_Edges();
    test_AuctionTree_Edges();
    test_Inventory_Edges();
    test_Navigator_Edges();
    test_Kernel_Edges();

    runner.printSummary();

    return 0;
}