#!/bin/bash 
source import.sh
name=(${1//./ })

asm_test $name
