#!/bin/sh

help() 
{
	echo "---------------------------------------------"
	echo "$0 [--help | [devicepath] [udev-action] "
	echo "Simulates an UDEV event by using the udevadm CLI"
	printf "\t --help : Shows this help dialog\n"
    printf "\t [devicepath] : Device-File (has to exist in /dev)\n"
    printf "\t [udev-action] : Action to execute (add, remove, change)\n"
	echo "---------------------------------------------"
}

if [ $# -lt 2 ]; then
	echo "Please provide all necessary parameters!"
	help
	exit 1 
fi

if [ $# -gt 2 ]; then
	echo "Too many arguments?"
	help
	exit 1 
fi

if [ "$1" = "--help" ]; then
	help
	exit 0
fi

if [ $2 != "add" ] && [ $2 != "remove" ] && [ $2 != "change" ]; then
    echo "Only [add|remove|change] are allowed as action (your input: '$2')!"
	help
	exit 1
fi

DEVPATH=$1
ACTION=$2
SYSPATH=$(udevadm info --query=all --name=$1 | grep -Eio 'DEVPATH=.*' | cut -d'=' -f 2)
udevadm trigger --action=$2 --property-match=DEVPATH=$SYSPATH
