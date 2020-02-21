#!/bin/bash

removed_files() {
	diff -r src src_cvs | grep 'Only in src_cvs' \
		| sed 's/Only in src_cvs\(.*\): /src_cvs\1\//' \
		| grep -v -E '/CVS$' \
		| grep -v -w 'src_cvs/.gitignore' \
		| grep -v -w 'src_cvs/.project' \
		| grep -v -w 'src_cvs/.settings' \
		| grep -v -w 'src_cvs/.cproject'
}

cvs_update() {
	# run in subshell to keep cwd intact
	(cd src_cvs && cvs update)
}

git pull --ff-only && \
	cvs_update && \
	make clean >/dev/null && \
	cp -a src/.cvsignore src_cvs/ && \
	cp -a src/* src_cvs/ && \
	cvs_update && \
	(
		echo checking for extra files
		if removed_files &>/dev/null; then
			echo "You have removed files in your cvs tree"
			echo "remove them with:"
			echo
			removed_files | xargs -n1 echo rm
			false
		fi
	) && \
	(
		if cvs_update |& grep -w '?' &>/dev/null; then
			echo "You have new files to add to CVS first:"
			echo "if you don't, put them in .cvsignore"
			echo
			cvs_update |& grep -w '?' | cut -d' ' -f2 | xargs -n1 echo cvs add
			false
		fi
	) && \
	echo "you sure you want to do this?" && \
	echo && \
	echo Run && \
	echo "(cd src_cvs && cvs commit -m "'"'"$(git rev-parse HEAD)"'")' &&\
	echo When you are
