#!/bin/bash
for f in *.png ; do convert "$f" -resize 200x150^ -gravity center -extent 200x150 "$f" ; done
