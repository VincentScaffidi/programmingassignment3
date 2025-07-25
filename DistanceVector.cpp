#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

const int INF = 999999;

int main() {
    map<string, map<string, int>> links;
    vector<string> routers;
    map<string, map<string, map<string, int>>> D;
    string line;
    
    // Read router names
    while (getline(cin, line)) {
        if (line == "START") break;
        if (!line.empty()) {
            routers.push_back(line);
        }
    }
    sort(routers.begin(), routers.end());
    
    // Read initial topology
    while (getline(cin, line) && line != "UPDATE") {
        istringstream iss(line);
        string r1, r2;
        int cost;
        if (iss >> r1 >> r2 >> cost) {
            if (cost == -1) {
                links[r1].erase(r2);
                links[r2].erase(r1);
            } else {
                links[r1][r2] = cost;
                links[r2][r1] = cost;
            }
        }
    }
    
    // Initialize distance tables
    for (const string& router : routers) {
        for (const string& dest : routers) {
            if (dest == router) continue;
            for (const string& via : routers) {
                if (via == router) continue;
                D[router][dest][via] = INF;
            }
            D[router][dest][dest] = links[router].count(dest) ? links[router][dest] : INF;
        }
    }
    
    // Function to print distance tables
    auto printDistanceTables = [&](int step) {
        for (const string& router : routers) {
            cout << "Distance Table of router " << router << " at t=" << step << ":" << endl;
            cout << "     ";
            for (const string& dest : routers) {
                if (dest != router) cout << dest << "    ";
            }
            cout << endl;
            
            for (const string& via : routers) {
                if (via == router) continue;
                cout << via << "    ";
                for (const string& dest : routers) {
                    if (dest == router) continue;
                    int cost = D[router][dest][via];
                    if (cost == INF) cout << "INF  ";
                    else cout << cost << "    ";
                }
                cout << endl;
            }
            cout << endl;
        }
    };
    
    // Function to print routing tables
    auto printRoutingTables = [&]() {
        for (const string& router : routers) {
            cout << "Routing Table of router " << router << ":" << endl;
            
            for (const string& dest : routers) {
                if (dest == router) continue;
                
                int minCost = INF;
                string nextHop = "";
                
                vector<string> neighbors;
                for (const string& via : routers) {
                    if (via != router && links[router].count(via)) {
                        neighbors.push_back(via);
                    }
                }
                sort(neighbors.begin(), neighbors.end());
                
                for (const string& via : neighbors) {
                    int cost = D[router][dest][via];
                    if (cost < minCost || (cost == minCost && via < nextHop)) {
                        minCost = cost;
                        nextHop = via;
                    }
                }
                
                if (minCost == INF) {
                    cout << dest << ",INF,INF" << endl;
                } else {
                    cout << dest << "," << nextHop << "," << minCost << endl;
                }
            }
            cout << endl;
        }
    };
    
    // Print initial tables
    printDistanceTables(0);
    
    // Run Distance Vector algorithm
    int step = 0;
    bool changed = true;
    
    while (changed) {
        changed = false;
        step++;
        
        // Calculate new distance vectors
        map<string, map<string, int>> newD;
        for (const string& router : routers) {
            for (const string& dest : routers) {
                if (dest == router) continue;
                
                int minCost = INF;
                for (const string& via : routers) {
                    if (via == router || !links[router].count(via)) continue;
                    int linkCost = links[router][via];
                    int neighborCost = D[via][dest][dest]; // Use neighbor's direct cost
                    if (neighborCost != INF && linkCost != INF) {
                        int totalCost = linkCost + neighborCost;
                        if (totalCost < minCost) {
                            minCost = totalCost;
                        }
                    }
                }
                newD[router][dest] = minCost;
            }
        }
        
        // Update distance tables
        for (const string& router : routers) {
            for (const string& dest : routers) {
                if (dest == router) continue;
                for (const string& via : routers) {
                    if (via == router || !links[router].count(via)) continue;
                    int oldCost = D[router][dest][via];
                    int newCost = newD[router][dest];
                    if (newCost != oldCost) {
                        D[router][dest][via] = newCost;
                        changed = true;
                    }
                }
            }
        }
        
        if (changed) {
            printDistanceTables(step);
        }
    }
    
    // Print final routing tables
    printRoutingTables();
    
    // Handle updates section
    bool hasUpdates = false;
    while (getline(cin, line) && line != "END") {
        istringstream iss(line);
        string r1, r2;
        int cost;
        if (iss >> r1 >> r2 >> cost) {
            hasUpdates = true;
            if (cost == -1) {
                links[r1].erase(r2);
                links[r2].erase(r1);
            } else {
                links[r1][r2] = cost;
                links[r2][r1] = cost;
            }
        }
    }
    
    if (hasUpdates) {
        // Reset step counter
        step = 0;
        
        // Reinitialize distance tables
        for (const string& router : routers) {
            for (const string& dest : routers) {
                if (dest == router) continue;
                for (const string& via : routers) {
                    if (via == router) continue;
                    D[router][dest][via] = INF;
                }
                D[router][dest][dest] = links[router].count(dest) ? links[router][dest] : INF;
            }
        }
        
        // Print initial tables for update
        printDistanceTables(0);
        
        // Run algorithm again
        changed = true;
        while (changed) {
            changed = false;
            step++;
            
            map<string, map<string, int>> newD;
            for (const string& router : routers) {
                for (const string& dest : routers) {
                    if (dest == router) continue;
                    
                    int minCost = INF;
                    for (const string& via : routers) {
                        if (via == router || !links[router].count(via)) continue;
                        int linkCost = links[router][via];
                        int neighborCost = D[via][dest][dest];
                        if (neighborCost != INF && linkCost != INF) {
                            int totalCost = linkCost + neighborCost;
                            if (totalCost < minCost) {
                                minCost = totalCost;
                            }
                        }
                    }
                    newD[router][dest] = minCost;
                }
            }
            
            for (const string& router : routers) {
                for (const string& dest : routers) {
                    if (dest == router) continue;
                    for (const string& via : routers) {
                        if (via == router || !links[router].count(via)) continue;
                        int oldCost = D[router][dest][via];
                        int newCost = newD[router][dest];
                        if (newCost != oldCost) {
                            D[router][dest][via] = newCost;
                            changed = true;
                        }
                    }
                }
            }
            
            if (changed) {
                printDistanceTables(step);
            }
        }
        
        // Print final routing tables
        printRoutingTables();
    }
    
    return 0;
}