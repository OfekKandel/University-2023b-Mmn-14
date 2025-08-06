# Open University Final Project in C
My final proejct in C for course 20465 מעבדה בתכנות מערכות in the Open University of Israel, which I took in 2024.  
An overview of the code structure is explained at the top of `main.c`, and the public functions of every file are described in its header file for readability and as per the instructions.

## The Assignment
The exact instructions for the project are under `instructions/Mmn 14 Instructions.pdf`.
The project was to build an assembler for an assembly-style language, which takes in a `.as` file and outputs:
* A `.ob` file - the assembled binary file, written as lines of 7 characters `*`,`!`,`%`,`#` each representing a different combination of 2 bits.
* A `.ent` file - containing every symbol declared as an entry point (`.entry`) and its position
* A `.ext` file - containing every symbol declared as external (`.external`) and its position
* A `.am` file - if the `.as` contains macros, the `.am` is the `.as` file with the macros expanded

**Note**: This project works on all OSes but was built on and in order to run on an Ubuntu Linux system.
