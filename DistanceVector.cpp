#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

const int INF = 999999;

class Router {
public:
    string name;
    map<string, int> directLinks;
    map<string, map<string, int>> distanceTable;
    map<string, int> bestDistances;
    map<string, string> nextHop;
    
    Router() : name("") {}
    Router(string n) : name(n) {}
    
    void initialize(const vector<string>& allRouters) {
        for (const string& dest : allRouters) {
            if (dest == name) continue;
            
            bestDistances[dest] = INF;
            nextHop[dest] = "";
            
            for (const string& hop : allRouters) {
                if (hop == name) continue;
                distanceTable[dest][hop] = INF;
            }
            
            // Fix: Set direct costs properly
            if (directLinks.count(dest)) {
                distanceTable[dest][dest] = directLinks[dest];
                bestDistances[dest] = directLinks[dest];
                nextHop[dest] = dest;
            }
        }
    }
    
    bool updateFromNeighbor(const string& neighbor, const map<string, int>& neighborDistances) {
        bool changed = false;
        
        for (const auto& entry : neighborDistances) {
            string dest = entry.first;
            int neighborDist = entry.second;
            
            if (dest == name) continue;
            distanceTable[dest][neighbor] = neighborDist;
        }
        
        for (const auto& entry : bestDistances) {
            string dest = entry.first;
            int oldBest = bestDistances[dest];
            string oldNextHop = nextHop[dest];
            int newBest = INF;
            string newNextHop = "";
            
            vector<string> neighbors;
            for (const auto& link : directLinks) {
                neighbors.push_back(link.first);
            }
            sort(neighbors.begin(), neighbors.end());
            
            for (const string& neighbor : neighbors) {
                int costToNeighbor = directLinks[neighbor];
                int neighborToDest = distanceTable[dest][neighbor];
                
                if (costToNeighbor != INF && neighborToDest != INF) {
                    int totalCost = costToNeighbor + neighborToDest;
                    if (totalCost < newBest) {
                        newBest = totalCost;
                        newNextHop = neighbor;
                    }
                }
            }
            
            if (newBest != oldBest || newNextHop != oldNextHop) {
                bestDistances[dest] = newBest;
                nextHop[dest] = newNextHop;
                changed = true;
            }
        }
        
        return changed;
    }
    
    map<string, int> getDistanceVector() {
        return bestDistances;
    }

    void printDistanceTable(int step, const vector<string>& allRouters) {
        cout << "Distance Table of router " << name << " at t=" << step << ":" << endl;
        
        cout << "     ";
        for (const string& dest : allRouters) {
            if (dest != name) {
                cout << dest << "    ";
            }
        }
        cout << endl;
        
        for (const string& hop : allRouters) {
            if (hop == name) continue;
            
            cout << hop << "    ";
            for (const string& dest : allRouters) {
                if (dest == name) continue;
                
                int dist = distanceTable[dest][hop];
                if (dist == INF) {
                    cout << "INF  ";
                } else {
                    cout << dist << "    ";
                }
            }
            cout << endl;
        }
        cout << endl;
    }

    void printRoutingTable(const vector<string>& allRouters) {
        cout << "Routing Table of router " << name << ":" << endl;
        
        for (const string& dest : allRouters) {
            if (dest == name) continue;
            
            if (bestDistances[dest] == INF) {
                cout << dest << ",INF,INF" << endl;
            } else {
                cout << dest << "," << nextHop[dest] << "," << bestDistances[dest] << endl;
            }
        }
        cout << endl;
    }
};

class Network {
private:
    map<string, Router> routers;
    vector<string> routerNames;
    
public:
    void addRouter(const string& name) {
        if (routers.find(name) == routers.end()) {
            routers[name] = Router(name);
            routerNames.push_back(name);
            sort(routerNames.begin(), routerNames.end());
        }
    }
    
    void addLink(const string& router1, const string& router2, int cost) {
        addRouter(router1);
        addRouter(router2);
        
        if (cost == -1) {
            routers[router1].directLinks.erase(router2);
            routers[router2].directLinks.erase(router1);
        } else {
            routers[router1].directLinks[router2] = cost;
            routers[router2].directLinks[router1] = cost;
        }
    }
    
    void initializeRouters() {
        for (auto& pair : routers) {
            pair.second.initialize(routerNames);
        }
    }
    
    void runDistanceVector() {
        initializeRouters();
        
        int step = 0;
        bool changed = true;
        
        for (const string& name : routerNames) {
            routers[name].printDistanceTable(step, routerNames);
        }
        
        while (changed) {
            changed = false;
            step++;
            
            map<string, map<string, int>> distanceVectors;
            for (const string& name : routerNames) {
                distanceVectors[name] = routers[name].getDistanceVector();
            }
            
            for (const string& name : routerNames) {
                Router& router = routers[name];
                
                for (const auto& link : router.directLinks) {
                    string neighbor = link.first;
                    if (router.updateFromNeighbor(neighbor, distanceVectors[neighbor])) {
                        changed = true;
                    }
                }
            }
            
            if (changed) {
                for (const string& name : routerNames) {
                    routers[name].printDistanceTable(step, routerNames);
                }
            }
        }
        
        for (const string& name : routerNames) {
            routers[name].printRoutingTable(routerNames);
        }
    }
};

int main() {
    Network network;
    string line;
    
    while (getline(cin, line)) {
        if (line == "START") {
            while (getline(cin, line) && line != "UPDATE") {
                istringstream iss(line);
                string router1, router2;
                int cost;
                
                if (iss >> router1 >> router2 >> cost) {
                    network.addLink(router1, router2, cost);
                }
            }
            
            network.runDistanceVector();
            
            bool hasUpdates = false;
            while (getline(cin, line) && line != "END") {
                istringstream iss(line);
                string router1, router2;
                int cost;
                
                if (iss >> router1 >> router2 >> cost) {
                    network.addLink(router1, router2, cost);
                    hasUpdates = true;
                }
            }
            
            if (hasUpdates) {
                network.runDistanceVector();
            }
            
            break;
            
        } else if (!line.empty()) {
            network.addRouter(line);
        }
    }
    
    return 0;
}