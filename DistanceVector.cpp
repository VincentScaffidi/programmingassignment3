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
    
    cout << "=== DEBUG: Starting Distance Vector Algorithm ===" << endl;
    
    // Read router names
    while (getline(cin, line)) {
        if (line == "START") break;
        if (!line.empty()) {
            routers.push_back(line);
            cout << "DEBUG: Added router: " << line << endl;
        }
    }
    sort(routers.begin(), routers.end());
    cout << "DEBUG: Router list (sorted): ";
    for (const string& r : routers) cout << r << " ";
    cout << endl;
    
    // Read initial topology
    cout << "DEBUG: Reading initial topology..." << endl;
    while (getline(cin, line) && line != "UPDATE") {
        istringstream iss(line);
        string r1, r2;
        int cost;
        if (iss >> r1 >> r2 >> cost) {
            if (cost == -1) {
                links[r1].erase(r2);
                links[r2].erase(r1);
                cout << "DEBUG: Removed link " << r1 << " <-> " << r2 << endl;
            } else {
                links[r1][r2] = cost;
                links[r2][r1] = cost;
                cout << "DEBUG: Added link " << r1 << " <-> " << r2 << " cost: " << cost << endl;
            }
        }
    }
    
    // Print all links
    cout << "DEBUG: Final link structure:" << endl;
    for (const string& router : routers) {
        cout << "  " << router << " links: ";
        for (const auto& link : links[router]) {
            cout << link.first << "(" << link.second << ") ";
        }
        cout << endl;
    }
    
    // Initialize distance tables
    cout << "DEBUG: Initializing distance tables..." << endl;
    for (const string& router : routers) {
        for (const string& dest : routers) {
            if (dest == router) continue;
            for (const string& via : routers) {
                if (via == router) continue;
                D[router][dest][via] = INF;
            }
            if (links[router].count(dest)) {
                D[router][dest][dest] = links[router][dest];
                cout << "DEBUG: " << router << " -> " << dest << " direct cost: " << links[router][dest] << endl;
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
        cout << "DEBUG: ===== STEP " << step << " =====" << endl;
        
        // Calculate distance vectors using current best distances
        map<string, map<string, int>> distVectors;
        cout << "DEBUG: Calculating distance vectors..." << endl;
        
        for (const string& router : routers) {
            cout << "DEBUG: Router " << router << " calculating distances:" << endl;
            for (const string& dest : routers) {
                if (dest == router) continue;
                
                int minCost = INF;
                string bestVia = "";
                
                for (const string& via : routers) {
                    if (via == router) continue;
                    if (links[router].count(via)) {
                        int linkCost = links[router][via];
                        int pathCost = D[router][dest][via];
                        if (linkCost != INF && pathCost != INF) {
                            int totalCost = linkCost + pathCost;
                            cout << "    " << dest << " via " << via << ": " << linkCost << " + " << pathCost << " = " << totalCost;
                            if (totalCost < minCost) {
                                minCost = totalCost;
                                bestVia = via;
                                cout << " (NEW BEST)";
                            }
                            cout << endl;
                        }
                    }
                }
                distVectors[router][dest] = minCost;
                cout << "  " << router << " best distance to " << dest << ": " << minCost << " via " << bestVia << endl;
            }
        }
        
        cout << "DEBUG: Distance vectors calculated. Now updating tables..." << endl;
        
        // Update distance tables with neighbor's BEST distances
        for (const string& router : routers) {
            cout << "DEBUG: Updating " << router << "'s distance table..." << endl;
            for (const string& neighbor : routers) {
                if (neighbor == router || !links[router].count(neighbor)) continue;
                
                cout << "  Processing updates from neighbor " << neighbor << endl;
                
                for (const string& dest : routers) {
                    if (dest == router) continue;
                    
                    // Don't overwrite direct costs
                    if (dest == neighbor) {
                        cout << "    Skipping " << dest << " (direct link)" << endl;
                        continue;
                    }
                    
                    int oldDist = D[router][dest][neighbor];
                    int neighborBestDist = distVectors[neighbor][dest];
                    int linkCost = links[router][neighbor];
                    
                    // Distance table stores what we would calculate via this neighbor
                    int newDist;
                    if (neighborBestDist == INF) {
                        newDist = INF;
                    } else {
                        newDist = linkCost + neighborBestDist;
                    }
                    
                    cout << "    " << dest << " via " << neighbor << ": old=" << oldDist 
                         << " new=" << linkCost << "+" << neighborBestDist << "=" << newDist;
                    
                    if (oldDist != newDist) {
                        D[router][dest][neighbor] = newDist;
                        changed = true;
                        cout << " (CHANGED!)";
                    }
                    cout << endl;
                }
            }
        }
        
        cout << "DEBUG: Step " << step << " changed=" << (changed ? "YES" : "NO") << endl;
        
        // Print distance tables if changed
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
    
    cout << "DEBUG: Algorithm converged. Calculating routing tables..." << endl;
    
    // Print routing tables - use minimum from distance table
    for (const string& router : routers) {
        cout << "DEBUG: Calculating routing table for " << router << endl;
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
            
            cout << "  " << dest << " options: ";
            for (const string& via : neighbors) {
                int cost = D[router][dest][via];
                cout << "via " << via << "(" << cost << ") ";
                if (cost < minCost) {
                    minCost = cost;
                    nextHop = via;
                }
            }
            cout << " -> chose " << nextHop << " cost " << minCost << endl;
            
            if (minCost == INF) {
                cout << dest << ",INF,INF" << endl;
            } else {
                cout << dest << "," << nextHop << "," << minCost << endl;
            }
        }
        cout << endl;
    }
    
    // Handle updates section (simplified debug for now)
    bool hasUpdates = false;
    while (getline(cin, line) && line != "END") {
        hasUpdates = true;
        cout << "DEBUG: Found update: " << line << endl;
        // Process normally but with debug...
    }
    
    cout << "DEBUG: Algorithm complete!" << endl;
    return 0;
}