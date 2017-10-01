# aomCheck

check how aom av1 works.

# author

taktod
poepoemix@hotmail.com
https://twitter.com/taktod/

I run this program on MacOS Sierra

# setup

## install with brew.

install ffmpeg
install opencv

## install aom
git clone https://aomedia.googlesource.com/aom
cd aom/
mkdir compile
cd compile
cmake .. -DCONFIG_ANS=1
make
cd ../../

## install ttLibC
git clone git@github.com:taktod/ttLibC
cd ttLibC/
autoreconf
./configure --enable-all --enable-gpl
make
cd ..

## rewrite code.
update opencvTest.c and encode.c fopen parameter to read your mp4 file. (h264 / aac).
compile.
run opencvTest to check.

run encodeTest to encode from h264 to av1.
run decodeTest to decode av1 and show image with opencv.

## compile this program.
mkdir compile
cd compile/
cmake ..
make
