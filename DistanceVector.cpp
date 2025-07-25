#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

const int INF = 999999;

void runAlgorithm(map<string, map<string, int>>& links, vector<string>& routers) {
    map<string, map<string, map<string, int>>> D;
    
    // Initialize distance tables
    for (const string& router : routers) {
        for (const string& dest : routers) {
            if (dest == router) continue;
            for (const string& via : routers) {
                if (via == router) continue;
                D[router][dest][via] = INF;
            }
            if (links[router].count(dest)) {
                D[router][dest][dest] = links[router][dest];
            }
        }
    }
    
    // Print initial tables
    for (const string& router : routers) {
        cout << "Distance Table of router " << router << " at t=0:" << endl;
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
    
    // Run Distance Vector algorithm
    int step = 0;
    bool changed = true;
    
    while (changed) {
        changed = false;
        step++;
        
// Calculate distance vectors - FIXED VERSION
        map<string, map<string, int>> distVectors;
        for (const string& router : routers) {
            for (const string& dest : routers) {
                if (dest == router) continue;
                
                int minCost = INF;
                // Check all neighbors to find minimum cost path
                for (const string& via : routers) {
                    if (via == router) continue;
                    if (links[router].count(via)) {
                        int linkCost = links[router][via];
                        int pathCost = D[router][dest][via];
                        if (linkCost != INF && pathCost != INF) {
                            int totalCost = linkCost + pathCost;
                            if (totalCost < minCost) {
                                minCost = totalCost;
                            }
                        }
                    }
                }
                distVectors[router][dest] = minCost;
            }
        }
        
        // Update distance tables
        for (const string& router : routers) {
            for (const string& neighbor : routers) {
                if (neighbor == router || !links[router].count(neighbor)) continue;
                
                for (const string& dest : routers) {
                    if (dest == router) continue;
                    
                    int oldDist = D[router][dest][neighbor];
                    int newDist = distVectors[neighbor][dest];
                    
                    if (oldDist != newDist) {
                        D[router][dest][neighbor] = newDist;
                        changed = true;
                    }
                }
            }
        }
        
        if (changed) {
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
        }
    }
    
    // Print routing tables
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
                int linkCost = links[router][via];
                int pathCost = D[router][dest][via];
                if (linkCost != INF && pathCost != INF) {
                    int totalCost = linkCost + pathCost;
                    if (totalCost < minCost) {
                        minCost = totalCost;
                        nextHop = via;
                    }
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
}

int main() {
    map<string, map<string, int>> links;
    vector<string> routers;
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
    
    // Run algorithm with initial topology
    runAlgorithm(links, routers);
    
    // Handle updates
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
        runAlgorithm(links, routers);
    }
    
    return 0;
}