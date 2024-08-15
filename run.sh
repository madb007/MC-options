#!/bin/bash

# Compile and Run Script for Monte Carlo Pricer

# Set your compiler and wxWidgets configuration here
COMPILER=g++
WXCONFIG=wx-config

# Compile the code
echo "Compiling..."
$COMPILER FinanceMonteCarlo.cpp -c -o FinanceMonteCarlo.o `$WXCONFIG --cxxflags`
$COMPILER MonteCarloApp.cpp FinanceMonteCarlo.o -o MonteCarloApp `$WXCONFIG --libs --cxxflags`

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# Run the executable
echo "Running MonteCarloApp..."
./MonteCarloApp
