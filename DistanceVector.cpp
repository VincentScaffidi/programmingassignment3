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
    
    // Default constructor needed for map operations
    Router() : name("") {}
    
    // Parameterized constructor
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
    
    // Core DV algorithm: Update distance table based on neighbor's distance vector
    // This implements the Bellman-Ford equation: D_x(y) = min_v{c(x,v) + D_v(y)}
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
            
            // Check if anything changed (for convergence detection)
            if (newBest != oldBest || nextHop[dest] != newNextHop) {
                bestDistances[dest] = newBest;
                nextHop[dest] = newNextHop;
                changed = true;
            }
        }
        
        return changed; // Return true if any distance changed
    }
    
    // Get this router's distance vector to send to neighbors
    map<string, int> getDistanceVector() {
        return bestDistances;
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

    // Print final routing table in required format: dest,nextHop,cost
    void printRoutingTable(const vector<string>& allRouters) {
        cout << "Routing Table of router " << name << ":" << endl;
        
        // Print destinations in alphabetical order
        for (const string& dest : allRouters) {
            if (dest == name) continue; // Skip self
            
            if (bestDistances[dest] == INF) {
                // Unreachable destination
                cout << dest << ",INF,INF" << endl;
            } else {
                // Reachable destination: dest,nextHop,totalCost
                cout << dest << "," << nextHop[dest] << "," << bestDistances[dest] << endl;
            }
        }
        cout << endl; // Blank line after each routing table
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
            routers[name] = Router(name);  // This now works with default constructor
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
    
    // Main Distance Vector algorithm implementation
    void runDistanceVector() {
        initializeRouters();
        
        int step = 0;
        bool changed = true;
        
        // Print initial distance tables (t=0)
        for (const string& name : routerNames) {
            routers[name].printDistanceTable(step, routerNames);
        }
        
        // Run algorithm until convergence (no changes)
        while (changed) {
            changed = false;
            step++;
            
            // Synchronous update: all routers send distance vectors simultaneously
            map<string, map<string, int>> distanceVectors;
            for (const string& name : routerNames) {
                distanceVectors[name] = routers[name].getDistanceVector();
            }
            
            // Each router processes updates from all its neighbors
            for (const string& name : routerNames) {
                Router& router = routers[name];
                
                // Process distance vectors from all direct neighbors
                for (const auto& link : router.directLinks) {
                    string neighbor = link.first;
                    if (router.updateFromNeighbor(neighbor, distanceVectors[neighbor])) {
                        changed = true; // At least one router changed
                    }
                }
            }
            
            // Print distance tables if there were changes
            if (changed) {
                for (const string& name : routerNames) {
                    routers[name].printDistanceTable(step, routerNames);
                }
            }
        }
        
        // Print final routing tables after convergence
        for (const string& name : routerNames) {
            routers[name].printRoutingTable(routerNames);
        }
    }
};

// Complete main function with full input handling
int main() {
    Network network;
    string line;
    
    // Read router names until START
    while (getline(cin, line)) {
        if (line == "START") {
            // Read topology links until UPDATE
            while (getline(cin, line) && line != "UPDATE") {
                istringstream iss(line);
                string router1, router2;
                int cost;
                
                if (iss >> router1 >> router2 >> cost) {
                    network.addLink(router1, router2, cost);
                }
            }
            
            // Run algorithm with initial topology and print results
            network.runDistanceVector();
            
            // Read updates until END
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
            
            // If there were updates, run algorithm again
            if (hasUpdates) {
                network.runDistanceVector();
            }
            
            break; // Exit after processing one complete input section
            
        } else if (!line.empty()) {
            // Add router name
            network.addRouter(line);
        }
    }
    
    return 0;
}