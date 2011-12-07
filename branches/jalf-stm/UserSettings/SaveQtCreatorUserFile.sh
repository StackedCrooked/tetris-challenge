#!/bin/sh
USER_DIR="UserSettings/`hostname`/`whoami`/"
mkdir -p $USER_DIR
cp ./QtTetris.creator.user $USER_DIR
