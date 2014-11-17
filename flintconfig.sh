#!/bin/bash

rm -rf ./project/flint_xbmc/assets/*
rm -rf ./project/flint_xbmc/res/*
rm -rf ./project/flint_xbmc/libs/*

cp -Rvf ./tools/android/packaging/xbmc/assets/* ./project/flint_xbmc/assets/
cp -Rvf ./tools/android/packaging/xbmc/res/* ./project/flint_xbmc/res/
cp -Rvf ./tools/android/packaging/xbmc/lib/* ./project/flint_xbmc/libs/
echo "copy success!"
