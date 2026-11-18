#!/bin/sh

echo '# Getting Started' > readme.md
sed -e '1 d' -e 's#(Programs/Source#(../Programs/Source#g' -e 's#(./Dockerfile#(../Dockerfile#' ../README.md >> readme.md

echo '# Client Interface' > client-interface.md
cat ../clients/README.md >> client-interface.md
