![RobCo Industries Unified Operating System Terminal](readme_assets/title.png)

---

#Table of Contents

- [Overview](#overview)
- [To-Do](#to-do)
- [Dependencies](#dependencies)

#<a name="overview"></a>Overview

A Fallout terminal emulator as seen in Fallout 3 and Fallout: New Vegas.

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
| Key | Effect                  |
| --- | ----------------------- |
|  q  | Quit application.       |
|  -  | Subtract one attempt.   |
|  +  | Add one attempt.        |
|  a  | Grant access.           |

#<a name="to-do"></a>To-Do
Basic features: (Functionality that the actual game has)
- Add lock screen transition where all lines on the screen move up and away.
- Erase word in bottom right when a password is entered, like in-game.
- Print UOS access menu options like in-game.
- Make uniqueRandomWord() return unique words.
- Make sure that the only matching brackets found on the same line are hacks.
- Hacks with hacks inside of them should contain said hack in the content.
- Passwords can exist inside hacks. These hacks can only be used once the password is removed as a dud.
  * Research: What are the contents of a hack when this happens?
- Research: Does the "Allowance replenished" hack return all allowances or just one?
- Make "!!! WARNING: LOCKOUT IMMINENT !!!" message blink when it appears.


WishList:
- Generate registers dynamically so every game is a little different.
- Add access to vault entries once access is granted.
- Allow user to type in passwords if desired.
- Allow user to skip terminal printing sequences.
- Add tab-completion for UOS terminal.
- Allow user to control experience from command line args.
  * Option for difficulty settings.

#<a name="dependencies"></a>Dependencies

+ [ncurses](http://www.gnu.org/software/ncurses/) - Great library for cursor management.