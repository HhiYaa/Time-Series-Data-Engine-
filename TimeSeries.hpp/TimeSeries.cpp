#include "TimeSeries.h"
#include "Binary.h"
#include <fstream>
#include <sstream>

const int MAX_COUNTRIES = 512; // Maximum countries allowed
extern TimeSeries countries[MAX_COUNTRIES]; // Array of TimeSeries objects
int countryCount = 0;
#define DBL_LARGE 1e9  // A very large value
#define DBL_SMALL -1e9 // A very small value


TimeSeries TimeSeries::instance;

TimeSeries& TimeSeries::getInstance() {
    return instance;
}

// -------- TimeSeriesData Implementation --------
TimeSeriesData::TimeSeriesData() : Cap(10), Size(0) {
    arrY = new int[Cap];
    arrD = new double[Cap];
    Valid = new bool[Cap] {false};
}

TimeSeriesData::~TimeSeriesData() {
    delete[] arrY;
    delete[] arrD;
    delete[] Valid;
}


void TimeSeriesData::Resize(int newCap) {
    int* newArrY = new int[newCap];
    double* newArrD = new double[newCap];
    bool* newValid = new bool[newCap]{false};
    
    for (int i = 0; i < Size; i++) {
        newArrY[i] = arrY[i];
        newArrD[i] = arrD[i];
        newValid[i] = Valid[i];
    }
    
    delete[] arrY;
    delete[] arrD;
    delete[] Valid;
    arrY = newArrY;
    arrD = newArrD;
    Valid = newValid;
    Cap = newCap;
}

void TimeSeriesData::ResizeEqual(int year, double data) {
    if (Size == Cap) {
        Resize(Cap * 2);
    }
    arrY[Size] = year;
    arrD[Size] = data;
    Valid[Size] = true;
    Size++;
}

double TimeSeriesData::ComputeMean() const {
    if (Size == 0) return -1;

    double sum = 0;
    int count = 0;

    for (int i = 0; i < Size; i++) {
        if (Valid[i] && arrD[i] >= 0) {
            sum += arrD[i];
            count++;
        }
    }
    if (count == 0) return -1;  // No valid values, return 0
    return sum / count;  // Compute the mean

}

// -------- TimeSeries Implementation --------
TimeSeries::TimeSeries(std::string country, std::string c_code, std::string series, std::string s_code) {
    Country_name = country;
    country_code = c_code;
    head = new TimeSeriesNode(country, c_code, series, s_code);
}


TimeSeries::TimeSeries() {
    head = nullptr;
    Country_name = "";
    country_code = "";
}

TimeSeries::~TimeSeries() {
    TimeSeriesNode* current = head;
    while (current) {
        TimeSeriesNode* temp = current;
        current = current->next;
        delete temp; //  Ensure each node is deleted exactly once
    }
}

void TimeSeries::add(std::string country, std::string c_code, std::string series, std::string s_code, int year, double value) {
    TimeSeriesNode* newNode = new TimeSeriesNode(country, c_code, series, s_code);
    newNode->timeseriesData.ResizeEqual(year, value);
    newNode->next = head;
    head = newNode;
}

bool TimeSeries::addData(std::string s_code, int year, double value) {
    TimeSeriesNode* temp = head;
    while (temp) {
        if (temp->series_code == s_code) {
            temp->timeseriesData.ResizeEqual(year, value);
            return true;
        }
        temp = temp->next;
    }
    return false;
}

std::string TimeSeries::print(std::string countryName){

    for (int i = 0; i < countryCount; i++) {
        if (countries[i].Country_name == countryName) {
            std::cout << countries[i].getCountryName() << " " << countries[i].getCountryCode() << " ";

            TimeSeriesNode* current = countries[i].head; // do I need get head?? 
            if (!current) {
                return "";
            }
            while (current) {
                std::cout << current->Series << " ";
                current = current->next;
            }
            return "";
        }
    }
    return " ";
}



// LOAD:  Reads data from the csv file and stores it the linked list 
void TimeSeries::load(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    std::string line; // String to store each line from the file 
    bool loaded = false;

    // Reads line by line to store the csv file data 
    while (std::getline(file, line)) {
        std::istringstream ss(line); // stream to process the line
        std::string Country, countryCode, Series, series_code, data; // Variables to store all the parsed metadata fields 
        int year = 1960; 

    // Extract metadata fields
    if (!std::getline(ss, Country, ',') ||
        !std::getline(ss, countryCode, ',') ||
        !std::getline(ss, Series, ',') ||
        !std::getline(ss, series_code, ',')) {
        continue;
        }

        // Find if country already exists
        int existingIndex = -1;
        for (int i = 0; i < countryCount; i++) {
            if (countries[i].Country_name == Country) {
                existingIndex = i;
                break;
            }
        }

        // If country does not exist, add a new entry
        if (existingIndex == -1) {

            if (countryCount >= 512) {
            }

            countries[countryCount].setData(Country, countryCode);
            countries[countryCount].head = new TimeSeriesNode(Country, countryCode, Series, series_code);
            existingIndex = countryCount;
            countryCount++;
        }
        else {
            TimeSeriesNode* newNode = new TimeSeriesNode(Country, countryCode, Series, series_code);
            TimeSeriesNode* temp = countries[existingIndex].head;
            while (temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = newNode; // Append at the end
        }


        while (std::getline(ss, data, ',')) {
            if (data != "-1") {
                double value = std::stod(data); // Convert string to double (the actual data value)
                TimeSeriesNode* temp = countries[existingIndex].head;
                while (temp) {
                    if (temp->series_code == series_code) {
                        temp->timeseriesData.ResizeEqual(year, value); // Add data to the matching node
                        break;
                    }
                    temp = temp->next;
                }
                year++; // Move to next year
            }
        }
        loaded = true;
    }

    file.close(); // Close the file after reading through all the data 

}

	/*CITATION: 
					This function was assisted by chat.openai.com with the prompt: "Can you please enhance my
					while loop that matches the country and adds the given data. The add function is not working
					can you please assist me." 
					 
						The funtion provided: 
						while(current){
									if(current->Country == country){
										current->timeseriesData.ResizeEqual(year, value); // Update the existing data 
										return;
									} 
									prev = current;
									current = current->next;
								}	
					I realized that I was only matching it with country and forgot to also consider 
					series.

		*/


void TimeSeries::setData(std::string Country_name, std::string country_code) {
    this->Country_name = Country_name;
    this->country_code = country_code;
}


void TimeSeries::range(std::string series_code) {
    double minMean = 1e9; // Initialize to 0 because no data should be negative
    double maxMean = -1e9;
    bool foundValidCountry = false;

    for (int i = 0; i < countryCount; i++) { // Loop through all countries
        TimeSeriesNode* current = countries[i].head;


        while (current) { 
            if (current->series_code == series_code) { // Match series code
                // Access the TimeSeriesData and compute the mean
                double mean = current->timeseriesData.ComputeMean(); // Use provided mean function

                if (mean > 0) { // Ensure valid data only
                    if (!foundValidCountry) { 
                        // First valid mean found, initialize min/max
                        minMean = mean;
                        maxMean = mean;
                        foundValidCountry = true;

                    } else {
                        if (mean < minMean  - 1e-9) {
                            minMean = mean;
                        }
                        if (mean > maxMean  + 1e-9) {
                            maxMean = mean;
                        }
                    }
                } else {
                }
            }

            current = current->next; // Move to next series
        }
    }

    if (!foundValidCountry) {
        return;
    }
    std::cout << minMean << " " << maxMean;
}



void TimeSeries::build(std::string seriesCode, std::string countryNames[], double means[], int &validCountryCount) {
    validCountryCount = 0;

    // Iterate over all loaded countries, not just those with data
    for (int i = 0; i < countryCount; i++) {
        double mean = 0.0;
        bool hasValidData = false;

        std::string countryName = countries[i].getCountryName();

        // Check if the country has any time series data
        TimeSeriesNode* current = countries[i].getHead();
        if (current == nullptr) {
            // No data at all, assign mean = 0.0
        } else {
            // Look for the specific seriesCode
            while (current != nullptr) {
                if (current->series_code == seriesCode) {
                    mean = current->timeseriesData.ComputeMean();
                    if (mean > 0) {
                        hasValidData = true;
                    }
                    break;
                }
                current = current->next;
            }
            if (!hasValidData) {
                mean = 0.0;
            }
        }

        // Add every country, regardless of data
        countryNames[validCountryCount] = countryName;
        means[validCountryCount] = mean;
        validCountryCount++;

    
    }
}


double TimeSeries::getMean(const std::string& seriesCode) const {
    TimeSeriesNode* current = head;
    while (current) {
        if (current->series_code == seriesCode) {
            return current->timeseriesData.ComputeMean();
        }
        current = current->next;
    }
    return -1; // Series not found or invalid.
}
