Game history and notes
----------------------

Originally this game was started in 2001 and targeted Palm based and
PocketPC based handheld devices. The main development environment was
Windows. The game was developed and run on Windows, which allowed for
fast iteration, and ports were simultaneously written and maintained
for Palm and PocketPC.

Code in the "game" directory is cross-platform. Platform
specific code is in subdirectories. The platform code for the original
Palm and Windows CE based Pocket PC devices is currently not part of this
release. It was removed because these platforms are long gone, and
putting time and effort into maintaining them would not be a good use
of time. If there is interest in these platforms, we can release this code
(email Scott and Darrin). The first Palm and PocketPC releases were made in
2003.

Later in 2007 the iPhone hit the scene and the iOS version was started.
The iOS platform specific code evolved while coding on a Mac using OS X.
The Windows platform code wasn't being maintained during this period, so it
is a bit out of sync with the game side. Since the iOS device dev environment
and simulator is reasonably good, there was also no effort put into an OS X
specific platform layer (for running on Macs). The first iOS release was made
in 2008.

Fast forward to 2012/13. An SDL layer (Simple Direct Media Layer, see
libsdl.org) was started, but not (yet) completed. The idea with the SDL
version is to use it as the platform layer for potentially all platforms.
Rather than having N platform specific layers, have SDL address the bulk
platform requirements, and then have smaller platform specific code as
necessary. As of 6/2014 this hasn't been finished but this is the direction
to go in for the future. In other words, when an Android version is created,
the SDL layer should be completed first rather than creating an Android layer.
Once this is done, the iOS version can be moved to the SDL version as well (and,
Windows and Mac versions can be created easily).

New artwork
-----------

Currently the game uses 2D artwork that was created for 8 bit Palm devices in
both "low res" and "high res" formats.  A decision was made to ship the
iOS version with the original artwork, rather than waiting for and
incorporating all new artwork. Because the iPhone res was so much higher than
the old Palms, the high res Palm artwork was used and autoscaled up by 1.33x.
Around 2008-09, we commissioned much higher res 32 bit artwork to be created,
with 2-4x the animation frames. This was never incorporated into the game,
but it can be. We are releasing this new artwork with the game. The artwork
is again 2D and rendered at a fixed size. A few issues with this new artwork:

- Current high res devices have again made this new artwork small by
  comparison. The proper path forward has to be evaluated (3D perhaps?).
- The render code is almost all handwritten, expects 8 bit artwork, and
  is on the game side. Ideally, the platform specific side would handle
  drawing and the game side wouldn't have format dependencies.
- The unit animations are currently tied into multiplayer simulation state.
  Read below.

The game's multiplayer model works by synchronized game state. In other words,
it assumes that if all clients get the same commands at the same time, they
will progress to the exact same state at time X. This makes maintaining game
state easy - just communicate the commands, not the game state. An issue that
currently exists is that the game simulation, and the unit animations are
joined at the hip, so to speak. If you have two clients, one
that has a Liberator with 16 frames of animation, the other with 24 frames
of animation, these clients won't be in sync with each other. Either this
design needs to change so animation isn't synchronized with game state this
way, or all clients need to animate the exact same way.

Platform specific code directories
----------------------------------

palm (removed)
ce (removed)
iphone (working. should of been named "ios")
sdl (not too far from working. Expected to be future king).
win (not working. Left for reference.)

There are more details about the sdl port to discuss in a more direct way
through email. Please contact Scott and Darrin.

Why are we releasing this
-------------------------

We have enjoyed creating and supporting the game community of users over the
years. We want to see the game continue to grow and flourish beyond what we
are capable of at this time. We believe releasing it with an open license
will make it an opportunity for others to build on and extend going forward.
For example we hope to see an Android port come out of this, and we're looking
forward to see what other innovations happen.

We ask that developers contribute changes back for others to use and benefit
from. This game has lot of potential to grow and get better. RTS games continue
to evolve with innovating features and game play, and mobile devices these days
are very capable. At a minimum, there is a huge amount of opportunity to grow
this game in these directions to take advantage of the current state of the
art.

Scott Ludwig (scottlu@gmail.com)
Darrin Massena (darrin@massena.com)
