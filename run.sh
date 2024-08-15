#!/bin/sh

# Monte Carlo GUI Build and Run Script

# Set compiler and flags
COMPILER=g++
STD_VERSION=c++17
WXCONFIG=wx-config

# Compile the code
echo "Compiling..."
$COMPILER -std=$STD_VERSION MonteCarloGUI.cpp FinanceMonteCarlo.cpp `$WXCONFIG --cxxflags --libs` -o MonteCarloGUI

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed. Please check the error messages above."
    exit 1
fi

echo "Compilation successful!"

# Run the executable
echo "Running MonteCarloGUI..."
./MonteCarloGUI

# Check if the program ran successfully
if [ $? -ne 0 ]; then
    echo "Program execution failed. Please check the error messages above."
    exit 1
fi

echo "Program execution completed."
