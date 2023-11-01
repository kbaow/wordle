# Wordle-C

A very simple wordle clone in C.
It doesn't uses any cursor movement instructions or OS dependent functions.

It compiles in Borland Turbo C++ and runs under DOS.

# How to run the program

**make**
**./wordle**

- Make will create and executable named wordle
- Entering "new" will generate a new game which the algorithm will solve within 12 guesses
- Entering "load 'int'" will generate a specific game the algorithm will solve within 12 guesses
- Make clean will remove the executable

## How the Algorithm works
the first guess is set to roate

    Phase 1: Elimination of characters

Our goal here is to remove as many letters as fast as possible. We do so by taking advantage of the 
alphabet we are already given. By using character subtraction we can locate a specific index in the 
alphabet and check if it is used by checking if the value at the index is equal to '_' or a character.

There is also a need to do this same form of character checking on the word itself. For example, we would
not want to waste one of our guesses on the words with repeating characters like aback. So we need to also 
keep track of the letters in the word we want to guess.

To score each word, there are three categories. Unseen and vowel, which is two points, unseen and consonant, 
one point, and seen (both in the alphabet, and in the current words characters).

This is done a total of 6 times to eliminate all characters in the alphabet

    Phase 2: Answer guessing

Our goal here is to use what we know about what characters are in the word and to guess the best possible word.

To do this we have two state variables, a string ans, and a string sig. Ans represents the answer, and stores where
correct letters are, this is done by adding the line of code ans[i] = copy[i]. Sig represents all the letters that 
are in the word regardless of position.

To score answers, letters in the correct postion are 2 points, letters in the word but not in the correct position are 
worth 1 point, everything else is a 0.

This will normally give an answer in 8 guesses, but for games such as number 1397, perch, will take multiple guesses. We do 
not want to keep guessing the same word, so we also use an array of ints seen[52], initialized all to -1, to store 
which filelines we have used as a guess. This ensures we do not guess the same word twice, and will solve (to my knowledge)
all solutions within 12 guesses.

-- Failsafe:

as a precaution, if we can't guess the answer within the 12 tries, the alogirthm will start looping through all of the possible
solutions to garuntee a solution is found, however, I never found a word that was not solved within 12 tries.


## Quick introduction to Wordle

- You have to guess a 5 letter word
- The system tells you which characters in your guess are correct
- You cannot guess random letters, only words from a list

### Character hints

Wordle presents the character hints as follows:

- `_` This character does not exist in the word at all.
- `o` This character exists in the word, but is in the wrong place
- `#` This character is correct

The application is smart in regards to double letter guesses.
This means if you guess `ZORRO` when the word is `CRANE` it will show `__o__` and not `__oo_`

In addition to the basic rules,
this application also shows you which letters you haven't used yet.

Tip: If you already get correct letters after your first two guesses,
do not try to guess words with the letters in the correct places,
instead try to eliminate as many letters from the alphabet as possible within your first 3 guesses.

## Installation

1. Download the appropriate executable from the releases section, or [use gitload to simplify it](https://gitload.net/AyrA/Wordle-C)
2. Create a directory "list" and put the two word lists from the releases in there

## Usage

You're presented with a menu when you launch the application.

Note: This is a console application.
You can double click to run it, but it will close immediately when the game ends.
This means if the game ends on the 6th turn you don't know if you won or lost.

## Game Id

This game presents you with a game id. The id can be used to load the same word again later.
The id is simply the line number in `SOLUTION.TXT`.
Consider randomizing the word order in the solution list,
or people can simply look up the solution with the game id.

**CAUTION! Do not randomize ALL.TXT or the game will break**

## ALL.TXT vs SOLUTION.TXT

`ALL.TXT` contains a list of all accepted words (over 10k words).
The `SOLUTION.TXT` contains the list of all possible words that can be a solution (approx 2k words).

## Speed optimizations

Speed optimizations are important for games that are intended to run on old or undersized machines.

The two most important things are described below.

### Basic checks

First of all, we do not need to scan the word list if the guess is correct.
At this point we can immediately terminate the application with a success message.

The next optimization is to check the word length.
No need for further processing at all if the length is invalid.
Words with invalid length are not counted as guesses.

The final optimization is to check if the input is made up of the characters a-z only.
If not, no need to scan the list. This is not counted as a guess either.

### Cache

This is the most important optimization.
A naive implementation of the game would have to scan the complete word list for every guess.
It also has to scan the solution word list to count the number of possible words for the game id.

A simple caching mechanism is used that takes up 108 bytes.
It uses the fact that the complete word list is alphabetical.

When no cache exists, the complete wordlist is scanned for words.
The words themselves are not of interest, only the first character.
The cache can hold all characters A-Z plus an extra slot,
and for each character the file position of the first word is stored.

This means if a user guesses `HELLO`, we do not need to scan the entire list,
but instead can seek to the first word that begins with `H`,
and stop scanning the list if either `HELLO` is found, or the word no longer starts with `I`.
This cuts the scan time approximately by a factor of 26.

The final extra slot of the cache is used to store the word count in the solution list.

This means once the cache is built, the game starts up instantly.

The result of this is:

- The solution list only needs to be scanned when a game is started to pick the word but not to count words.
- The full word list is only ever scanned for words with the same letter as the current guess.



