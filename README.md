Race track model used:
"Cartoon Race Track Spielberg" by RCC Design, used under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/),
source: [Sketchfab](https://sketchfab.com/3d-models/cartoon-race-track-spielberg-23dbb21af64e407286fd16e29c9aea25)

Vehicle model used:
"Nissan Skyline R34 GT-R" by Lexyc16, used under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/),
source: [Sketchfab](https://sketchfab.com/3d-models/nissan-skyline-r34-gt-r-ff8fb2251dfa4bb9979e7022c5a6666c)

Building the game:

```
mkdir build && cd build
cmake .. 
cmake --build .
```

Building the server (with checks):

```
cd netcode/server
mkdir build && cd build
cmake -DCMAKE_CXX_CLANG_TIDY="clang-tidy;-checks=-*" ..
cmake --build .
```

Running valgrind on the server (assuming it has been built):

```
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./server <port>
```