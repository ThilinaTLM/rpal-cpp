
DEBUG_DIR_NAME="debug"
EXECUTABLE_NAME="rpal"

if [ "$1" = "build" ]; then
    [ ! -d "./$DEBUG_DIR_NAME" ] && mkdir "$DEBUG_DIR_NAME"
    cd "$DEBUG_DIR_NAME"
    cmake -H. -GNinja ..
    ninja
    cp compile_commands.json ../
    cd ..
elif [ "$1" = "run" ]; then
    ./$DEBUG_DIR_NAME/$EXECUTABLE_NAME $2 $3 $4 $5 $6 $7 $8
fi;



