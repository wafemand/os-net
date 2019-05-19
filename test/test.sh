#!/usr/bin/env bash

function check_diff() {
    local DIFF=$(diff $1 $2)
    if [[ $? -eq 0 ]] && [[ ${DIFF} == "" ]]
    then
        echo "OK"
    else
        echo "FAILED"
    fi
}

CLIENT=$(realpath $1)
SERVER=$(realpath $2)

rm -rf client
rm -rf server
mkdir client
mkdir server

cp server_file server/file
cp client_file client/file
touch server/empty
touch client/empty

cd server
trap "kill 0" EXIT
${SERVER} 1337 &
cd ..

cd client

echo ""
echo "--------- download test ----------"
${CLIENT} localhost 1337 download file fileFromServer
check_diff ../server/file fileFromServer

echo ""
echo "--------- upload test ----------"
${CLIENT} localhost 1337 upload file fileFromClient
check_diff ../server/fileFromClient file

echo ""
echo "--------- download nonexistent file test ----------"
${CLIENT} localhost 1337 download nonexistent fileFromServer

echo ""
echo "--------- download empty test ----------"
${CLIENT} localhost 1337 download empty emptyFromServer
check_diff ../server/empty emptyFromServer

echo ""
echo "--------- upload empty test ----------"
${CLIENT} localhost 1337 upload empty emptyFromClient
check_diff ../server/emptyFromClient empty

echo ""
echo "--------- upload and download test ----------"
${CLIENT} localhost 1337 upload file file1
${CLIENT} localhost 1337 download file1 file2
check_diff file file2

