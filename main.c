
/**
Rule based engine:

- This program should take any number of rule database filenames, open, and compile them to quickly executable statements

It should have the following syntax:
    . - matches any token
    <token>+ - the token must be matched 1 or more times
    <token>* - the token must be matched 0 or more times
    <token1>|<token2> - matches token1 or token2
    <token>{<num1>,<num2>} - the token must be matched <num1> to <num2> times
    <token>$<varnum> - the token will be bound to the varnum
    #<token> - internal variable. Will be changed to #<currentInternalVariable> by the program

    This is the default functionality of these tokens. To escape this functionality, prepend a backslash:
        [".", "+", "*", "|", "{", "$", "#", '"']
    
The rule database should be made of Rules
    Each Rule is made up of Clauses
        Each Clause is an array of tokens with heuristic values appended to it
        Each Clause should be separated by "eq" (without the quotes)
        Each Clause should have a ; at the end
        Each Clause's Tokens should be separated by spaces and surrounded with quotes
        The heuristic metrics should be appended using : (and each value separated with colons)
            Metric value of _ is equivalent to -1 (empty)
    # starts a single line comment when outside of quotes

 
*/


#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "structures.h"

#include "engine.h"

int numberOfDatabaseFiles;
char** databaseFilenames;


int printUsage(){
    printf("Usage:\n");
    printf("\t./rbe <rule_database1> <rule_database2> ... <rule_databaseN>\n");

    return 0;
}


int parseCliArgs(int argc, char** argv){
    if (argc < 2){
        perror("Not enough args supplied.\n");
        printUsage();
        return 1;
    }

    numberOfDatabaseFiles = argc - 1;
    databaseFilenames = (char**) malloc(sizeof(char*) * numberOfDatabaseFiles);

    for (int i=1; i<argc; i++){
        databaseFilenames[i-1] = argv[i];
        DBG("Database file added: %s\n", argv[i]);
    }

    return 0;
}


// Take in command line args to get the filenames of all rule databases to compile
// After that, start accepting standard input as input to the rule based engine
int main(int argc, char** argv){
    DBG("Hello, World!\n");

    int parseError = parseCliArgs(argc, argv);
    if (parseError){
        return parseError;
    }

    DBG("Creating Engine...\n");
    Engine* engine = Engine_init(numberOfDatabaseFiles, databaseFilenames);

    DBG("Rule Based Engine is fully initialized!\n");

    char* testTokens[5];
    testTokens[0] = "Hello";
    testTokens[1] = "World";
    testTokens[2] = "how";
    testTokens[3] = "are";
    testTokens[4] = "you?";
    int numberOfTokens = 5;

    int newLength;
    char** result = Engine_execute(engine, testTokens, numberOfTokens, 0, -1, &newLength);

    /*
    DBG("\nRESULT:");
    int i = 0;
    while (result[i] != NULL){
        DBG("%s ", result[i]);
        i++;
    }
    DBG("\n");
    */


    return 0;
}


