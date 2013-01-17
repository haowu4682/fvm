#!/bin/sh
# Usage: init.sh [work_dir]
rm -rf $1
mkdir $1
cd $1
git init
cp $2 .fvmaccesslist
git add .fvmaccesslist
git commit --allow-empty -am "init commit"

