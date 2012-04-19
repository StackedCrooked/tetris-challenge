if [ ! -d 3rdParty/stm ]; then
    cd 3rdParty && ./get-dikustm.sh
else
    echo "stm is already checked out"
fi

./UserSettings/LoadQtCreatorUserFile.sh
