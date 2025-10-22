#ifndef SCREEN_CONFIG_H
#define SCREEN_CONFIG_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

struct ScreenConfig {
    // Screen identification
    uint8_t screen_id;
    
    // Matrix configuration
    int rows;
    int cols;
    int chain_length;
    int parallel;
    std::string hardware_mapping;
    std::string pixel_mapper;
    int gpio_slowdown;
    
    // Serial configuration
    std::string serial_port;
    int serial_baudrate;
    
    // Display options
    bool show_diagnostics;
    
    // Default constructor with default values for 192x192 screen (ID=1)
    ScreenConfig() 
        : screen_id(1)
        , rows(64)
        , cols(64)
        , chain_length(3)
        , parallel(3)
        , hardware_mapping("regular")
        , pixel_mapper("")
        , gpio_slowdown(3)
        , serial_port("/dev/ttyUSB0")
        , serial_baudrate(1000000)
        , show_diagnostics(true)
    {}
    
    // Load configuration from INI file
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << filename << std::endl;
            return false;
        }
        
        std::string line;
        std::string current_section;
        
        while (std::getline(file, line)) {
            // Remove leading/trailing whitespace
            line = trim(line);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            
            // Check for section header
            if (line[0] == '[' && line[line.length()-1] == ']') {
                current_section = line.substr(1, line.length()-2);
                continue;
            }
            
            // Parse key = value
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));
                
                // Parse configuration values
                if (key == "screen_id") {
                    screen_id = std::stoi(value);
                } else if (key == "rows") {
                    rows = std::stoi(value);
                } else if (key == "cols") {
                    cols = std::stoi(value);
                } else if (key == "chain_length") {
                    chain_length = std::stoi(value);
                } else if (key == "parallel") {
                    parallel = std::stoi(value);
                } else if (key == "hardware_mapping") {
                    hardware_mapping = value;
                } else if (key == "pixel_mapper") {
                    pixel_mapper = value;
                } else if (key == "gpio_slowdown") {
                    gpio_slowdown = std::stoi(value);
                } else if (key == "serial_port") {
                    serial_port = value;
                } else if (key == "serial_baudrate") {
                    serial_baudrate = std::stoi(value);
                } else if (key == "show_diagnostics") {
                    show_diagnostics = (value == "true" || value == "1" || value == "yes");
                }
            }
        }
        
        file.close();
        return true;
    }
    
    // Print configuration
    void print() const {
        std::cout << "=== Screen Configuration ===" << std::endl;
        std::cout << "Screen ID: " << (int)screen_id << std::endl;
        std::cout << "Matrix: " << rows << "x" << cols << std::endl;
        std::cout << "Chain length: " << chain_length << std::endl;
        std::cout << "Parallel: " << parallel << std::endl;
        std::cout << "Total resolution: " << (cols * chain_length) << "x" << (rows * parallel) << std::endl;
        std::cout << "Hardware mapping: " << hardware_mapping << std::endl;
        std::cout << "Pixel mapper: " << (pixel_mapper.empty() ? "(none)" : pixel_mapper) << std::endl;
        std::cout << "GPIO slowdown: " << gpio_slowdown << std::endl;
        std::cout << "Serial port: " << serial_port << " @ " << serial_baudrate << " baud" << std::endl;
        std::cout << "Show diagnostics: " << (show_diagnostics ? "yes" : "no") << std::endl;
        std::cout << "============================" << std::endl;
    }
    
private:
    // Helper function to trim whitespace
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, last - first + 1);
    }
};

#endif // SCREEN_CONFIG_H

