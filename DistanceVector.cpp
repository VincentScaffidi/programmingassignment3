#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

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
    // TODO: Implement network management
};

int main() {
    // TODO: Complete main function
    return 0;
}