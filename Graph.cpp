#include "Graph.h"
#include "TimeSeries.h"
#include <cmath>
#include <queue>

Graph::Graph() {
    // Initially, nodes and edges are empty.
}

// Build nodes from the global TimeSeries array.
// Assumes that the global variable "countryCount" and array "countries" (from TimeSeries) are available.
extern int countryCount;
extern TimeSeries countries[];

void Graph::initializeNodes() {
    nodes.clear();
    // Iterate through all loaded countries.
    for (int i = 0; i < countryCount; i++) {
        std::string code = countries[i].getCountryCode();
        std::string name = countries[i].getCountryName();
        Node node;
        node.countryCode = code;
        node.countryName = name;
        node.ts = &countries[i];
        nodes[code] = node;
    }
    // Clear any existing edges.
    adjList.clear();
}

void Graph::initializeEdges() {
    // Clear only the edge list.
    adjList.clear();
}

// Helper: Given a mean and relationship type, return whether the condition holds.
bool satisfiesCondition(double mean, double threshold, const std::string& relation) {
    if (mean < 0) return false; // means < 0 indicates invalid or missing data.
    //if else statments to check if it satify's the conditions with the threshold
    if (relation == "greater")
        return mean > threshold;
    else if (relation == "less")
        return mean < threshold;
    else if (relation == "equal")
        return std::fabs(mean - threshold) < 1E-3;
    return false;
}

/*

    CITATION:
    This function was assited by chat.openai.com because of some errors with my update edges functions that I kept encountering.
    I used the promt "Can you please help me find the error which is preventing this function to work and please provide explanations" 
    
    Here was the code that had some errors:
    bool Graph::updateEdges(const std::string& seriesCode, double threshold, const std::string& relation) {
    bool addedAny = false;
    for (auto it1 = nodes.begin(); it1 != nodes.end(); ++it1) {
        for (auto it2 = nodes.begin(); it2 != nodes.end(); ++it2) { // MISTAKE: Starts it2 from begin()
            double mean1 = it1->second.ts->getMean(seriesCode);
            double mean2 = it2->second.ts->getMean(seriesCode);

            if (satisfiesCondition(mean1, threshold, relation) &&
                satisfiesCondition(mean2, threshold, relation)) {
                Relationship rel = {seriesCode, threshold, relation};
                
                // Only updates one direction (it1 to it2), missing symmetry
                adjList[it1->first][it2->first].relationships.insert(rel); // MISTAKE: No uniqueness check or reverse edge
                addedAny = true; // MISTAKE: Assumes addition without verifying
            }
        }
    }
    return addedAny;
}
    I realized the following problems:
    Starting it2 from nodes.begin() instead of std::next(it1) means you’d compare every country with every other.
    It only updated adjList[it1->first][it2->first], not the reverse it2->first to it1->first.

    Therfore the following updated code was used below which worked out.
*/

// UPDATE_EDGES command: iterate over all pairs of nodes.
bool Graph::updateEdges(const std::string& seriesCode, double threshold, const std::string& relation) {
    bool addedAny = false;
    // For each pair of countries.
    for (auto it1 = nodes.begin(); it1 != nodes.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != nodes.end(); ++it2) {
            // Retrieve the mean for the given seriesCode from each country.
            // it1->second is the Node, and ts is a pointer to its TimeSeries object.
            double mean1 = it1->second.ts->getMean(seriesCode);
            double mean2 = it2->second.ts->getMean(seriesCode);

            // Check if both satisfy the condition.
            if (satisfiesCondition(mean1, threshold, relation) &&
                satisfiesCondition(mean2, threshold, relation)) {

                // Create a Relationship tuple with the seriesCode, threshold, and relation.
                Relationship rel = {seriesCode, threshold, relation};

                // Update edge from it1->second to it2->second.
                bool newRel = false;
                if (adjList[it1->first][it2->first].relationships.find(rel) == adjList[it1->first][it2->first].relationships.end()) {
                    // If not found, insert the new relationship into the set.
                    adjList[it1->first][it2->first].relationships.insert(rel);
                    newRel = true;
                }
                // Ensure symmetry: update edge from it2->first to it1->first.
                if (adjList[it2->first][it1->first].relationships.find(rel) == adjList[it2->first][it1->first].relationships.end()) {
                    // If not found, insert it to maintain symmetry.
                    adjList[it2->first][it1->first].relationships.insert(rel);
                    newRel = true;
                }
                if (newRel)
                    addedAny = true;
            }
        }
    }
    return addedAny;
}

// ADJACENT command: Given a country code, list all adjacent (neighbor) countries.
std::vector<std::string> Graph::adjacent(const std::string& countryCode, bool &found) {
    std::vector<std::string> result;
    // Check if the country exists.
    if (nodes.find(countryCode) == nodes.end()) {
        found = false;
        return result;
    }
    found = true;
    // Look in the adjacency list for this country.
    if (adjList.find(countryCode) != adjList.end()) {
        // Iterate over all neighbors of countryCode in the adjacency list.
        for (auto& pair : adjList[countryCode]) {
            // Exclude self if it appears.
            if (pair.first != countryCode) {
                // Get the country name from nodes.
                result.push_back(nodes[pair.first].countryName);
            }
        }
    }
    return result;
}


/*
CITATION

        The path function was done using BFS algorithm which was implented using helpe of L16 lecture slides
        BFS (from, to)
                Queue todo
                Set visited
                todo.push(the path [from])
                as long as todo is not empty
                currentPath = todo.pop()
                currentNode = currentPath.lastNode()
                if(currentNode == to)
                return currentPath
                if (currentNode is not visited)
                visited.add(currentNode)
                for each (x adjacent to currentNode)
                todo.push(currentPath.addNode(x))
                return no valid path exists

        It was adjusted for the function requirment and my project logic. 


*/


// PATH command: Determine if a path exists between two countries using BFS.
bool Graph::path(const std::string& code1, const std::string& code2) {
    if (nodes.find(code1) == nodes.end() || nodes.find(code2) == nodes.end())
        return false;
    // Map to track visited countries, preventing cycles and redundant checks.
    std::unordered_map<std::string, bool> visited;
    std::queue<std::string> q;

    // Add the starting country to the queue and mark it as visited.
    q.push(code1);
    visited[code1] = true;

    // Continue BFS while there are countries to explore.
    while (!q.empty()) {
        // Get the current country from the front of the queue.
        std::string cur = q.front();
        q.pop();

        // Get the current country from the front of the queue.
        if (cur == code2)
            return true;
        // Check all adjacent nodes.
        if (adjList.find(cur) != adjList.end()) {
            
            // Iterate over all neighbors of the current country.
            for (auto& neighbor : adjList[cur]) {
                // If this neighbor hasn’t been visited yet, process it.
                if (!visited[neighbor.first]) {
                    visited[neighbor.first] = true;
                    q.push(neighbor.first);
                }
            }
        }
    }
    return false;
}

// RELATIONSHIPS command: Return the relationship tuples for the direct edge between two countries.
std::vector<Relationship> Graph::getRelationships(const std::string& code1, const std::string& code2) {
    std::vector<Relationship> result;

    // Check if code1 has any edges in the adjacency list.
    if (adjList.find(code1) != adjList.end()) {

        // Check if code2 is a neighbor of code1 in the inner map.
        if (adjList[code1].find(code2) != adjList[code1].end()) {
            // Iterate over the set of relationships for the edge from code1 to code2.
            for (const auto& rel : adjList[code1][code2].relationships) {
                // Add each relationship to the result vector
                result.push_back(rel);
            }
        }
    }
    return result;
}
