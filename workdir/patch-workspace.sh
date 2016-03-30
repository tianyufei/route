#!/bin/bash
SRCDIR=$1
DESTDIR=$2
TOOLDIR=$3

echo "Patching $DESTDIR from $SRCDIR....."

if [ ! -e $SRCDIR ] ; then
   echo "ERROR: $SRCDIR does not exist"
   exit 1
fi
if [ ! -e $DESTDIR ] ; then
   echo "ERROR: $DESTDIR does not exist"
   exit 1
fi

# rm -rf $DESTDIR/package/iproute2
# creeak link
echo "====================create link from tmp to etc begin================================="
#touch /tmp/fw_env.config
#touch /tmp/root
#mkdir -p $SRCDIR/package/base-files/files/etc/crontabs
#rm -f $SRCDIR/package/base-files/files/etc/crontabs/root
#rm -rf $SRCDIR/package/base-files/files/etc/fw_env.config
#ln -sf /tmp/fw_env.config $SRCDIR/package/base-files/files/etc/fw_env.config
#ln -sf /tmp/root $SRCDIR/package/base-files/files/etc/crontabs/root
#echo "====================create link from tmp to etc end================================="
#tar -C $SRCDIR -cf - . | tar -xf - -C $DESTDIR
#cp -rf $SRCDIR/* $DESTDIR

cur_path=$PWD
cd $SRCDIR
sfiles=`find . -name "*"`
cd $cur_path

for fl in $sfiles
do
	sfl=${SRCDIR}${fl:1}
	dfl=${DESTDIR}${fl:1}
	if [ -d "$sfl" ];then
		mkdir -p $dfl
	fi
	if [ -f "$sfl" ];then
		$TOOLDIR/mfile $sfl $dfl
	fi
done


echo "Patching Openwrt finished"


