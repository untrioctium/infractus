#!/bin/bash
function build
{
	make -C ./plugins/$1
}

function clean
{
	rm ./plugins/$1/*.o
	rm ./plugins/$1/plugin.so
}

if [ "$1" = "all" ]; then
	list=(`ls ./plugins`)
	for plugin in "${list[@]}"; do
		build $plugin
	done
elif [ "$1" = "list" ]; then
	list=(`ls ./plugins`)
	for plugin in "${list[@]}"; do
		echo $plugin
	done
elif [ "$1" = "cleanall" ]; then
	list=(`ls ./plugins`)
	for plugin in "${list[@]}"; do
		clean $plugin
	done
elif [ "$1" = "clean" ]; then
	clean $2
else
	build $1
fi


