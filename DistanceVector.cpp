#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

const int INF = 999999; // Using large number for infinity as specified

class Router {
public:
    string name;
    map<string, int> directLinks;           // Store direct neighbor costs
    map<string, map<string, int>> distanceTable; // [destination][nextHop] = cost
    map<string, int> bestDistances;         // Best known distance to each destination
    map<string, string> nextHop;            // Best next hop for each destination
    
    Router(string n) : name(n) {}
    
    // Initialize distance table for all known routers
    // This implements the initialization phase of DV algorithm
    void initialize(const vector<string>& allRouters) {
        for (const string& dest : allRouters) {
            if (dest == name) continue; // Skip self
            
            // Initialize with infinity as per DV algorithm
            bestDistances[dest] = INF;
            nextHop[dest] = "";
            
            // Initialize distance table row for this destination
            for (const string& hop : allRouters) {
                if (hop == name) continue;
                distanceTable[dest][hop] = INF;
            }
            
            // Set direct link costs if they exist
            // D_x(y) = c(x,y) for direct neighbors
            if (directLinks.count(dest)) {
                distanceTable[dest][dest] = directLinks[dest];
                bestDistances[dest] = directLinks[dest];
                nextHop[dest] = dest;
            }
        }
    }
    bool updateFromNeighbor(const string& neighbor, const map<string, int>& neighborDistances) {
        bool changed = false;
        
        // Update our knowledge of neighbor's distances to all destinations
        for (const auto& entry : neighborDistances) {
            string dest = entry.first;
            int neighborDist = entry.second;
            
            if (dest == name) continue; // Skip self
            
            // Update distance table with neighbor's reported distance
            distanceTable[dest][neighbor] = neighborDist;
        }
        
        // Recalculate best distances using Bellman-Ford equation
        for (const auto& entry : bestDistances) {
            string dest = entry.first;
            int oldBest = bestDistances[dest];
            int newBest = INF;
            string newNextHop = "";
            
            // Check all possible next hops (direct neighbors only)
            vector<string> neighbors;
            for (const auto& link : directLinks) {
                neighbors.push_back(link.first);
            }
            sort(neighbors.begin(), neighbors.end()); // Alphabetical order for tie-breaking
            
            // Apply Bellman-Ford: find minimum cost path
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


    // Print distance table in required format
    void printDistanceTable(int step, const vector<string>& allRouters) {
        cout << "Distance Table of router " << name << " at t=" << step << ":" << endl;
        
        // Print header with destination names
        cout << "     ";
        for (const string& dest : allRouters) {
            if (dest != name) {
                cout << dest << "    ";
            }
        }
        cout << endl;
        
        // Print each row (next hop options)
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
};

class Network {
private:
    map<string, Router> routers;
    vector<string> routerNames;
    
public:
    // Add a router to the network
    // Maintains alphabetical ordering for consistent output
    void addRouter(const string& name) {
        if (routers.find(name) == routers.end()) {
            routers[name] = Router(name);
            routerNames.push_back(name);
            sort(routerNames.begin(), routerNames.end()); // Keep alphabetical order
        }
    }
    
    // Add or update a link between two routers
    // Cost of -1 means remove the link
    void addLink(const string& router1, const string& router2, int cost) {
        // Ensure both routers exist
        addRouter(router1);
        addRouter(router2);
        
        if (cost == -1) {
            // Remove link - bidirectional
            routers[router1].directLinks.erase(router2);
            routers[router2].directLinks.erase(router1);
        } else {
            // Add/update link - bidirectional since it's undirected network
            routers[router1].directLinks[router2] = cost;
            routers[router2].directLinks[router1] = cost;
        }
    }
    
    // Initialize all routers with current topology
    void initializeRouters() {
        for (auto& pair : routers) {
            pair.second.initialize(routerNames);
        }
    }
    
    // TODO: Implement the actual Distance Vector algorithm
    void runDistanceVector() {
        initializeRouters();
        
        // Print initial state
        for (const string& name : routerNames) {
            routers[name].printDistanceTable(0, routerNames);
        }
    }
};

int main() {
    Network network;
    string line;
    
    // Read router names until START
    while (getline(cin, line)) {
        if (line == "START") {
            break;
        } else if (!line.empty()) {
            network.addRouter(line);
        }
    }
    
    // Read initial topology until UPDATE
    while (getline(cin, line) && line != "UPDATE") {
        istringstream iss(line);
        string router1, router2;
        int cost;
        
        if (iss >> router1 >> router2 >> cost) {
            network.addLink(router1, router2, cost);
        }
    }
    
    // Run algorithm with initial topology
    network.runDistanceVector();
    
    return 0;
}