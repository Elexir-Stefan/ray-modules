#!/bin/sh
NAMEFROM=module.name

if [ "$1" == "--help" ]
then
    echo -e "USAGE: make.sh [GCC_ARGS]"
    echo -e "Compiles a module in the current dir (name from file $NAMEFROM)"
    echo -e "\tGCC_ARGS\tAdditional arguments to gcc"
else

	if [ -r $NAMEFROM ]
	then
		MODFILE=`cat $NAMEFROM`

		if [ -r $MODFILE.c ]
		then
			echo -e "Compiling $MODFILE.c ..."
			C_ARGS="-O0 -I ../../kernel/include -c -nostartfiles -ffreestanding -fno-common -Wall -Werror -fno-ident -fno-stack-protector $@"
			LD_ARGS="-r"
		
			if [ -f $MODFILE.o ]
			then
				rm $MODFILE.o
			fi
			
			gcc $C_ARGS $MODFILE.c -o $MODFILE.o
			
			if [ -f $MODFILE.o ]
			then
			ld $MODFILE.o $LD_ARGS -o $MODFILE
			echo -e "Done."
			else
			echo -e "!! NOT COMPILED DUE TO WARNINGS !!!"
			fi
		else
			echo -e "File '$MODFILE' not found! (Or at least not readable)"
		fi
	else
		echo -e "Module name definition file not found!"
		echo -e "Name definition expected to be in '$NAMEFROM'. Exiting."
	fi

fi
