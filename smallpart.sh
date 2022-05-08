#!/bin/sh
gpart create -s GPT ada1
gpart add -t freebsd-ufs -a 1M ada1

newfs -U /dev/ada1p1
mkdir /newdisk
sudo mount /dev/ada1p1 /newdisk

gpart show ada1

mkdir /newdisk/dir1

touch /newdisk/file1
echo "foo" >> /newdisk/file1