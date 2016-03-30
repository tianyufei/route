#!/bin/sh
TOPDIR=`pwd`

PACKAGE=$1
DESTDIR=$2
DLFORDER=$3
echo "Creating openWRT $DESTDIR from $PACKAGE....."

if [ ! -e $PACKAGE ] ; then
   echo "ERROR: $PACKAGE does not exist"
   exit 1
fi
if [ -e $DESTDIR ] ; then
   echo "INFO: $DESTDIR exist, it will be used as currrent workspace"
   exit 0
fi

#create workspace directory
if [ ! -e $DESTDIR ] ; then
   mkdir -p $DESTDIR
   tar -zxvf $PACKAGE -C $DESTDIR 
   ln -s  $DLFORDER $DESTDIR/dl 
fi

cd $DESTDIR
#remove the *.o files, otherwise compile error in diff platform
ls scripts/config/*.o >.scripts.config.o.tmp
while read i
do
    echo 'removed...' $i 
    rm -rf $i
done<.scripts.config.o.tmp
rm .scripts.config.o.tmp

#remove .svn files, we won't use svn
find ./ -name ".svn" > .remove-svn.tmp
while read i
do
    echo 'removed...' $i 
    rm -rf $i
done<.remove-svn.tmp
rm .remove-svn.tmp

cd $CURDIR

