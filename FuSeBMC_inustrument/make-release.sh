#!/bin/sh
if [ ! -d "release" ]; then
	mkdir release
fi
rm -rf ./release/*

echo ""
#echo "Building FuSeBMC_inustrment release ... "
echo ""

#make clean release
echo ""
if [ -f "./FuSeBMC_instrument" ]; then
	echo "Found: ./FuSeBMC_instrument"
	cp ./FuSeBMC_instrument ./release/
	cp ./myincludes.txt ./release/
	cp -r ./external_include ./release/
	echo "The executable FuSeBMC_instrument is located in './release' directory."
else
	echo "Cannot find FuSeBMC_instrument"
fi
echo ""
echo "DONE"
echo ""

