Parsnip Theory
Copyright (c) 2014, fanzyflani. All rights reserved.
See LICENCE.txt for licensing details.

This program is now OPEN SOURCE. Share it however you like... although you should probably include the LICENCE.txt file.

If you want to build this, you may want to fight with the Makefile first.
Also, read autobuild.sh for how I actually build this - I use different systems to do the Linux builds, though.

Here's my general routine:
cd sackit && gmake -j8 && cd .. && gmake -j8 && ./parsniptheory

----

Windows users: Run "parsniptheory-windows-i386.exe".

Linux users: Run "./parsniptheory-linux.sh". Alternatively, if you just so happen to have all the right libs installed, you can try running "./parsniptheory-linux-i386".
LET ME KNOW IF THIS IS MISSING ANY LIBRARIES! (They're from a version of Debian.)

Raspberry Pi users: Run "./parsniptheory-linux.sh". You will need the correct libraries to run this (SDL, SDL_net, and some others you ALREADY HAVE). This was built on Raspbian. Prod me if you need help or are missing libraries.

FreeBSD users: You can work this one out. Alternatively, you can try emulating the Windows or Linux i386 versions.

Mac OS X users: I have no plans for a native port to this platform. If you want there to be a port, I'll need you to test, and also help me set up a cross-compiler. Otherwise, use Wine.


----

Map list:
- genesis - The first map ever. Actually a cafeteria!
- layer8 - Simple, boring map to test 8 players.
- airport - Much more interesting map to test 8 players.

----

Network info:
TCP port 2014. New protocol since Alpha 9 - older clients will not be able to connect.

----

Playing instructions:
* For "L select, R act":
  * Left click either selects or deselects a unit.
  * Right click either moves your unit, stands/crouches your unit, or attempts to throw a tomato at an enemy player.
* For "L for all":
  * Left click either selects a unit, moves your unit, stands/crouches your unit, or attempts to throw a tomato at an enemy player.
  * Right click deselects.
* Shift + click on a tile attempts to attack that tile.
* Enter ends your turn and moves onto the next player.
* Escape "pauses" the game, mostly so you can move your mouse elsewhere.
* Your units get 7 "steps" each.
  * Moving by 1 tile  uses up 1 step when standing or 2 when crouching.
  * Throwing a tomato uses up 2 steps.
  * Switching between crouching and standing uses up 1 step.
* When crouched directly behind a table, shots coming from that direction will miss you.
* Getting hit by a tomato takes 10 points off that unit's tolerance meter.
* Once the tolerance meter hits 0, that unit is out!
* When a player has no more units, they are out of the game!
* Last remaining player wins!

Editor reference:
* Left-mouse: Set tile on level
* Middle-mouse-drag: Scroll view
* Right-mouse: Get tile from level
* 1-8: Place player 1-8 spawn
* G: Hold to show grid
* T: Select a tile (press ESC to bail out of this)
* Ctrl-N (Nuke): Clear ALL spawns! (Doesn't clear the tiles)
* Ctrl-L: Load level
* Ctrl-S: Save level

