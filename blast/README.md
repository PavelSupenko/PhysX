# Blast SDK Repo

Online documentation may be found here: [Blast SDK Documentation](https://nvidia-omniverse.github.io/PhysX/blast/index.html).

## Building the SDK

### Automatic

Run the `build.py` script and configure the build settings as you want.

Building for Android requires NDK path providing, in my Unity case it can be:
`/Applications/Unity/Hub/Editor/2022.3.28f1/PlaybackEngines/AndroidPlayer/NDK`

### Manual (Deprecated, new command args should be checked in the actual build.py script)

### Windows
1. run `build.bat`
2. built sdk location: `_build\windows-x86_64\release\blast-sdk` (release), `_build\windows-x86_64\debug\blast-sdk` (debug) 

### Linux
0. initialize (once): run `./setup.sh`
1. run `./build.sh`
2. built sdk location: `_build/linux-x86_64/release/blast-sdk` (release), `_build/linux-x86_64/debug/blast-sdk` (debug) 

### Building for Android

Building for Android requires NDK path providing, in my Unity case it can be:
`/Applications/Unity/Hub/Editor/2022.3.28f1/PlaybackEngines/AndroidPlayer/NDK`

So the whole command looks like:
```
./build.sh --config DEBUG --out ./output --ndk /Applications/Unity/Hub/Editor/2022.3.28f1/PlaybackEngines/AndroidPlayer/NDK
```
