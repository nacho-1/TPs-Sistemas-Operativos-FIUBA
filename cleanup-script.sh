#!/bin/bash

for branch in $(git branch --format="%(refname:short)" | grep '^src-base_'); do
	git checkout "$branch"
	rm -rf .github
	rm .pre-commit*
	git add .
	git commit -m "Cleanup github workflows"
	git checkout src-main
	git merge --no-ff "$branch" -m "Merge $branch into src-main"
done

