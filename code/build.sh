pushd ../build/

INCLUDE="-I../code/include/raylib -I../code/include/libchardet"
LIBS="-L. libraylib.a libchardet.a"
SHARED_OBJECTS="./libraylib.so.450 ./libchardet.so.1.0.0"

# Use -O3 for heavy optimization

g++ -fstack-protector-all -fno-gnu-unique -rdynamic -shared -fPIC -o demian.so $INCLUDE  ../code/demian.cpp -g
g++ -fstack-protector-all -fno-gnu-unique -o demian.x86_64 $INCLUDE $LIBS $SHARED_OBJECTS ../code/platform_layer_linux.cpp

popd
