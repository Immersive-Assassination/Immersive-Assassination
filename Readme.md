# ImmersiveAssassination

- dont use this its not tested at all (not even tried running it in game, may just crash)
- if you use this its entirely at your own risk
- compile only tested on my linux laptop so no idea if the msvc build works


## build
`git submodule update --init --recursive`

in External/ZHMModSDK:

`cmake --preset x64-Release-Mingw .`
`cmake --build --target install _build/x64-Release-Mingw`

in main folder:

`cmake --preset x64-Release-Mingw .`

`cmake --build _build/x64-Release-Mingw`

