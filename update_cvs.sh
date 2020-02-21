#!/bin/bash
git pull --ff-only && \
	(cd src_cvs && cvs update) && \
	make clean && \
	cp -a src/* src_cvs/ && \
	(
		cd src_cvs && \
		cvs update && \
		(
			if cvs update |& grep -w '?'; then
				echo "You have new files to add to CVS first:"
				echo "if you don't, put them in .cvsignore"
				echo
				cvs update |& grep -w '?' | cut -d' ' -f2 | xargs -n1 echo cvs add
				false
			fi
		) && \
		echo "you sure you want to do this?" && \
		echo && \
		echo Run && \
		echo "(cd src_cvs && cvs commit -m "'"'"$(git rev-parse HEAD)"'")' &&\
		echo When you are
	)
