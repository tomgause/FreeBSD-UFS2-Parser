#!/bin/sh
gpart create -s GPT ada1
gpart add -t freebsd-ufs -a 1M ada1

newfs -U /dev/ada1p1
mkdir /newdisk
mount /newdisk

gpart show ada1

mkdir /newdisk/dir1
mkdir /newdisk/dir1/dir1.1
touch /newdisk/dir1/dir1.1/file2
echo "bar" >> /newdisk/dir1/dir1.1/file2
mkdir /newdisk/dir1/dir1.2
mkdir /newdisk/dir1/dir1.3

mkdir /newdisk/dir2
mkdir /newdisk/dir2/dir2.1
mkdir /newdisk/dir2/dir2.2
mkdir /newdisk/dir2/dir2.3

mkdir /newdisk/dir3
mkdir /newdisk/dir3/dir3.1
mkdir /newdisk/dir3/dir3.2
mkdir /newdisk/dir3/dir3.3

touch file1
echo "foo" >> file1
touch dead
echo "squirrel-pancake" >> dead
touch beef
echo "medium-rare" >> beef
touch big-file
for i in `seq 1 1000`; do echo "fuck Microsoft " >> big-file; done