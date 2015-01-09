![RobCo Industries Unified Operating System Terminal](readme_assets/title.png)

---

#Table of Contents

- [Overview](#overview)
- [To-Do](#to-do)
- [Dependencies](#dependencies)

#<a name="overview"></a>Overview

A Fallout terminal emulator as seen in Fallout 3 and Fallout: New Vegas.

I plan to flesh out this program in C, then write at least Ruby and C++ versions of it.

A goal of mine is to make the finished code extremely clean and readable.

##Usage:
Compile with Make from project root:
```
$ make
```

Run program hackterm:
```
$ ./hackterm
```

##Requirements:
- OS: Mac OSX or GNU/Linux
- Terminal: minimum 80 x 24 with ascii characters
- Ncurses library installed
  * Linux: ```sudo apt-get install ncurses```

##Controls:
Use the arrow keys to navigate the board. Use enter to submit the highlighted word as a password.

##Secret codes:

For aspects of the game that I have not implemented yet, I have in place keys that the user can press in order to achieve certain effects:

| Key | Effect                                                  |
| --- | ------------------------------------------------------- |
|  q  | Quit application.                                       |
|  -  | Subtracts one attempt.                                  |
|  +  | Adds one attempt.                                       |
|  a  | Grants access.                                          |

#<a name="to-do"></a>To-Do
Basic features: (Functionality that the actual game has)
- Add lock screen transition where all lines on the screen move up and away.
- Erase word in bottom right when a password is entered, like in-game.
- Print UOS access menu options like in-game.
- Make uniquerandomword() return unique words.
- Make sure that the only matching brackets found on the same line are hacks.
- Hacks with hacks inside of them should contain said hack in the content.
- Passwords can exist inside hacks. These hacks can only be used once the password is removed as a dud.
  * Research: What are the contents of a hack when this happens?
- Research: Does the "Allowance replenished" hack return all allowances or just one?
- Make "!!! WARNING: LOCKOUT IMMINENT !!!" message blink when it appears.


WishList:
- Generate registers dynamically so every game is a little different.
- Move word text files to subdirectory, or consolidate into one file.
- Add capability of running a pseudo-terminal once game is complete.
  * Access some kind of fun files.
- Allow user to type in password if desired.
- Allow user to skip terminal printing sequences.
- Make RobCo UOS centered at all terminal resolutions.
- Add tab-completion for UOS terminal.
- Allow user to control experience from command line args.
  * Option for difficulty settings.
- Add how to install ncurses for requirements mac section.

#<a name="dependencies"></a>Dependencies

+ [ncurses](http://www.gnu.org/software/ncurses/) - Great library for cursor management.