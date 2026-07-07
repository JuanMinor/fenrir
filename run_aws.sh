#!/bin/bash
set -e

echo "============================================="
echo "   Fenrir AWS Training Setup Script          "
echo "============================================="

# Ensure we are in the fenrir directory
if [ ! -f "supervised/supervised.py" ]; then
    echo "Error: Please run this script from the root of the fenrir repository."
    exit 1
fi

echo "--> Step 1: Installing system dependencies..."
if command -v apt-get &> /dev/null; then
    sudo apt-get update -y
    sudo apt-get install -y python3-venv python3-pip zstd wget
elif command -v yum &> /dev/null; then
    sudo yum update -y
    sudo yum install -y python3-pip zstd wget
else
    echo "Unsupported package manager. Please install python3, pip, and zstd manually."
fi

echo "--> Step 2: Setting up Python Virtual Environment..."
python3 -m venv .venv
source .venv/bin/activate

echo "--> Step 3: Installing Python packages..."
pip install --upgrade pip
pip install -r requirements.txt

echo "--> Step 4: Downloading Lichess dataset..."
if [ ! -f "lichess_db_standard_rated_2013-01.pgn" ]; then
    if [ ! -f "lichess_db_standard_rated_2013-01.pgn.zst" ]; then
        echo "Downloading PGN file directly from Lichess to AWS..."
        wget https://database.lichess.org/standard/lichess_db_standard_rated_2013-01.pgn.zst
    fi
    echo "Decompressing PGN file..."
    zstd -d lichess_db_standard_rated_2013-01.pgn.zst --rm
else
    echo "Lichess dataset already exists!"
fi

echo "============================================="
echo "   Setup Complete! Starting Training...      "
echo "============================================="

# Run the training script, pointing it to the correct PGN file
python supervised/supervised.py lichess_db_standard_rated_2013-01.pgn
