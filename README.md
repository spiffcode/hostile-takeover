## About

*Hostile Takeover* is the open source release of the wildly popular mobile Real Time Strategy game [*Warfare Incorporated*](http://www.warfareincorporated.com/). Warfare Incorporated's developers, grateful for all the contributions of the open source community, are delighted to give something back.

#### Legal Stuff
*Hostile Takeover* and *Warfare Incorporated* are registered trademarks of *Spiffcode, Inc* the developers of Warfare Incorporated. *Hostile Takeover* is copyrighted 2004-2014 by Spiffcode, Inc. The Hostile Takeover source code is made available under the [BSD open source license](LICENSE.txt).

## Building

### Prerequisites
**macOS and iOS Ports:**
- Mac OS X
- [Xcode 5 (or later)](https://itunes.apple.com/us/app/xcode/id497799835?mt=12) with *Command Line Tools* installed
	- Install *Command Line Tools* from Xcode *Preferences... Downloads*.

**Android Port:**
- [Eclipse](https://eclipse.org)
- [Eclipse ADT Plugin](https://dl-ssl.google.com/android/eclipse/)
- [Android NDK](https://developer.android.com/ndk/)
- [Android SDK](https://developer.android.com/studio/) Command Line Tools

### Building the Hostile Takeover Executable

**iPhone Port:**
1. Within Xcode, open `game/wi.xcodeproj`.
- Select a Build Configuration via *Edit Scheme...Info*
	- **Debug (default)**. Has extra runtime checks and is typically used during development. Displays "DEV BUILD" and date on Hostile Takeover startup screen. The Debug version requires a local server to be running for multiplayer testing.
	- **Release**. W/o extra runtime checks. Displays date on Hostile Takeover startup screen.
	- **Distribution**. Same as Release but displays game database version number on Hostile Takeover startup screen.
	- **Distribution-Store**. Same as Distribution but is signed with a necessary key for redistribution in the iTunes App Store. NOTE: You'll have to get your own key if you want to distribute through the App Store.
	- **Adhoc**. ???
- Build and run!
	- On your device or in the iOS Simulator.

**SDL macOS and iOS Ports:**
- Within Xcode, open `game/sdl/htsdl.xcodeproj`.
- Select a Build Configuration via *Edit Scheme...Info*
	- **Debug (default)** for development, has extra runtime cheacks and displays "DEV BUILD" and date on the game startup screen.
	- **Release** W/o extra runtime checks and displays game database version number on the game startup screen.
- Select a Build Target:
	- **HT - Desktop** for building for macOS.
	- **HT  Mobile** for building for iOS device or iOS simulator.
- Build, and run on your target device!

**Android Port:**
- Set desired app ABI in `game/sdl/android/jni/Application.mk` (default ABI is: all).
- Add your NDK path to your environment variables `export PATH=$PATH:[YOUR NDK PATH]`.
- `cd game/sdl/android`.
- For debug build: `ndk-build`. For release build: `ndk-build REL=1`.
- Import the project into Eclipse if you have not already done so.
    - File > New > Other... > Android Project from Existing Code
    - Browse to the HT project at `game/sdl/android` and click finish.
- Build and run!
	- To run on connected device in Eclipse: Hostile Takeover > Run As > Android Application
	- To build an .apk in Eclipse: File > Export... > Export Android Application > Hostile Takeover > [Developer Keystore](https://developer.android.com/training/articles/keystore.html) > Finish

**Linux Port (WIP)**

- Run `game/sdl/linux/install.sh` to create the necessary directories and install the necessary packages.
- `cd game/sdl/linux/` and `make` or `make REL=1`.

### Building the Hostile Takeover Data File
The Hostile Takeover iOS executable incorporates a data file, **htdata832.pdb**, which includes all the art, audio, and levels for the game. A prebuilt version of this data file is already in the source tree so you don't need to build it unless you are modifying assets it contains.

Originally all of Hostile Takeover was developed on Windows using Windows tools like Visual Studio and C#. To build the data file on the Mac, the cross platform Mono framework is used.

1. Install the Mono runtime environment.
	- Available from http://www.go-mono.com/mono-downloads/download.html. Version 3.4 (Mono MRE installer) is known to work.
- `cd data`
- `make IPHONE=1 MULTIPLAYER=1 clean all`
	- some harmless warnings will be displayed
- Mark the data file with a version number.
	- `cd ../game`
    - `../bin/markversion.exe 1 htdata832.pdb "+++VERSION+++" 1.6`

### Building and running the Hostile Takeover Multiplayer Server
Hostile Takeover supports single player and multiplayer play. Multiplayer is supported by two server components: a TCP based game server located in the **server** directory, and an HTTP based server located in the **stats** directory used for accounts, leaderboard, game history and game stats.

1. The game server is a command line server which builds can be built and run on OSX or Linux. The game server is does not (currently) build and run on Windows. The production server environment is expected to be Linux based.
- `cd server`
- make REL=1 clean all (for a release build)
- make clean all (for a debug build)

Please refer to the README.txt in the **server** directory for more detailed information on how to run the server environment in development and production.

## Tools
The `bin` directory contains many command line tools used to build the htdata file. There are also GUI tools for editing maps (M) and animations (AniMax).

The GUI tools have primarily been used on Windows and are best described as 'flaky' on the Mac. If you want to give level editing a try on a Mac:

1. `mono bin/m.exe`
- File > Open
- Select a .ld (level description) file from the `data` directory.
- There is an m.chm file for M help. m.chm is a Windows specific file format. There is also an overview on the web found here: http://sites.warfareincorporated.com/mission-authoring/

See [Building the Hostile Takeover Data File](#building-the-hostile-takeover-data-file) for instructions on how to rebuild the data file after making changes to any level, image, sound files or other game assets.
