Parsnip Theory
Copyright (c) 2014, fanzyflani. All rights reserved.
See LICENCE.txt for licensing details.

This program is SHAREWARE. Unfortunately, there is no "registered version" yet, as this is currently in ALPHA TESTING. That means this ISN'T FINISHED.

Please, if you're sharing this program, make sure you share the unmodified files. (You can share modified files, just make sure you have unmodified files handy)
If you accidentally ruin your dat/level.psl file, rip it out of the .zip file again.

----

Windows users: Double-click on RUN.BAT (might just say RUN) to run this. Yes, the thing with the cog on it. A nicer .exe file with a nice icon will come later.

FreeBSD users: Run the "run.sh" file, and cross your fingers. i386 and amd64 builds are provided.

Linux users: Sorry, haven't got that toolchain sorted out yet! Run the Windows version in wine. run.sh will do that step for you.

Mac OS X users: I have no plans for a native port to this platform. If you want there to be a port, I'll need you to test, and also help me set up a cross-compiler. Otherwise, use Wine.

Raspberry Pi users: I haven't got a build for this yet. If you really want one, prod me on Twitter and I'll whip one up. Otherwise... use your other computer.

----

Playing instructions:
* Left click selects a unit.
* Right click either moves your unit, or attempts to throw a tomato at an enemy player.
* Enter ends your turn and moves onto the next player.
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
* 1: Place player 1 spawn
* 2: Place player 2 spawn
* 3: Place player 3 spawn
* 4: Place player 4 spawn
* G: Hold to show grid
* T: Select a tile (press ESC to bail out of this)
* Ctrl-N (Nuke): Clear ALL spawns! (Doesn't clear the tiles)
* Ctrl-L: Load dat/level.psl
* Ctrl-S: Save dat/level.psl

