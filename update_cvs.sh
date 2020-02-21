#!/bin/bash

removed_files() {
	# does not detect CVS files (CVS and .settings folders)
	# excludes some files from the initial code (.project, .settings, .cproject)
	diff -r src src_cvs | grep 'Only in src_cvs' \
		| sed 's/Only in src_cvs\(.*\): /src_cvs\1\//' \
		| grep -v -E '/CVS$' \
		| grep -v -w 'src_cvs/.gitignore' \
		| grep -v -w 'src_cvs/.project' \
		| grep -v -w 'src_cvs/.settings' \
		| grep -v -w 'src_cvs/.cproject'
}

# wrapper to update cvs quietly from the (git) repo root
cvs_update() {
	# run in subshell to keep cwd intact
	(cd src_cvs && cvs -q update)
}

set -e

# ensure working dir is clean and repos up-to-date
echo Syncing git
git pull --ff-only
echo
echo Syncing CVS
cvs_update
echo
make clean >/dev/null

echo
echo Copying files
echo

# copy stuff (.cvsignore would be ignored by *)
cp -a src/.cvsignore src_cvs/
cp -a src/* src_cvs/

# update again to show changes
echo Changes to commit:
cvs_update
echo

# detect files that are not in git but are in CVS
if removed_files &>/dev/null; then
	echo "You have removed files in your cvs tree"
	echo "remove them with:"
	echo
	removed_files | xargs -n1 echo rm

	exit 1
fi

# detect files not added to CVS
if cvs_update |& grep -w '?' &>/dev/null; then
	echo "You have new files to add to CVS first:"
	echo "if you don't, put them in .cvsignore"
	echo
	cvs_update |& grep -w '?' | cut -d' ' -f2 | xargs -n1 echo cvs add
	exit 1
fi

# detect up to date CVS
if [ "$(cvs_update |& wc -l)" = "0" ]; then
	echo "CVS is already up to date it seems"
	false
fi

echo "you sure you want to do this?"
echo
echo Run
echo '(cd src_cvs && cvs commit -m "'"$(whoami)@$(hostname) - $(git rev-parse HEAD) $(date --iso-8601=minutes)"'")'
