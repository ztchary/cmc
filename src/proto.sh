#!/bin/bash
cat proto.c | sed -n 's/\s*\S\+,/,/g;s/\s*\S\+)/)/;s/ {/;/;/^\w/p'
