#include "TimeSeries.h"
#include "Binary.h"
#include "HashTable.h"
#include "Graph.h" 
#include <iostream>
#include <fstream>
#include <sstream>
extern int countryCount;
extern TimeSeries countries[];
bool loadCountryFromFile(const std::string &filename, const std::string &targetCode, TimeSeries &tempTS) {
    std::ifstream file(filename);
    if (!file.is_open())
        return false;
    std::string line;
    bool found = false;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string Country, countryCode, Series, series_code, data;
        // Read the first 4 fields.
        if (!(std::getline(ss, Country, ',') &&
              std::getline(ss, countryCode, ',') &&
              std::getline(ss, Series, ',') &&
              std::getline(ss, series_code, ',')))
        {
            continue;
        }
        // Check if this line matches the target country code.
        if (countryCode == targetCode) {
            // Set up the temporary TimeSeries object.
            tempTS.setData(Country, countryCode);
            // Clean up any existing linked list in tempTS (if any)
            if (tempTS.getHead() != nullptr) {
                TimeSeriesNode* cur = tempTS.getHead();
                while (cur) {
                    TimeSeriesNode* nxt = cur->next;
                    delete cur;
                    cur = nxt;
                }
                tempTS.setHead(nullptr);
            }
            // Create the first TimeSeriesNode.
            tempTS.setHead(new TimeSeriesNode(Country, countryCode, Series, series_code));
            int year = 1960;
            while (std::getline(ss, data, ',')) {
                if (data != "-1") {
                    double value = std::stod(data);
                    tempTS.getHead()->timeseriesData.ResizeEqual(year, value);
                    year++;
                }
            }
            found = true;
            break; // Stop after finding the target country.
        }
    }
    file.close();
    return found;
}


int main() {
    BinaryTree db;
    HashTable hashTable;
    std::string command;
    Graph graph;

    while (std::cin >> command) {
        //std::cout << "Test";
        if (command == "LOAD") {
            std::string filename;
            std::cin >> filename;
            std::cout << db.load(filename) << std::endl;

            for (int i = 0; i < countryCount; i++) {
                std::string code = countries[i].getCountryCode();
                hashTable.insert(code, countries[i], false);
            }

        } 
        else if (command == "LIST") {
            std::string countryName;
            std::cin.ignore();
            std::getline(std::cin, countryName);
            db.print(countryName);
            std::cout << std::endl;
    
        } 
        else if (command == "RANGE") {
            std::string seriesCode;
            std::cin >> seriesCode;
            std::cout << db.range(seriesCode) << std::endl;
        } 
        else if (command == "BUILD") {
            std::string seriesCode;
            std::cin >> seriesCode;
            db.build(seriesCode);
            std::cout << "success" << std::endl;
        } 
        else if (command == "FIND") {
            double mean;
            std::string operation;
            std::cin >> mean >> operation;
            db.find(mean, operation);
        } 
        else if (command == "DELETE") {
            std::string countryName;
            std::cin.ignore();
            std::getline(std::cin, countryName);
            db.deleteCountry(countryName); 
            std::cout << "success" << std::endl;

        } 
        else if (command == "LIMITS") {
            std::string condition;
            std::cin >> condition;
            std::cout << db.Limits(condition) << std::endl;
        } 
        else if (command == "LOOKUP") {
                std::string countryCode;
                std::cin >> countryCode;
                // Use double hashing lookup.
                auto res = hashTable.lookup(countryCode);
                if (res.first == -1)
                    std::cout << "failure" << std::endl;
                else
                    std::cout << "index " << res.first << " searches " << res.second << std::endl;
        }
        else if (command == "REMOVE") {
            std::string countryCode;
            std::cin >> countryCode;
            if (hashTable.remove(countryCode,db)){
            std::cout << "success" << std::endl;
            }else{
            std::cout << "failure" << std::endl;
            }
        }
        
        else if (command == "INSERT") {
            std::string countryCode, filename;
            std::cin >> countryCode >> filename;
            auto res = hashTable.lookup(countryCode);
            if (res.first != -1) {
                std::cout << "failure" << std::endl;
            } else {
                // Allocate a new TimeSeries dynamically.
                TimeSeries* temp = new TimeSeries();
                if (loadCountryFromFile(filename, countryCode, *temp)) {
                    // Mark as owned so the HashTable destructor will delete it.
                    if (hashTable.insert(countryCode, *temp, true))
                        std::cout << "success" << std::endl;
                    else {
                        delete temp;
                        std::cout << "failure" << std::endl;
                    }
                } else {
                    delete temp;
                    std::cout << "failure" << std::endl;
                }
            }

        }
        else if (command == "INITIALIZE") {
            // Build nodes from the loaded countries and clear any edges.
            graph.initializeNodes();
            graph.initializeEdges();
            std::cout << "success" << std::endl;
        }
        else if (command == "UPDATE_EDGES") {
            std::string seriesCode, relation;
            double threshold;
            std::cin >> seriesCode >> threshold >> relation;
            bool added = graph.updateEdges(seriesCode, threshold, relation);
            std::cout << (added ? "success" : "failure") << std::endl;
        }
        else if (command == "ADJACENT") {
            std::string countryCode;
            std::cin >> countryCode;
            bool found;
            std::vector<std::string> adjacentCountries = graph.adjacent(countryCode, found);
            if (!found) {
                std::cout << "failure" << std::endl;
            } else if (adjacentCountries.empty()) {
                std::cout << "none" << std::endl;
            } else {
                // Output the adjacent country names space-separated.
                for (size_t i = 0; i < adjacentCountries.size(); i++) {
                    std::cout << adjacentCountries[i];
                    if (i != adjacentCountries.size() - 1)
                        std::cout << " ";
                }
                std::cout << std::endl;
            }
        }
        else if (command == "PATH") {
            std::string code1, code2;
            std::cin >> code1 >> code2;
            bool connected = graph.path(code1, code2);
            std::cout << (connected ? "true" : "false") << std::endl;
        }
        else if (command == "RELATIONSHIPS") {
            std::string code1, code2;
            std::cin >> code1 >> code2;
            std::vector<Relationship> rels = graph.getRelationships(code1, code2);
            if (rels.empty()) {
                std::cout << "none" << std::endl;
            } else {
                for (const auto& r : rels) {
                    // Format each relationship as (Series_Code Threshold Relation)
                    std::cout << "(" << r.seriesCode << " " << r.threshold << " " << r.relation << ") ";
                }
                std::cout << std::endl;
            }
        }
        
        else if (command == "EXIT") {
            db.exit();
        }
        // std::cout << std::endl;
    }
    return 0;
}
