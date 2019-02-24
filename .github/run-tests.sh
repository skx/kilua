#!/bin/sh

apt-get update -qq
apt-get install -y make liblua5.2-dev

make
