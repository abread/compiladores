#!/bin/sh

# default
# assumes script inside test folder and `og` in ../
export testfolder="$(dirname $0)"

export lightred="\033[31m"
export lightgreen="\033[32;m"
export red="\033[31;1m"
export green="\033[32;1m"
export reset="\033[0m"

! [ -d results ] && mkdir results

testone() {
	set -e

	input=$1
	name=$(basename $input | cut -f 1 -d'.')
	exe=${input%.og}
	expected=$testfolder/expected/$name.out
	out=${input%.og}.out

	if ! [ -r $input ]; then
		printf '%b' "$0: cannot read file "$input"${red}aborting$reset\n"
		exit 1
	fi
	if ! [ -r $expected ]; then
		printf '%b' "$0: cannot read file $expected${red}aborting$reset\n"
		exit 1
	fi

	if make -j1 $out > /dev/null && diff -b $expected $out &> /dev/null; then
		printf '%b' "Test $name -- ${green}OK$reset\n"
	else
		printf '%b' "Test $name -- ${red}FAILED${reset}\n"
		diff --color -b $expected $out
		printf '%b' "$reset"
		exit 1
	fi
}
export -f testone

nice parallel --progress --eta --bar --joblog results/log $@ testone ::: $testfolder/*.og

n_tests=$(cat results/log | tail -n +2 | wc -l)
succs=$(cat results/log | tail -n +2 | cut -f7 | grep '0' | wc -l)
if [ $succs -eq $n_tests ]; then
	printf '%b' "Score$green $succs/$n_tests\n"
else
	printf '%b' "Score$red $succs/$n_tests\n"
fi

printf '%b' "$reset"
echo "$0: finished"
