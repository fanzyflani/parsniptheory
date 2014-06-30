Parsnip Theory
Copyright (c) 2014, fanzyflani. All rights reserved.
See LICENCE.txt for licensing details.

This program is SHAREWARE. Unfortunately, there is no "registered version" yet, as this is currently in ALPHA TESTING. That means this ISN'T FINISHED.

Please, if you're sharing this program, make sure you share the unmodified files. (You can share modified files, just make sure you have unmodified files handy)

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
TCP port 2014. New protocol since Alpha 7 - older clients will not be able to connect.

----

Playing instructions:
* For "L for all":
  * Left click either selects a unit, moves your unit, or attempts to throw a tomato at an enemy player.
  * Right click deselects.
* For "L select, R act":
  * Left click either selects or deselects a unit.
  * Right click either moves your unit, or attempts to throw a tomato at an enemy player.
* Enter ends your turn and moves onto the next player.
* Escape "pauses" the game, mostly so you can move your mouse elsewhere.
* Your units get 7 "steps" each.
  * Moving by 1 tile  uses up 1 step.
  * Throwing a tomato uses up 2 steps.
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

