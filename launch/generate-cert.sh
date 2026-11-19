#!/usr/bin/env bash

i=$1

openssl req -newkey rsa -nodes -x509 -out Player-Data/P$i.pem -keyout Player-Data/P$i.key -subj "/CN=P$i" -addext "subjectAltName=DNS:P$i"

echo "Copy Player-Data/P$i.pem to Player-Data/ on all parties and then run 'openssl rehash Player-Data/' (or c_rehash) there"
