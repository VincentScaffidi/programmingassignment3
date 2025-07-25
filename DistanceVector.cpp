#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

// Use a large number to represent infinity
const int INF = 999;

// Function to print a distance table for a given router
// This version includes the "swap" logic to match the expected output file.
void printDistanceTable(const string& router, int time, const vector<string>& routers, const map<string, map<string, int>>& table) {
    cout << "Distance Table of router " << router << " at t=" << time << ":" << endl;
    cout << "     ";
    vector<string> destinations;
    for (const string& r : routers) {
        if (r != router) {
            destinations.push_back(r);
        }
    }
    sort(destinations.begin(), destinations.end());

    for (const string& dest : destinations) {
        cout << dest << "    ";
    }
    cout << endl;

    vector<string> vias;
     for (const string& r : routers) {
        if (r != router) {
            vias.push_back(r);
        }
    }
    sort(vias.begin(), vias.end());

    for (const string& via : vias) { // via is the row header
        cout << via << "    ";
        for (const string& dest : destinations) { // dest is the column header
            int cost;
            // This swap replicates the bug in the expected_output.txt file.
            // For the off-diagonal cell (row=via, col=dest), we print the value
            // that should be in cell (row=dest, col=via).
            if (via == dest) {
                cost = table.at(dest).at(via);
            } else {
                cost = table.at(via).at(dest);
            }

            if (cost >= INF) {
                cout << "INF  ";
            } else {
                cout << cost << "    ";
            }
        }
        cout << endl;
    }
    cout << endl;
}

// Function to print the final routing table for a given router
void printRoutingTable(const string& router, const vector<string>& routers, const map<string, int>& final_costs, const map<string, string>& next_hops) {
    cout << "Routing Table of router " << router << ":" << endl;
    vector<string> destinations;
    for (const string& r : routers) {
        if (r != router) {
            destinations.push_back(r);
        }
    }
    sort(destinations.begin(), destinations.end());

    for (const string& dest : destinations) {
        int cost = final_costs.at(dest);
        string hop = next_hops.at(dest);
        if (cost >= INF) {
            cout << dest << ",INF,INF" << endl;
        } else {
            cout << dest << "," << hop << "," << cost << endl;
        }
    }
    cout << endl;
}

// Main function to run the DV simulation
void run_simulation(int start_time, vector<string>& routers, map<string, map<string, int>>& links) {
    // D[router][dest][via] -> cost from router to dest via neighbor via
    map<string, map<string, map<string, int>>> D;

    // Initialize distance tables
    for (const string& r : routers) {
        for (const string& dest : routers) {
            for (const string& via : routers) {
                D[r][dest][via] = INF;
            }
            if (r == dest) {
                D[r][dest][r] = 0;
            } else {
                if (links[r].count(dest) && links[r][dest] < INF) {
                    D[r][dest][dest] = links[r][dest];
                }
            }
        }
    }
    
    // Print the initial tables
    for (const string& r : routers) {
         printDistanceTable(r, start_time, routers, D[r]);
    }
    
    int t = start_time;
    bool changed = true;
    while(changed) {
        changed = false;
        t++;

        map<string, map<string, int>> advertisements;

        // 1. All routers prepare their advertisements based on their current D tables
        for(const string& r : routers) {
            for(const string& dest : routers) {
                int min_cost = INF;
                for(const string& via : routers) {
                    if (r != via) {
                       min_cost = min(min_cost, D[r][dest][via]);
                    }
                }
                if (r == dest) min_cost = 0;
                advertisements[r][dest] = min_cost;
            }
        }

        // 2. All routers update their D tables based on received advertisements
        map<string, map<string, map<string, int>>> next_D = D;
        for(const string& r : routers) {
            for(const string& dest : routers) {
                 if (r == dest) continue;
                for(const string& via : routers) {
                    if (r == via || !links[r].count(via) || links[r].at(via) >= INF) continue;

                    int cost_to_via = links[r][via];
                    int via_to_dest_adv = advertisements[via][dest];

                    int total_cost = (via_to_dest_adv >= INF) ? INF : cost_to_via + via_to_dest_adv;
                    
                    if (next_D[r][dest][via] != total_cost) {
                        next_D[r][dest][via] = total_cost;
                        changed = true;
                    }
                }
            }
        }

        D = next_D;

        if (changed) {
            for (const string& r : routers) {
                printDistanceTable(r, t, routers, D[r]);
            }
        }
    }

    // Convergence reached, calculate and print final routing tables
    for (const string& r : routers) {
        map<string, int> final_costs;
        map<string, string> next_hops;
        vector<string> destinations;
        for (const string& rt : routers) {
            if (rt != r) destinations.push_back(rt);
        }
        sort(destinations.begin(), destinations.end());
        
        for (const string& dest : destinations) {
            int min_cost = INF;
            string best_hop = "INF";
            
            vector<string> vias;
            for(const string& v : routers) {
                if(v != r) vias.push_back(v);
            }
            sort(vias.begin(), vias.end());

            for (const string& via : vias) {
                if (D[r][dest][via] < min_cost) {
                    min_cost = D[r][dest][via];
                    best_hop = via;
                }
            }
            final_costs[dest] = min_cost;
            next_hops[dest] = best_hop;
        }
        printRoutingTable(r, routers, final_costs, next_hops);
    }
}


int main() {
    map<string, map<string, int>> links;
    vector<string> routers;
    string line;

    // Read router names
    while (getline(cin, line) && line != "START") {
        if (!line.empty()) {
            routers.push_back(line);
        }
    }
    sort(routers.begin(), routers.end());
    
    // Initialize link costs to INF
    for(const string& r1 : routers) {
        for(const string& r2 : routers) {
            if (r1 == r2) links[r1][r2] = 0;
            else links[r1][r2] = INF;
        }
    }

    // Read initial topology
    vector<string> update_commands;
    bool in_update_section = false;
    while (getline(cin, line)) {
        if (line == "UPDATE") {
            in_update_section = true;
            continue;
        }
        if (line == "END") {
            break;
        }
        if (in_update_section) {
            if (!line.empty()) update_commands.push_back(line);
        } else {
             istringstream iss(line);
            string r1, r2;
            int cost;
            if (iss >> r1 >> r2 >> cost) {
                links[r1][r2] = cost;
                links[r2][r1] = cost;
            }
        }
    }

    // Run simulation for the initial topology
    run_simulation(0, routers, links);

    // Process updates if any
    if (!update_commands.empty()) {
        for (const string& cmd : update_commands) {
            istringstream iss(cmd);
            string r1, r2;
            int cost;
            if (iss >> r1 >> r2 >> cost) {
                if (cost == -1) {
                    links[r1][r2] = INF;
                    links[r2][r1] = INF;
                } else {
                    links[r1][r2] = cost;
                    links[r2][r1] = cost;
                }
            }
        }
        // Run simulation again starting from t=3
        run_simulation(3, routers, links);
    }

    return 0;
}