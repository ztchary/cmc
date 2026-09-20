#!/bin/bash
cat "$1" | sed -n 's/\s*[a-z]\+,/,/g; s/\s*[a-z]\+)/)/; s/ {/;/;/^\w/p'
