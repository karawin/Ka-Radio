#!/bin/bash

echo style
python3 ./css-html-js-minify.py style.css 
mv style.css style.ori
mv style.min.css style.css
gzip  style.css 
mv style.css.gz style.css
xxd -i style.css > style
sed -i 's/\[\]/\[\] ICACHE_STORE_ATTR ICACHE_RODATA_ATTR /g' style
sed -i 's/unsigned/const /g' style
mv style.ori style.css

echo style1
python3 ./css-html-js-minify.py style1.css 
mv style1.css style1.ori
mv style1.min.css style1.css
gzip  style1.css 
mv style1.css.gz style1.css
xxd -i style1.css > style1
sed -i 's/\[\]/\[\] ICACHE_STORE_ATTR ICACHE_RODATA_ATTR /g' style1
sed -i 's/unsigned/const /g' style1
mv style1.ori style1.css

echo script
python3 ./css-html-js-minify.py script.js 
mv script.js script.ori
mv script.min.js script.js
gzip  script.js 
mv script.js.gz script.js
xxd -i script.js > script
sed -i 's/\[\]/\[\] ICACHE_STORE_ATTR ICACHE_RODATA_ATTR  /g' script
sed -i 's/unsigned/const /g' script
mv script.ori script.js

echo index
cp index.html index.htm
python3 ./css-html-js-minify.py index.htm
gzip index.html
mv index.html.gz index.html
xxd -i index.html > index
sed -i 's/\[\]/\[\] ICACHE_STORE_ATTR ICACHE_RODATA_ATTR  /g' index
sed -i 's/unsigned/const /g' index
mv index.htm index.html

echo logo
cp logo.png logo.ori
gzip logo.png
mv logo.png.gz logo.png
xxd -i logo.png > logo
sed -i 's/\[\]/\[\] ICACHE_STORE_ATTR ICACHE_RODATA_ATTR  /g' logo
sed -i 's/unsigned/const /g' logo
mv logo.ori logo.png

echo favicon
cp favicon.png favicon.ori
gzip favicon.png
mv favicon.png.gz favicon.png
xxd -i favicon.png > favicon
sed -i 's/\[\]/\[\] ICACHE_STORE_ATTR ICACHE_RODATA_ATTR  /g' favicon
sed -i 's/unsigned/const /g' favicon
mv favicon.ori favicon.png
