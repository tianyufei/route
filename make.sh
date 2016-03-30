#!/bin/sh

# 
# Copyright (c) 2014 YAYA Software Team.
# All Rights Reserved.
# example-ap121.mk
# Draft:  zhengdajun 
#

# example ./make.sh {env|target|host|pc|sdk|yaya} {products/example.mk} [clear|install|uninstall] 

action=${1:-help}
product=${2:-null}
option=${3:-null}

#create enviroment
export YB_TOPDIR=`pwd`
YB_SCRIPT=$YB_TOPDIR/$product
chmod 777 $YB_SCRIPT
. $YB_SCRIPT

export YB_ENV_HEAD_FILE=$YB_TOPDIR/src/inc/config.h
export YB_BUILD_OUT_PATH=$YB_TOPDIR/out/$action
export YB_BUILD_TIME=`date "+%Y%m%d%H%M%S"`
export YB_MAKE_COMMAND="SCRIPT_FILE=$YB_SCRIPT"

echo "-------- PRODUCT NAME       = $Y_PRODUCT_NAME" 
echo "-------- HARDWARE_VENDOR    = $Y_HARDWARE_VENDOR"			 
echo "-------- HARDWARE_CLASS     = $Y_HARDWARE_CLASS"
echo "-------- HARDWARE_VERSION   = $Y_HARDWARE_VERSION"
echo "-------- SOFTWARE_VERSION   = $Y_SOFTWARE_VERSION"
echo "-------- PLATFORM_TYPE      = $Y_PLATFORM_TYPE"
echo "-------- PLATFORM_VERSION   = $Y_PLATFORM_VERSION"
echo "-------- BUILD_TIME         = $YB_BUILD_TIME"

tools() {
    mkdir -p ./tools/bin
    if [ ! -f "./tools/bin/def2cfg" ]
    then
   	g++ -o ./tools/bin/def2cfg ./tools/def2cfg.cpp
    fi
    if [ ! -f "./tools/bin/mfile" ]
    then
	g++ -o ./tools/bin/mfile ./tools/mfile.cpp
    fi

    product_file=${product#*/}
    script_compare=`diff $YB_SCRIPT /tmp/yaya-script/$action-$product_file 2>&1`
    if [ -n "$script_compare" ]; then	
	    if [ -d "$YB_BUILD_OUT_PATH" ]; then	
	    	rm -rf $YB_BUILD_OUT_PATH
	    fi
	    touch $YB_ENV_HEAD_FILE
           mkdir -p $YB_BUILD_OUT_PATH
	    echo "#ifndef _PRODUCTS_DEFINE" > $YB_ENV_HEAD_FILE
	    echo "#define _PRODUCTS_DEFINE" >> $YB_ENV_HEAD_FILE
	    ./tools/bin/def2cfg $YB_SCRIPT >> $YB_ENV_HEAD_FILE
	    echo "#define TARGET_TYPE_$action 1" >> $YB_ENV_HEAD_FILE
	    echo "#endif" >> $YB_ENV_HEAD_FILE
	    mkdir -p /tmp/yaya-script
	    rm /tmp/yaya-script/*
            cp $YB_SCRIPT /tmp/yaya-script/$action-$product_file -rf
    fi
}

tag(){
	echo "product_name: "$Y_PRODUCT_NAME > $YB_BUILD_OUT_PATH/version_file
	echo "hardware_vendor: "$Y_HARDWARE_VENDOR >> $YB_BUILD_OUT_PATH/version_file
	echo "hardware_class: "$Y_HARDWARE_CLASS >> $YB_BUILD_OUT_PATH/version_file
	echo "hardware_version: "$Y_HARDWARE_VERSION >> $YB_BUILD_OUT_PATH/version_file
	echo "software_version: "$Y_SOFTWARE_VERSION >> $YB_BUILD_OUT_PATH/version_file 
	echo "platform_type: "$Y_PLATFORM_TYPE >> $YB_BUILD_OUT_PATH/version_file 
	echo "platform_version: "$Y_PLATFORM_VERSION >> $YB_BUILD_OUT_PATH/version_file 
	echo "build_time: "$YB_BUILD_TIME >> $YB_BUILD_OUT_PATH/version_file 
}

env(){
    chmod 777 ./tools/envsetup.sh
    ./tools/envsetup.sh
    make ROOT_PATH=$YB_TOPDIR OUT_PATH=$YB_BUILD_OUT_PATH -C external
}

host() {
   tools
   if [ "$option" = "clean" ]; then
        echo "Clean host......."
    	if [ -d "$YB_BUILD_OUT_PATH" ]; then	
		rm -rf $YB_BUILD_OUT_PATH
	fi
   else
   	echo "Compile host......."
   	make $YB_MAKE_COMMAND BUILD_TARGET=$action ROOT_PATH=$YB_TOPDIR OUT_PATH=$YB_BUILD_OUT_PATH -C src
   fi
   cd $YB_TOPDIR
   tar -C $YB_TOPDIR/tools/$action -cf - . | tar -xf - -C $YB_TOPDIR/out/$action
   tag	
}

target() {
    tools
    if [ "$option" = "clean" ]; then
        echo "Clean target......."
    	if [ -d "$YB_BUILD_OUT_PATH" ]; then	
		rm -rf $YB_BUILD_OUT_PATH
	fi
    else
    	echo "Compile target......."
    	make $YB_MAKE_COMMAND BUILD_TARGET=$action ROOT_PATH=$YB_TOPDIR OUT_PATH=$YB_BUILD_OUT_PATH -C workdir
	cd $YB_TOPDIR/workdir/build/$Y_PRODUCT_NAME
	make V=99 -j 4
	cd $YB_TOPDIR
        tag
    fi
}

sdk() {
    tools
    if [ "$option" = "clean" ]; then
        echo "Clean sdk......."
    	if [ -d "$YB_BUILD_OUT_PATH" ]; then	
		rm -rf $YB_BUILD_OUT_PATH
	fi
    else
   	echo "Compile sdk......."
    	make $YB_MAKE_COMMAND BUILD_TARGET=$sdk ROOT_PATH=$YB_TOPDIR OUT_PATH=$YB_BUILD_OUT_PATH -C sdk
        tag
    fi
}

#help() {
#       cat <<EOF
#Usage: ./make.sh {board type} {product} [options] 
#
#Board type:
#        target        Compile the hardware image
#        host          Compile the emulator image
#	pc	      Compile pc image
#	sdk	      Compile from sdk
#Product:
#        script/xxx.mk Customer Products File
#Options:
#        clean         Clean 
#        install       Install 
#        uninstall     Uninstall #
#	EOF
#}

if [ "$option" != "clean" ] && [ "$option" != "install" ] && [ "$option" != "uninstall" ] && [ "$option" != "null" ]; then
    action=help
fi
if [ "$action" != "host" ] && [ "$action" != "target" ] && [ "$action" != "pc" ] && [ "$action" != "env" ]&& [ "$action" != "sdk" ]; then
    action=help
fi

$action 

