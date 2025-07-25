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
            // Direct link costs
            if (links[router].count(dest)) {
                D[router][dest][dest] = links[router][dest];
            }
        }
    }

    // Print initial distance tables
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

    // Run DV algorithm
    int step = 0;
    bool changed = true;

    while (changed) {
        changed = false;
        step++;

        // Store what each router will advertise
        map<string, map<string, int>> advertisements;
        
        // Each router calculates its best distances
        for (const string& router : routers) {
            for (const string& dest : routers) {
                if (dest == router) {
                    advertisements[router][dest] = 0;
                    continue;
                }
                
                int minCost = INF;
                
                // Direct connection
                if (links[router].count(dest)) {
                    minCost = links[router][dest];
                }
                
                // Via neighbors
                for (const string& via : routers) {
                    if (via != router && via != dest && links[router].count(via)) {
                        int costToVia = links[router][via];
                        int costViaToDest = D[router][dest][via];
                        
                        if (costToVia != INF && costViaToDest != INF) {
                            minCost = min(minCost, costToVia + costViaToDest);
                        }
                    }
                }
                
                advertisements[router][dest] = minCost;
            }
        }

        // Update distance tables with what neighbors advertise
        for (const string& router : routers) {
            for (const string& neighbor : routers) {
                if (neighbor == router || !links[router].count(neighbor)) continue;
                
                for (const string& dest : routers) {
                    if (dest == router) continue;
                    
                    // What the neighbor advertises as its distance to dest
                    int advertised = advertisements[neighbor][dest];
                    
                    // Special case: if dest is the neighbor itself, distance is 0
                    if (dest == neighbor) {
                        advertised = 0;
                    }
                    
                    // Store in distance table
                    if (D[router][dest][neighbor] != advertised) {
                        D[router][dest][neighbor] = advertised;
                        changed = true;
                    }
                }
            }
        }

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

    // Print routing tables
    for (const string& router : routers) {
        cout << "Routing Table of router " << router << ":" << endl;
        
        for (const string& dest : routers) {
            if (dest == router) continue;
            
            int minCost = INF;
            string nextHop = "";
            
            // Check all neighbors in alphabetical order
            vector<string> neighbors;
            for (const string& n : routers) {
                if (n != router && links[router].count(n)) {
                    neighbors.push_back(n);
                }
            }
            sort(neighbors.begin(), neighbors.end());
            
            for (const string& via : neighbors) {
                int totalCost;
                
                if (dest == via) {
                    // Direct connection
                    totalCost = links[router][via];
                } else {
                    // Via this neighbor
                    int linkCost = links[router][via];
                    int neighborDist = D[router][dest][via];
                    if (linkCost != INF && neighborDist != INF) {
                        totalCost = linkCost + neighborDist;
                    } else {
                        totalCost = INF;
                    }
                }
                
                if (totalCost < minCost) {
                    minCost = totalCost;
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

    // Handle updates section
    bool hasUpdates = false;
    vector<string> updateLines;
    
    while (getline(cin, line) && line != "END") {
        if (!line.empty()) {
            updateLines.push_back(line);
            hasUpdates = true;
        }
    }
    
    if (hasUpdates) {
        // Process updates
        for (const string& update : updateLines) {
            istringstream iss(update);
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
        
        // Reinitialize distance tables  
        D.clear();
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
        
        // Print initial tables after update
        for (const string& router : routers) {
            cout << "Distance Table of router " << router << " at t=3:" << endl;
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
        
        // Run algorithm again
        step = 3;
        changed = true;
        
        while (changed) {
            changed = false;
            step++;
            
            // Store what each router will advertise
            map<string, map<string, int>> advertisements;
            
            // Each router calculates its best distances
            for (const string& router : routers) {
                for (const string& dest : routers) {
                    if (dest == router) {
                        advertisements[router][dest] = 0;
                        continue;
                    }
                    
                    int minCost = INF;
                    
                    // Direct connection
                    if (links[router].count(dest)) {
                        minCost = links[router][dest];
                    }
                    
                    // Via neighbors
                    for (const string& via : routers) {
                        if (via != router && via != dest && links[router].count(via)) {
                            int costToVia = links[router][via];
                            int costViaToDest = D[router][dest][via];
                            
                            if (costToVia != INF && costViaToDest != INF) {
                                minCost = min(minCost, costToVia + costViaToDest);
                            }
                        }
                    }
                    
                    advertisements[router][dest] = minCost;
                }
            }
            
            // Update distance tables
            for (const string& router : routers) {
                for (const string& neighbor : routers) {
                    if (neighbor == router || !links[router].count(neighbor)) continue;
                    
                    for (const string& dest : routers) {
                        if (dest == router) continue;
                        
                        int advertised = advertisements[neighbor][dest];
                        
                        if (dest == neighbor) {
                            advertised = 0;
                        }
                        
                        if (D[router][dest][neighbor] != advertised) {
                            D[router][dest][neighbor] = advertised;
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
        
        // Print final routing tables
        for (const string& router : routers) {
            cout << "Routing Table of router " << router << ":" << endl;
            
            for (const string& dest : routers) {
                if (dest == router) continue;
                
                int minCost = INF;
                string nextHop = "";
                
                vector<string> neighbors;
                for (const string& n : routers) {
                    if (n != router && links[router].count(n)) {
                        neighbors.push_back(n);
                    }
                }
                sort(neighbors.begin(), neighbors.end());
                
                for (const string& via : neighbors) {
                    int totalCost;
                    
                    if (dest == via) {
                        totalCost = links[router][via];
                    } else {
                        int linkCost = links[router][via];
                        int neighborDist = D[router][dest][via];
                        if (linkCost != INF && neighborDist != INF) {
                            totalCost = linkCost + neighborDist;
                        } else {
                            totalCost = INF;
                        }
                    }
                    
                    if (totalCost < minCost) {
                        minCost = totalCost;
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
    }

    return 0;
}