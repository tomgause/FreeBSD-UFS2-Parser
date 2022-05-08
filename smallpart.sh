#!/bin/sh
gpart create -s GPT ada2
gpart add -t freebsd-ufs -a 1M ada1

newfs -U /dev/ada2p1
mkdir /newdisk
sudo mount /dev/ada2p1 /newdisk

gpart show ada2

mkdir /newdisk/dir1

touch /newdisk/file1
echo "foo" >> /newdisk/file1