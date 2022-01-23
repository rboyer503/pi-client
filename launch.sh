#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: ./launch.sh <server hostname/ip> <port>"
    exit 1
fi

# Generate token and securely write to server token file.
token_file="/tmp/pi-server-token"
token=$(openssl rand -base64 12)
echo -n "$token" | ssh pi@"$1" -p "$2" -T "cat > $token_file"

# Launch pi-client with same token.
./pi-client "$1" "$token"
