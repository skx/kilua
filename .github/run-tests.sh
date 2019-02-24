#!/bin/sh

# Update
apt-get update -qq

# Install dependencies
apt-get install -y make liblua5.2-dev

# Compile
make
