#!/bin/bash
find ./ -name \*\~ | xargs /bin/rm
git ls-files --deleted -z | xargs -0 git rm
