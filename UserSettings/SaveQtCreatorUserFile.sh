#!/bin/sh
USER_DIR="UserSettings/`hostname`/`whoami`/QtTetris.creator.user/"
mkdir -p $USER_DIR
cp ./QtTetris.creator.user $USER_DIR
