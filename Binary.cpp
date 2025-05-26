#include "Binary.h"
#include "TimeSeries.h"
#include <fstream>
#include <sstream>
#include <cfloat>
#define DBL_LARGE 1e9  // A very large value
#define DBL_SMALL -1e9 // A very small value


TimeSeries countries[MAX_SIZE];

TreeNode::TreeNode(std::string countryName, double meanValue, TreeNode* left, TreeNode* right)
    : countryName(countryName), meanValue(meanValue), left(left), right(right) {}


BinaryTree::BinaryTree(){
    root = nullptr;
    countryCount = 0;

    // Initialize each TimeSeries object in the array
    for (int i = 0; i < MAX_SIZE; i++) {
        countries[i] = TimeSeries();
    }
}

BinaryTree::~BinaryTree() { deleteTree(root); }

std::string BinaryTree::load(std::string countryname) const{
    TimeSeries ts;  // Create an instance of TimeSeries
    ts.load(countryname);
    return "success"; 
}

std::string BinaryTree::print(std::string name) {
    TimeSeries ts;  // Create an instance of TimeSeries
    ts.print(name);
    return " "; 
}

std::string BinaryTree::range(std::string series_code) {
    TimeSeries ts;  // Create an instance of TimeSeries
    ts.range(series_code);
    return " "; 
}

void BinaryTree::exit() {
    //commented to prevent memory leaks 
   // std::exit(0);
}

void BinaryTree::deleteTree(TreeNode* node) { 
    if (node != nullptr) {
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
}

void BinaryTree::initializeRoot(double maxMean, double minMean) {
    root = new TreeNode("Root", maxMean);  // Root represents full range

    // Store all countries initially in root
    for (int i = 0; i < countryCount; i++) {
        TimeSeriesNode* current = countries[i].getHead();

        while (current != nullptr) {
            double mean = current->timeseriesData.ComputeMean();
            if (mean != -1) {  // Ignore invalid means
                root->countries[root->countryCount++] = countries[i].getCountryName();
            }
            current = current->next;
        }
    }
}


// Helper function to insert a node into BST
TreeNode* BinaryTree::insert(TreeNode* node, std::string countryName, double meanValue) {
    if (node == nullptr) {
        return new TreeNode(countryName, meanValue);
    }

    if (meanValue < node->meanValue) {
        node->left = insert(node->left, countryName, meanValue);
    } else {
        node->right = insert(node->right, countryName, meanValue);
    }

    return node;
}

        /*CITATION: 
					This function was assisted by chat.openai.com with the prompt: "Can you please enhance help debug my build and make sure
                    it manipulayes the mean properly." 
					 
						The funtion provided: 
						 TreeNode* zeroMeanRoot = nullptr;
                    if (zeroEnd >= start) {
                        std::string zeroMeanName = "";
                        for (int i = start; i <= zeroEnd; ++i) {
                            if (!zeroMeanName.empty()) zeroMeanName += ", ";
                            zeroMeanName += countryNames[i];
                        }
                        zeroMeanRoot = new TreeNode(zeroMeanName, 0.0);
                        start = zeroEnd + 1;
                    }


		*/


TreeNode* BinaryTree::buildTree(std::string countryNames[], double means[], int start, int end, double minRange, double maxRange) {
    if (start > end) return nullptr;

    // Step 1: Collect all zero-mean countries
    int zeroEnd = start - 1;
    while (zeroEnd + 1 <= end && means[zeroEnd + 1] == 0.0) zeroEnd++;

    // Step 2: If all values in the range are zero, create a single node
    if (zeroEnd == end) { 
        //std::cout << "DEBUG: Creating single leaf for all zero-mean countries.\n";
        std::string zeroMeanName = "";
        for (int i = start; i <= end; ++i) {
            if (!zeroMeanName.empty()) zeroMeanName += ", ";
            zeroMeanName += countryNames[i];
        }
        return new TreeNode(zeroMeanName, 0);
    }

    // Step 3: Store zero-mean countries separately in the leftmost node
    TreeNode* zeroMeanRoot = nullptr;
    if (zeroEnd >= start) {
        //std::cout << "DEBUG: Assigning zero-mean countries to single leaf.\n";
        std::string zeroMeanName = "";
        for (int i = start; i <= zeroEnd; ++i) {
            if (!zeroMeanName.empty()) zeroMeanName += ", ";
            zeroMeanName += countryNames[i];
        }
        zeroMeanRoot = new TreeNode(zeroMeanName, 0);
        start = zeroEnd + 1; // Move start past zero-means
    }

    // Step 4: Handle cases where only **one** non-zero country remains
    if (start == end) {
       // std::cout << "DEBUG: Single country leaf: " << countryNames[start] << " (" << means[start] << ")\n";
        return new TreeNode(countryNames[start], means[start]);
    }

    // Step 5: Check if all remaining values are equal (within tolerance)
    bool allEqual = true;
    for (int i = start + 1; i <= end; ++i) {
        if (std::abs(means[i] - means[start]) > 1E-3) {
            allEqual = false;
            break;
        }
    }

    if (allEqual || std::fabs(maxRange - minRange) < 1E-3) {
       // std::cout << "DEBUG: Grouping equal-mean countries into one leaf: ";
        std::string groupedName = "";
        for (int i = start; i <= end; ++i) {
            if (!groupedName.empty()) groupedName += ", ";
            groupedName += countryNames[i];
          //  std::cout << countryNames[i] << " (" << means[i] << ") ";
        }
       // std::cout << "\n";

        TreeNode* leaf = new TreeNode(groupedName, means[start]);

          // Ensure zero-mean node is placed in the **deepest** leftmost node
          if (zeroMeanRoot) {
            TreeNode* temp = leaf;
            while (temp->left) temp = temp->left; // Traverse leftmost
            temp->left = zeroMeanRoot;
        }
        return leaf;

    }

    //  **Fix midpoint selection** to ensure small values are correctly placed
    double midRange = (minRange + maxRange) / 2.0;
    int mid = start;
    while (mid <= end && means[mid] < midRange) mid++;

    if (mid > end || mid == start) mid = (start + end) / 2; // Adjust mid to avoid skipping small values
    if (means[mid] < 1E-3) mid = start; // Ensure smallest values are included

    //  **Prevent empty left subtree**
    if (mid - 1 < start) mid = start + 1;

    //std::cout << "DEBUG: Splitting range [" << minRange << ", " << maxRange << "] at " << midRange << std::endl;

    TreeNode* node = new TreeNode("ALL_COUNTRIES [" + std::to_string(minRange) + "," + std::to_string(maxRange) + "]",
                                  midRange,
                                  buildTree(countryNames, means, start, mid - 1, minRange, midRange),
                                  buildTree(countryNames, means, mid, end, midRange, maxRange));

        if (zeroMeanRoot) {
            // Traverse to the leftmost node of the entire tree
            TreeNode* temp = node;
            while (temp->left) {
                temp = temp->left;
            }
            // Attach zeroMeanRoot to the leftmost node
            temp->left = zeroMeanRoot;
        }
return node;
}


 /*CITATION: 
					This function was assisted by chat.openai.com with the prompt: "Can you please help me how to access all the data
                    from  mmy time series into the binary because I am having many errors with my current approach in the build 
                    bcause it is not being added correctly and please enhance it" 
					 
						The chat added the following things:
                        TimeSeries::getInstance().build(seriesCode, countryNames, means, validCountryCount);

                         for (int i = 0; i < validCountryCount - 1; i++) {
                        for (int j = i + 1; j < validCountryCount; j++) {
                            if (means[i] > means[j]) {
                                std::swap(means[i], means[j]);
                                std::swap(countryNames[i], countryNames[j]);
                            }
                        }
                    }



		*/

void BinaryTree::build(std::string seriesCode) {
    if (root != nullptr) {
        deleteTree(root); // Fix: Use deleteTree, not BinaryTree(root)
    }
    root = nullptr;

    std::string countryNames[MAX_SIZE];
    double means[MAX_SIZE];
    int validCountryCount = 0;

    TimeSeries::getInstance().build(seriesCode, countryNames, means, validCountryCount);

    if (validCountryCount == 0) return;

    for (int i = 0; i < validCountryCount - 1; i++) {
        for (int j = i + 1; j < validCountryCount; j++) {
            if (means[i] > means[j]) {
                std::swap(means[i], means[j]);
                std::swap(countryNames[i], countryNames[j]);
            }
        }
    }

    double minMean = means[0];
    for (int i = 0; i < validCountryCount; ++i) {
        if (means[i] > 0) {
            minMean = means[i];
            break;
        }
    }
    double maxMean = means[validCountryCount - 1];

    root = buildTree(countryNames, means, 0, validCountryCount - 1, minMean, maxMean);
 
}


    /*CITATION: 
					This function was assisted by chat.openai.com with the prompt: "I am having trouble with workingw itnb mean 0 can you please
                    help to approach it properly. All the countries are not being printed properly." 
					 
						The chat added the following things:
                        double tolerance = 1E-3;

                        if (node->countryName.find("ALL_COUNTRIES [") != std::string::npos) {
                            findHelper(node->left, mean, operation, result);
                            findHelper(node->right, mean, operation, result);
                            return;
                        }


// 		*/

void BinaryTree::findHelper(TreeNode* node, double targetMean, const std::string& operation, std::string& result) {
    if (node == nullptr)
        return;

    double diff = std::fabs(node->meanValue - targetMean);

    // Case 1: Leaf Node -> Perform Check
    if (node->left == nullptr && node->right == nullptr) {
        bool match = false;

        if (operation == "less" && node->meanValue < targetMean) {
            match = true;
        } 
        else if (operation == "equal" && diff <= 1E-3) {  
            match = true;
        } 
        else if (operation == "greater" && node->meanValue > targetMean) {
            match = true;
        }

        if (match) {
            std::istringstream iss(node->countryName);
            std::string token;
            while (std::getline(iss, token, ',')) {
                while (!token.empty() && token[0] == ' ')  
                    token.erase(0, 1);  
                if (!result.empty())
                    result += " ";
                result += token;
            }
        }
        return;
    }
    if (operation == "less") {
        findHelper(node->left, targetMean, operation, result);
        findHelper(node->right, targetMean, operation, result);
    } 
    else if (operation == "greater") {
        findHelper(node->right, targetMean, operation, result);
        findHelper(node->left, targetMean, operation, result);
    } 
    else if (operation == "equal") {
        findHelper(node->left, targetMean, operation, result);
        findHelper(node->right, targetMean, operation, result);

        if (diff <= 1E-3 || (node->meanValue == 0.0 && std::fabs(targetMean) < 1E-3)) {
            std::istringstream iss(node->countryName);
            std::string token;
            while (std::getline(iss, token, ',')) {
                while (!token.empty() && token[0] == ' ')  
                    token.erase(0, 1);  
                if (!result.empty())
                    result += " ";
                result += token;
            }
        }
    }
}


    // FIND command: outputs the names of all countries whose time series means satisfy the operation.
    // For "equal", the tolerance is 1E-3.
    void BinaryTree::find(double targetMean, const std::string& operation) {
        // Validate operation string. Only accept "less", "equal", or "greater".
        if (operation != "less" && operation != "equal" && operation != "greater") {
            //std::cout << "failure" << std::endl;
            return;
        }

        if (root == nullptr) {
            std::cout << "failure" << std::endl;
            return;
        }
        // Instead of using vector, we accumulate the matching country names in a single string.
        std::string result;
        findHelper(root, targetMean, operation, result);
        // Output the result (if no match, this will be a blank line).
       //// std::cout << result << std::endl;
       if (result.empty()) {
        //std::cout << "failure" << std::endl;
       // std::cout << "No matching countries found." << std::endl;
    } else {
        std::cout << result << std::endl;
    }
    }


// Helper function to find the maximum node in a subtree (used in deletion).
TreeNode* BinaryTree::findMax(TreeNode* node) {
    while (node && node->countryName.find("ALL_COUNTRIES [") != std::string::npos) {
        node = node->right; // Keep moving right to find max value node.
    }
    return node;
}




 /*CITATION: 
					This function was assisted by chat.openai.com with the prompt: "Can you please provcheck and ehnance my
                     a recursive delete function" 
					 
						The chat added the following things:
                        if (node->countryName.find(", ") != std::string::npos) {
            std::string countries = node->countryName;
            size_t pos = countries.find(country_name);
            if (pos != std::string::npos) {
                found = true;
                std::cout << "DEBUG: Found `" << country_name << "` in `" << countries << "`\n";
                size_t end = countries.find(',', pos);
                if (end == std::string::npos) end = countries.length();
                if (pos > 0 && countries[pos - 2] == ',') pos -= 2; // Adjust for ", "
                countries.erase(pos, end - pos + (pos > 0 ? 2 : 0));

                // If the node is now empty, delete it.
                if (countries.empty()) {
                    std::cout << "DEBUG: Leaf emptied, deleting\n";
                    delete node;
                    return nullptr;
                }
      */
      TreeNode* BinaryTree::deleteCountryHelper(TreeNode* node, const std::string& country_name, bool& found) {
        if (!node) return nullptr;
    
        // If this is a leaf node.
        if (node->left == nullptr && node->right == nullptr) {
            // If the node stores multiple countries.
            if (node->countryName.find(", ") != std::string::npos) {
                std::istringstream iss(node->countryName);
                std::string token;
                std::string newNames;
                bool first = true;
                bool removed = false;
                while (std::getline(iss, token, ',')) {
                    // Trim token
                    token.erase(0, token.find_first_not_of(" "));
                    token.erase(token.find_last_not_of(" ") + 1);
                    if (token == country_name) {
                        removed = true;
                        continue;
                    }
                    if (!first) newNames += ", ";
                    newNames += token;
                    first = false;
                }
                if (removed) {
                    found = true;
                    if (newNames.empty()) {
                        delete node;
                        return nullptr;
                    }
                    node->countryName = newNames;
                }
                return node;
            }
            // If the node holds a single country.
            else if (node->countryName == country_name) {
                found = true;
                delete node;
                return nullptr;
            }
            return node;
        }
    
        // Recurse into left and right subtrees.
        node->left = deleteCountryHelper(node->left, country_name, found);
        node->right = deleteCountryHelper(node->right, country_name, found);
    
        // Also, if this node's countryName holds multiple names, remove country_name from it.
        if (node->countryName.find(", ") != std::string::npos) {
            std::istringstream iss(node->countryName);
            std::string token;
            std::string newNames;
            bool first = true;
            bool removed = false;
            while (std::getline(iss, token, ',')) {
                token.erase(0, token.find_first_not_of(" "));
                token.erase(token.find_last_not_of(" ") + 1);
                if (token == country_name) {
                    removed = true;
                    continue;
                }
                if (!first) newNames += ", ";
                newNames += token;
                first = false;
            }
            if (removed) {
                found = true;
                if (newNames.empty()) {
                    // If no country remains in this node, handle as a deletion case.
                    // If both children are null, delete the node.
                    if (!node->left && !node->right) {
                        delete node;
                        return nullptr;
                    }
                    // If one subtree exists, promote it.
                    if (node->left && !node->right) {
                        TreeNode* temp = node->left;
                        delete node;
                        return temp;
                    }
                    if (!node->left && node->right) {
                        TreeNode* temp = node->right;
                        delete node;
                        return temp;
                    }
                    // If both children exist, use the maximum from the left subtree as a replacement.
                    TreeNode* maxLeft = findMax(node->left);
                    node->countryName = maxLeft->countryName;
                    node->meanValue = maxLeft->meanValue;
                    node->left = deleteCountryHelper(node->left, maxLeft->countryName, found);
                } else {
                    node->countryName = newNames;
                }
            }
        }
        // If this node holds a single country and it matches.
        else if (node->countryName == country_name) {
            found = true;
            // If node has no children, delete it.
            if (!node->left && !node->right) {
                delete node;
                return nullptr;
            }
            // If only one child exists, promote that child.
            if (node->left && !node->right) {
                TreeNode* temp = node->left;
                delete node;
                return temp;
            }
            if (!node->left && node->right) {
                TreeNode* temp = node->right;
                delete node;
                return temp;
            }
            // If both children exist, find the in-order predecessor.
            TreeNode* maxLeft = findMax(node->left);
            node->countryName = maxLeft->countryName;
            node->meanValue = maxLeft->meanValue;
            node->left = deleteCountryHelper(node->left, maxLeft->countryName, found);
        }
    
        return node;
    }    


void BinaryTree::deleteCountry(std::string country_name) {
    if (!root) {
       // std::cout << "DEBUG: Tree is empty. Cannot delete `" << country_name << "`\n";
        std::cout << "failure\n";
        return;
    }
    bool found = false;
    root = deleteCountryHelper(root, country_name, found);
    if (found) {
        //std::cout << "success\n";
    } else {
        std::cout << "failure\n";
    }
}




// ------------------LIMITS-------------------------------
TreeNode* BinaryTree::findLowestLeaf(TreeNode* node) const {
    if (!node) return nullptr;
    TreeNode* lowest = nullptr;
    // If this is a leaf, consider it.
    if (!node->left && !node->right)
        lowest = node;
    // Otherwise, search both subtrees.
    TreeNode* leftLowest = findLowestLeaf(node->left);
    TreeNode* rightLowest = findLowestLeaf(node->right);
    if (leftLowest && (!lowest || leftLowest->meanValue < lowest->meanValue))
        lowest = leftLowest;
    if (rightLowest && (!lowest || rightLowest->meanValue < lowest->meanValue))
        lowest = rightLowest;
    return lowest;
}

TreeNode* BinaryTree::findHighestLeaf(TreeNode* node) const {
    if (!node) return nullptr;
    TreeNode* highest = nullptr;
    if (!node->left && !node->right)
        highest = node;
    TreeNode* leftHighest = findHighestLeaf(node->left);
    TreeNode* rightHighest = findHighestLeaf(node->right);
    if (leftHighest && (!highest || leftHighest->meanValue > highest->meanValue))
        highest = leftHighest;
    if (rightHighest && (!highest || rightHighest->meanValue > highest->meanValue))
        highest = rightHighest;
    return highest;
}

std::string BinaryTree::Limits(std::string condition) const {
    if (!root) return "failure";
    if (condition != "lowest" && condition != "highest") {
        //std::cout << "failure" << std::endl;
        return "";
    }

    TreeNode* node = nullptr;
    
    if (condition == "lowest") {
        // Find the leaf with the smallest mean
        node = findLowestLeaf(root);
    } else if (condition == "highest") {
        // Find the leaf with the highest mean
        node = findHighestLeaf(root);
    } else {
        return "";
    }
    
    if (!node) return "failure";
    
    // Extract the country names from the node (splitting by commas)
    std::string output;
    if (node->countryName.find(", ") != std::string::npos) {
        std::string countries = node->countryName;
        std::string country;
        std::istringstream ss(countries);
        while (std::getline(ss, country, ',')) {
            // Trim leading and trailing spaces
            country.erase(0, country.find_first_not_of(" "));
            country.erase(country.find_last_not_of(" ") + 1);
            if (!output.empty()) output += " ";
            output += country;
        }
    } else {
        output = node->countryName;
    }
    
    return output;
}
