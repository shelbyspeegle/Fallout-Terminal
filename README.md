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

###Secret codes:

For aspects of the game that I have not implemented yet, I have in place keys that the user can press in order to achieve certain effects:

| Key | Effect                                                  |
| --- | ------------------------------------------------------  |
| 'q' | Quit application.                                       |
| 'e' | Prints a dummy "Dud removed." message to messages area. |
| '-' | Subtracts one attempt.                                  |

#<a name="to-do"></a>To-Do
- Allow user to control experience from command line args.

#<a name="dependencies"></a>Dependencies

+ [ncurses](http://www.gnu.org/software/ncurses/) - Great library for cursor management.