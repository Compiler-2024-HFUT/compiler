#!/bin/bash 
source import.sh
name=(${1//./ })

compile_pass $name
pass_test $name