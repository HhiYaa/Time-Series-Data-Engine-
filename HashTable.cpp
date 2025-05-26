#include "HashTable.h"
#include "Binary.h"
#include "TimeSeries.h"
#include <cmath>

HashTable::~HashTable() {
    // not deleting table[i].ts since they are not allocated by the hash table.
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (table[i].status == OCCUPIED && table[i].owned && table[i].ts != nullptr) {
            delete table[i].ts;  // Free the dynamically allocated object.
            table[i].ts = nullptr;
        }
    }
}

// Converts a 3-letter country code to an integer using base‑26 conversion.
unsigned int HashTable::convertCountryCode(const std::string& code) const {
    if (code.length() != 3) return 0; // In production, handle error appropriately.
    unsigned int W = (code[0] - 'A') * 26 * 26 + (code[1] - 'A') * 26 + (code[2] - 'A');
    return W;
}

// Primary hash function: index = W mod TABLE_SIZE.
int HashTable::hashPrimary(unsigned int W) const {
    return W % TABLE_SIZE;
}

// Secondary hash function: based on floor(W / TABLE_SIZE). If even, add 1 to ensure an odd value.
int HashTable::hashSecondary(unsigned int W) const {
    int h2 = (W / TABLE_SIZE) % TABLE_SIZE;
    if (h2 % 2 == 0) {
        h2 = h2 + 1;
    }
    return h2;
}

   /*CITATION: 
					This function was assisted by chat.openai.com with the prompt: "Can you please enhance help enhance my for loop to look 
                    up by the countrycode" 
					 
						The funtion provided: 
						 if (table[index].status == EMPTY) {
                        return std::make_pair(-1, steps);
                        } else if (table[index].status == OCCUPIED && table[index].countryCode == countryCode) {
                            return std::make_pair(index, steps);
                        }


    */

// Lookup function uses double hashing. Returns (index, steps) if found; (-1, steps) if not.
std::pair<int, int> HashTable::lookup(const std::string& countryCode) const {
    unsigned int W = convertCountryCode(countryCode);
    int h1 = hashPrimary(W);
    int h2 = hashSecondary(W);
    int index;
    int steps = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        index = (h1 + i * h2) % TABLE_SIZE;
        steps++;
        if (table[index].status == EMPTY) {
            return std::make_pair(-1, steps);
        } else if (table[index].status == OCCUPIED && table[index].countryCode == countryCode) {
            return std::make_pair(index, steps);
        }
    }
    return std::make_pair(-1, steps);
}

 /*CITATION: 
					This function was assisted by chat.openai.com with the prompt: "Can you please help how to store the 
                    address properly with global object" 
					 
						Provided: 
						table[index].ts = &ts;  // Store the address; assumes ts comes from a global or persistent object.


    */


// Insert the country into the hash table if it does not already exist.
// Instead of copying the TimeSeries, store a pointer to it.
//bool HashTable::insert(const std::string& countryCode, const TimeSeries& ts) {
    bool HashTable::insert(const std::string& countryCode, const TimeSeries& ts, bool owned){  
    auto res = lookup(countryCode);
    if (res.first != -1) {
        return false; // Country already exists.
    }
    unsigned int W = convertCountryCode(countryCode);
    int h1 = hashPrimary(W);
    int h2 = hashSecondary(W);
    int index;
    for (int i = 0; i < TABLE_SIZE; i++) {
        index = (h1 + i * h2) % TABLE_SIZE;
        if (table[index].status == EMPTY || table[index].status == DELETED) {
            table[index].countryCode = countryCode;
            table[index].ts = &ts;  // Store the address; assumes ts comes from a global or persistent object.
            table[index].status = OCCUPIED;
            table[index].owned = owned; // Mark ownership.

            return true;
        }
    }
    return false; // Table is full.
}

// Remove the country by marking its slot as DELETED.
bool HashTable::remove(const std::string& countryCode, BinaryTree& db) {
    auto res = lookup(countryCode);
    if (res.first == -1) {
        return false; 
    }
    int index = res.first;
    table[index].status = DELETED;

    if (db.getRoot() != nullptr && table[index].ts != nullptr) {
        // Retrieve the full country name using getCountryName() from the TimeSeries or Country class.
        std::string fullCountryName = table[index].ts->getCountryName();
        db.deleteCountry(fullCountryName);
    } 

    return true;
}
