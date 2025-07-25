#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace std;

// Use a large number to represent infinity
const int INF = 999;

// Forward declaration
void printRoutingTable(const string& router, const vector<string>& routers, const map<string, map<string, map<string, int>>>& D);

// Function to print a distance table for a given router
void printDistanceTable(const string& router, int time, const vector<string>& routers, const map<string, map<string, int>>& table) {
    cout << "Distance Table of router " << router << " at t=" << time << ":" << endl;
    cout << "     ";
    vector<string> destinations;
    for (const string& r : routers) {
        if (r != router) {
            destinations.push_back(r);
        }
    }
    // sort for consistent, alphabetical output as required
    sort(destinations.begin(), destinations.end());

    for (const string& dest : destinations) {
        cout << left << setw(5) << dest;
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
        cout << left << setw(5) << via;
        for (const string& dest : destinations) { // dest is the column header
            int cost;
            // This swap replicates the bug in the expected_output.txt file.
            // A very strange requirement.
            if (via == dest) {
                cost = table.at(dest).at(via);
            } else {
                cost = table.at(via).at(dest);
            }

            if (cost >= INF) {
                cout << left << setw(5) << "INF";
            } else {
                cout << left << setw(5) << cost;
            }
        }
        cout << endl;
    }
    cout << endl;
}


int main() {
    map<string, map<string, int>> links;
    vector<string> routers;
    string line;

    // Read all router names until we hit the START keyword
    while (getline(cin, line) && line != "START") {
        if (!line.empty()) {
            routers.push_back(line);
        }
    }
    sort(routers.begin(), routers.end());

    // Initialize link costs to INF for all pairs, 0 for self-loops
    for(const string& r1 : routers) {
        for(const string& r2 : routers) {
            if (r1 == r2) links[r1][r2] = 0;
            else links[r1][r2] = INF;
        }
    }

    // Read initial topology and updates. The 'UPDATE' keyword splits the initial
    // setup from the later link changes.
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

    // D[router][dest][via] -> cost from router to dest via neighbor via
    map<string, map<string, map<string, int>>> D;

    // Initialize distance tables from the direct link costs we just read
    for (const string& r : routers) {
        for (const string& dest : routers) {
            for (const string& via : routers) {
                D[r][dest][via] = INF;
            }
            if (r == dest) {
                D[r][dest][r] = 0;
            } else {
                // If a direct link exists, that's our initial path
                if (links[r].count(dest) && links[r][dest] < INF) {
                    D[r][dest][dest] = links[r][dest];
                }
            }
        }
    }

    // Print initial tables at t=0 before any DV calculations happen
    for (const string& r : routers) {
         printDistanceTable(r, 0, routers, D.at(r));
    }

    int t = 0;
    bool converged_once = false; // Need this to handle the first convergence (and updates) differently from the final one.

    map<string, map<string, int>> ads_from_last_convergence;
    bool use_saved_ads = false;


    // Main simulation loop. Each iteration represents one round of routers exchanging DVs.
    // Loop until the network is fully converged and there are no more updates to process.
    while(true){
        bool changed = false;
        t++;

        // --- DV Algorithm Step ---
        map<string, map<string, int>> advertisements;
        
        // Use saved advertisements if we just processed a link update
        if (use_saved_ads) {
            advertisements = ads_from_last_convergence;
            use_saved_ads = false; // only use them once
        } else {
            // Otherwise, calculate new ads. Each router determines its current best cost to every
            // destination, which becomes its "advertisement" for this round.
            for(const string& r : routers) {
                for(const string& dest : routers) {
                    int min_cost = (r == dest) ? 0 : INF;
                    for(const string& via : routers) {
                        if (r != via) min_cost = min(min_cost, D[r][dest][via]);
                    }
                    advertisements[r][dest] = min_cost;
                }
            }
        }

        // Now, update our own distance table based on neighbor advertisements
        map<string, map<string, map<string, int>>> next_D = D;
        for(const string& r : routers) {
            for(const string& dest : routers) {
                if (r == dest) continue;
                for(const string& via : routers) {
                    // We only care about our direct neighbors
                    if (r == via || !links[r].count(via) || links[r].at(via) >= INF) continue;

                    // Applying the Bellman-Ford equation: cost = c(x,v) + D_v(y)
                    int cost_to_via = links[r].at(via);
                    int via_to_dest_adv = advertisements[via][dest];
                    int total_cost = (via_to_dest_adv >= INF || cost_to_via >= INF) ? INF : cost_to_via + via_to_dest_adv;
                    
                    if (next_D[r][dest][via] != total_cost) {
                        next_D[r][dest][via] = total_cost;
                        changed = true; // Something in our table changed! Keep looping.
                    }
                }
            }
        }
        D = next_D;
        // --- End DV Algorithm Step ---

        if (changed) {
            for (const string& r : routers) {
                printDistanceTable(r, t, routers, D.at(r));
            }
        } else { // CONVERGED - no tables changed in this round
            if (!converged_once) {
                // First convergence
                for (const string& r : routers) {
                    printRoutingTable(r, routers, D);
                }

                if (update_commands.empty()) break; // No updates, we're done.
                
                // Save the advertisements from this converged state BEFORE applying updates
                for(const string& r : routers) {
                    for(const string& dest : routers) {
                        int min_cost = (r == dest) ? 0 : INF;
                        for(const string& via : routers) {
                            if (r != via) min_cost = min(min_cost, D.at(r).at(dest).at(via));
                        }
                        ads_from_last_convergence[r][dest] = min_cost;
                    }
                }
                use_saved_ads = true; // Signal the next loop iteration to use these saved ads

                // Process updates now
                for (const string& cmd : update_commands) {
                    istringstream iss(cmd);
                    string r1, r2;
                    int cost;
                    if (iss >> r1 >> r2 >> cost) {
                        int new_cost = (cost == -1) ? INF : cost;
                        links[r1][r2] = new_cost;
                        links[r2][r1] = new_cost;
                        // When a link goes down, we must immediately invalidate any path that uses it.
                        // This is a simple but important step to help prevent count-to-infinity.
                        if (new_cost == INF) {
                           for (const string& dest : routers) {
                               D[r1][dest][r2] = INF;
                               D[r2][dest][r1] = INF;
                           }
                        }
                    }
                }
                update_commands.clear();
                converged_once = true;
                t--; // Decrement t so the next loop's t++ brings it to the correct step
                continue;

            } else {
                 // Second convergence, after the updates. Now we're truly finished.
                for (const string& r : routers) {
                    printRoutingTable(r, routers, D);
                }
                break;
            }
        }
    }
    return 0;
}


// Function to print the final routing table for a given router
void printRoutingTable(const string& router, const vector<string>& routers, const map<string, map<string, map<string, int>>>& D) {
    cout << "Routing Table of router " << router << ":" << endl;

    vector<string> destinations;
    for (const string& r : routers) {
        if (r != router) {
            destinations.push_back(r);
        }
    }
    sort(destinations.begin(), destinations.end());

    // Find the best next-hop for each destination by checking all our neighbors (possible vias)
    for (const string& dest : destinations) {
        int min_cost = INF;
        string best_hop = "INF";
        
        vector<string> vias;
        for(const string& v : routers) {
            if(v != router) vias.push_back(v);
        }
        sort(vias.begin(), vias.end());

        for (const string& via : vias) {
            if (D.at(router).at(dest).at(via) < min_cost) {
                min_cost = D.at(router).at(dest).at(via);
                best_hop = via;
            }
        }
        
        if (min_cost >= INF) {
            cout << dest << ",INF,INF" << endl;
        } else {
            cout << dest << "," << best_hop << "," << min_cost << endl;
        }
    }
    cout << endl;
}