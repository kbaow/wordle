//Used for exit constants mostly
#include <stdlib.h>
//printf and puts
#include <stdio.h>
//string comparison functions
#include <string.h>
//Used to seed the rng for random games
#include <time.h>

//Length of a word.
//Note: You cannot change this without changing the word list too.
//You also must adapt the scanf calls for the new length
#define WORD_LENGTH 5
//Number of tries allowed to find the word
#define MAX_TRIES 12
//Number of characters in the alphabet + 1
#define ALPHA_SIZE 27
//If set, the word and a few stats are printed before the game starts
//#define DEBUG

//Very cheap error termination script
#define err(x) fprintf(stderr, EOL "[%s:%i] Fatal error: %s" EOL, __FILE__, __LINE__, x);abort();

//Note: CRLF is also used for telnet.
//If you want to make it available in a BBS you may want to force a CRLF
#ifdef WIN32
#define EOL "\r\n"
#else
#define EOL "\n"
#endif

//Cheap boolean
#define bool int
#define false (0)
#define true (!false)

//Files for lists that contain all words and solutions
FILE * fpAll,  * fpSol;
//Number of words in the solution list
long wordCount = 0;
//Selected word from solution list
char word[WORD_LENGTH + 1] = {0};
//Possible characters (used to show unused characters)
//The size specifier is necessary or its value will be readonly
char alpha[ALPHA_SIZE] = "abcdefghijklmnopqrstuvwxyz";
char sig[ALPHA_SIZE] = "abcdefghijklmnopqrstuvwxyz";
char ans[WORD_LENGTH + 1] = "_____";
//Memory cache:
//0-25 File position in the complete list with words that start with the given letter a-z
//26: Number of words in the solution list
long memcache[ALPHA_SIZE];

//Reads solution list and picks a random word
long setup(void);
//Pick the given word
int pickWord(char * word, int index);
//Checks if the supplied word is in the list
bool hasWord(const char * word);
//Convert to lowercase
int toLower(char * str);
//Checks the entered word against the solution
bool checkWord(const char * guess);
//Checks if the entered string is a potentially valid word
bool isWord(const char* word);
//Gets the first position of the given char in the given string
int strpos(const char * str, char search);
//Removes characters in the supplied argument from the alphabet
void removeAlpha(const char * guess);
//Runs the main game loop
void gameLoop(void);
//Runs the menu
int menu(void);
//Shows the help text
void help(void);

char * guesser(int n);
int isVowel(char c);
int isAlpha(char c);
int scoreFind(char * word);
int scoreAns(char * word);
int isSeen(int n);
int seen[6] = {-1, -1, -1, -1, -1, -1};
// current number of guesses for answer
int currAns = 0;

//Main function
int main() {
	int gameId;
	setbuf(stdout, NULL);
	//Note: This will search for the file in the current directory
#ifdef ORIGINAL_SRC
	fpAll = fopen("LISTS\\ALL.TXT", "r");
	fpSol = fopen("LISTS\\SOLUTION.TXT", "r");
#else
	fpAll = fopen("./lists/ALL.TXT", "r");
	fpSol = fopen("./lists/SOLUTION.TXT", "r");
#endif /* starting the game */
	
	if (fpAll == NULL || fpSol == NULL) {
		err("error opening wordle lists");
	}
	#ifdef DEBUG
	printf("Word count: %i" EOL, setup());
	#else
	setup();
	#endif
	gameId = menu();
	if (gameId >= 0) {
		pickWord(word, gameId);
		#define DEBUG
		#ifdef DEBUG
		printf("Word: %s" EOL, word);
		#endif
	} else {
		return EXIT_SUCCESS;
	}
	printf("Running game #%i" EOL, gameId + 1);
	gameLoop();
	fclose(fpAll);
	fclose(fpSol);
	return EXIT_SUCCESS;
}

/*
* Starting the initial game minu
* Multiple options for different game types
*/
int menu() {
	char buffer[21];
	int gameId = 0;
	int scan = 0;
	puts(
		"Main Menu" EOL
		"=========" EOL
		"NEW: start a new game." EOL
		"LOAD <num>: load a specific game" EOL
		"HELP: More information" EOL
		"EXIT: End game");
	printf("The game number must be in range of 1-%li" EOL, wordCount);
	while (true) {
		printf("Input: ");
		while ((scan = scanf("%20s", buffer)) != 1) {
			if (scan == EOF) {
				return -1;
			}
		}
		toLower(buffer);
		if (strcmp(buffer, "exit") == 0) {
			return -1;
		} else if (strcmp(buffer, "help") == 0) {
			help();
		} else if (strcmp(buffer, "new") == 0) {
			return rand() % wordCount;
		} else if (strcmp(buffer, "load") == 0) {
			if (scanf("%i",  & gameId) == 1) {
				if (gameId > 0 && gameId <= wordCount) {
					return gameId - 1;
				}
			}
			puts("Invalid number");
		} else {
			puts("Invalid input");
		}
	}
}

void help() {
	printf("Wordle is a simple game:" EOL "Guess the %i letter word within %i tries" EOL, WORD_LENGTH, MAX_TRIES);
	puts(
		"After every guess, hints are shown for each character." EOL
		"They look like this:" EOL
		"  _ = Character not found at all" EOL
		"  # = Character found and position correct" EOL
		"  o = Character found but position wrong" EOL
		"Unused letters of the alphabet are shown next to the hint" EOL
		EOL
		"The game prefers valid positions over invalid positions," EOL
		"And it handles double letters properly." EOL
		"Guessing \"RATES\" when the word is \"TESTS\" shows \"__oo#\"");
}

/* 
* Game loop, automatically guesses a wordle word
*/
void gameLoop() {
	char guess[WORD_LENGTH + 1] = {0};
	int guesses = 0;
	int scan = 0;
	puts(
		"word\tunused alphabet" EOL
		"====\t===============");
	while (guesses < MAX_TRIES && strcmp(guess, word)) {
		printf("Guess %i: ", guesses + 1);
		printf("%s ", guesser(guesses));

		if (true) {
			strcpy(guess, guesser(guesses));		  
			toLower(guess);
			// Do not bother doing all the test logic if we've found the word.
			if (strcmp(guess, word)) {
				if (isWord(guess) && hasWord(guess)) {
					++guesses;
					//TODO: Check guess against word
					if (checkWord(guess)) {
						removeAlpha(guess);
						printf("\t%s\n", alpha);
					}
				} else {
					puts("Word not in list");
				}
			}
		} else {
			if (scan == EOF) {
				exit(EXIT_FAILURE);
			}
			printf("Invalid word. Must be %i characters\n", WORD_LENGTH);
		}
	}
	if (strcmp(guess, word)) {
		printf("You lose. The word was %s\n", word);
	} else {
		puts("You win");
	}
}

/*
* check out of the guesses if word was already used out of possible words
*/
int isSeen(int n){
	for (int i = 0; i < 7; i++){
		if (n == seen[i]){
			return 1;
		}
	}
	return 0;
}

/*
* @param n is the guess number
* Algorithm to pick the best possible guess
* Weighted on missing vowels (2 points) and missing consonants (1 point)
* If no guesses have been made, guess roate
*/
char * guesser(int n){
  char * best = (char*) malloc((WORD_LENGTH + 4) * sizeof(char));
  char * buffer = (char*) malloc((WORD_LENGTH + 4) * sizeof(char));
  int score = 0;
  int fileline = 0;
  fseek(fpAll, 0, SEEK_SET);
  fseek(fpSol, 0, SEEK_SET);
  
  if (n == 0){
    return "roate";
  }

  else if (n > 0 && n < 7){
    while (fgets(buffer, WORD_LENGTH + 4, fpAll) != NULL){
      if (scoreFind(buffer) > score){
        score = scoreFind(buffer);
        strcpy(best, buffer);
		best[WORD_LENGTH] = 0;
      }
    }
    return best;
  }
  else{
    while (fgets(buffer, WORD_LENGTH + 4, fpSol) != NULL){
		if (scoreAns(buffer) > score){
        	score = scoreAns(buffer);
			strcpy(best, buffer);
			best[WORD_LENGTH] = 0;
        }
	  }
	  fileline += 1;
    }
    return best;
  }

/*
* scoreFind calulates the score of a word (for letters that are not yet guessed, before 7 guesses)
* Vowels are worth 2 points, consonants are worth 1
* Duplicates are not counted
*/
int scoreFind(char * word){
  char localSeen[ALPHA_SIZE] = "abcdefghijklmnopqrstuvwxyz";
  int score = 0;

  for (int i = 0; i < WORD_LENGTH + 1; i ++){
    if ((alpha[word[i] - 'a'] != '_') && localSeen[word[i] - 'a'] != '_'){
      if (isVowel(word[i])){
        score += 2;
        localSeen[strpos(localSeen, word[i])] = '_';
      }
      else{
        score += 1;
        localSeen[strpos(localSeen, word[i])] = '_';
      }
    }
  }
  return score;
}

/*
* scoreAns calculates the score of a word (when we have guessed more than 7 times)
* Letters that are in the answer are worth 2 points
* Otherwise it is one point
*/
int scoreAns(char * word){
	int score = 0;
	for (int i = 0; i < WORD_LENGTH + 1; i ++){
		if (word[i] == ans[i]){
			score += 2;
		}
		else if (sig[word[i] - 'a'] == '_'){
			score += 1;
		}
    }
	return score;
}

/*
* @param c is a character
* Check if c is Alpha
*/
int isAlpha(char c){
  if (c - 'a' >= 0 || c - 'a' < 26){
    return 1;
  }
  return 0;
}

/*
* @param c is a character
* Check if c is Vowel
*/
int isVowel(char c){
  if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u'){
    return 1;
  }
  return 0;
}

/*
* @param c is a character
* Remove the alpha characters that have been guessed
*/
void removeAlpha(const char * guess) {
	int i = 0;
	int pos = 0;
	if (guess != NULL) {
		while (guess[i]) {
			pos = strpos(alpha, guess[i]);
			if (pos >= 0) {
				alpha[pos] = '_';
			}
			++i;
		}
	}
}

/*
* strpos caluclates the position of a guess in the file
* @param str guess string
*/
int strpos(const char * str, char search) {
	int i = 0;
	if (str != NULL) {
		while (str[i]) {
			if (str[i] == search) {
				return i;
			}
			++i;
		}
	}
	return -1;
}

/*
* checkWord checks a guess with a word, displaying it on the game
* @param guess is the guess
*/
bool checkWord(const char * guess) {
	if (strlen(guess) == strlen(word)) {
		int i = 0;
		int pos = -1;
		//Copy is used to blank found characters
		//This avoids wrong reports for double letters, for example "l" in "balls"
		char copy[WORD_LENGTH + 1];
		char result[WORD_LENGTH + 1];
		result[WORD_LENGTH] = 0;
		strcpy(copy, word);
		//Do all correct positions first
		while (copy[i]) {
			if (copy[i] == guess[i]) {
				//Character found and position correct
				ans[i] = copy[i];
				result[i] = '#';
				copy[i] = '_';
				
			} else {
				//Fills remaining slots with blanks
				//We could do this before the loop as well
				result[i] = '_';
				sig[copy[i] - 'a'] = '_';
			}
			++i;
		}
		i = 0;
		while (copy[i]) {
			pos = strpos(copy, guess[i]);
			//Char must exist but do not overwrite a good guess
			if (pos >= 0 && result[i] != '#') {
				//Character found but position wrong
				result[i] = 'o';
				copy[pos] = '_';
			}
			++i;
		}
		printf("%s", result);
		return true;
	}
	return false;
}

/*
* toLower converts a string to lowercase
* @param str is the string
* returns length of str
*/
int toLower(char * str) {
	int i = 0;
	while (str[i]) {
		if (str[i] >= 'A' && str[i] <= 'Z') {
			str[i] |= 0x20; //Make lowercase
		}
		++i;
	}
	return i;
}

/*
* isSame checks if two words are the same, 0 if not, 1 if yes
* @param w1, w2 are the words to compare
*/
int isSame(char * w1, char * w2){
	for (int i = 0; i < WORD_LENGTH; i ++){
		if (w1[i] != w2[i]){
			return 0;
		}
	}
	return 1;
}

/*
* hasWord checks if a word is in the 
*/
int hasWord(const char * word) {
	//A bit longer to also contain the line terminator
	char buffer[WORD_LENGTH + 4];
	//Don't bother if the argument is invalid
	if (word == NULL || strlen(word) != WORD_LENGTH || !isWord(word)) {
		return false;
	}
	fseek(fpAll, memcache[word[0]-'a'], SEEK_SET);
	//Stop comparing once we are beyond the current letter
	while (fgets(buffer, WORD_LENGTH + 4, fpAll) != NULL && buffer[0]==word[0]) {
		buffer[WORD_LENGTH]=0;
		
		if (strcmp(word, buffer)){
			return true;
		}
	}
	return false;
}
	
bool isWord(const char* word){
	int i=-1;
	if(strlen(word) == WORD_LENGTH){
		while(word[++i]){
			if(word[i]<'a' || word[i]>'z'){
				return false;
			}
		}
		return true;
	}
	return false;
}

/* 
* setup the game
*/
long setup() {
	FILE* cache;
	char currentChar;
	char currentWord[WORD_LENGTH + 1];
	bool success = false;
	
	//Don't bother if setup was already performed
	if (wordCount > 0) {
		return wordCount;
	}
	srand((int)time(0));
	
	if ((cache = fopen("LISTS\\CACHE.BIN","rb")) != NULL) {
		printf("Reading cache... ");
	    success = fread(memcache, sizeof(long), ALPHA_SIZE, cache) == ALPHA_SIZE;
	    fclose(cache);
		if(success){
			puts(" [OK]");
			return wordCount = memcache[ALPHA_SIZE - 1];
		}
		else{
			puts(" [FAIL]");
		}
	}
	
	printf("Loading word list...");
	fseek(fpSol, 0, SEEK_SET);
	while (fgets(currentWord, WORD_LENGTH + 1, fpSol) != NULL) {
		//Only increment if word length is correct
		if (strlen(currentWord) == WORD_LENGTH) {
			++wordCount;
		}
	}
	puts(" [OK]");
	memcache[ALPHA_SIZE-1] = wordCount;

	if (!success) {
	    printf("Building cache...");
		currentChar = 'a';
		memcache[0] = 0;
		fseek(fpAll, 0, SEEK_SET);
		while (fgets(currentWord, WORD_LENGTH + 1, fpAll) != NULL) {
			//Only proceed if word length is correct
			if (strlen(currentWord) == WORD_LENGTH) {
			    if (currentChar != currentWord[0]) {
			        currentChar = currentWord[0];
			        memcache[currentChar - 'a'] = ftell(fpAll) - 5;
			    }
			}
		}
		cache = fopen("LISTS\\CACHE.BIN", "wb");
		if (cache == NULL) {
			puts(" [FAIL]");
		}
		else{
			fwrite(memcache, sizeof(long), ALPHA_SIZE, cache);
			fclose(cache);
			puts(" [OK]");
		}
	}
	return wordCount;
}

/*
* pickWord finds the word, and records the index
* @param word is the guess
* @param index is the index of the word
* Returns the index of the word in the list
*/
int pickWord(char * word, int index) {
	int i = 0;
	fseek(fpSol, 0, SEEK_SET);
	while (i <= index && fgets(word, WORD_LENGTH + 1, fpSol) != NULL) {
		if (strlen(word) == WORD_LENGTH) {
			++i;
		}
	}
	return index;
}
