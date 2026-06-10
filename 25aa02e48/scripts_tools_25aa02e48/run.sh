#!/bin/bash


if [[ -e "app" ]];then
	rm app
#g++ -o app main.cc
else
	echo "app no existe"
fi


g++ -o app main.cc



if [[ -e "app" ]];then
sudo ./app
fi
