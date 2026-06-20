#!/usr/bin/env zsh

# Since I have to fill out the table., 
# I used this script file to generate 72 different combinations of output
# at a go for ease of use. Because, executing all combinations in terminal individually 
# takes a lot of time.

while IFS= read -r cmd; do
    
    [[ -z "$cmd" ]] && continue

    output=$(eval "$cmd" 2>&1)

    echo "$cmd -> $output"

done << 'EOF'
./main Trace1.txt S 64 4
./main Trace1.txt S 64 16
./main Trace1.txt S 64 32
./main Trace1.txt S 64 64
./main Trace1.txt S 128 4
./main Trace1.txt S 128 16
./main Trace1.txt S 128 32
./main Trace1.txt S 128 64
./main Trace1.txt S 256 4
./main Trace1.txt S 256 16
./main Trace1.txt S 256 32
./main Trace1.txt S 256 64
./main Trace2.txt S 64 4
./main Trace2.txt S 64 16
./main Trace2.txt S 64 32
./main Trace2.txt S 64 64
./main Trace2.txt S 128 4
./main Trace2.txt S 128 16
./main Trace2.txt S 128 32
./main Trace2.txt S 128 64
./main Trace2.txt S 256 4
./main Trace2.txt S 256 16
./main Trace2.txt S 256 32
./main Trace2.txt S 256 64
./main Trace1.txt L 64 4
./main Trace1.txt L 64 16
./main Trace1.txt L 64 32
./main Trace1.txt L 64 64
./main Trace1.txt L 128 4
./main Trace1.txt L 128 16
./main Trace1.txt L 128 32
./main Trace1.txt L 128 64
./main Trace1.txt L 256 4
./main Trace1.txt L 256 16
./main Trace1.txt L 256 32
./main Trace1.txt L 256 64
./main Trace2.txt L 64 4
./main Trace2.txt L 64 16
./main Trace2.txt L 64 32
./main Trace2.txt L 64 64
./main Trace2.txt L 128 4
./main Trace2.txt L 128 16
./main Trace2.txt L 128 32
./main Trace2.txt L 128 64
./main Trace2.txt L 256 4
./main Trace2.txt L 256 16
./main Trace2.txt L 256 32
./main Trace2.txt L 256 64
./main Trace1.txt E 64 4
./main Trace1.txt E 64 16
./main Trace1.txt E 64 32
./main Trace1.txt E 64 64
./main Trace1.txt E 128 4
./main Trace1.txt E 128 16
./main Trace1.txt E 128 32
./main Trace1.txt E 128 64
./main Trace1.txt E 256 4
./main Trace1.txt E 256 16
./main Trace1.txt E 256 32
./main Trace1.txt E 256 64
./main Trace2.txt E 64 4
./main Trace2.txt E 64 16
./main Trace2.txt E 64 32
./main Trace2.txt E 64 64
./main Trace2.txt E 128 4
./main Trace2.txt E 128 16
./main Trace2.txt E 128 32
./main Trace2.txt E 128 64
./main Trace2.txt E 256 4
./main Trace2.txt E 256 16
./main Trace2.txt E 256 32
./main Trace2.txt E 256 64
EOF