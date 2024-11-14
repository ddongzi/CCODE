#!/bin/bash

echo "Hello!"
make
rm my.db
rspec test.rb
