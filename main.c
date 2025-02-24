
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
#include <string.h>

#include "debug.h"
#include "structures.h"

#include "engine.h"

int numberOfDatabaseFiles;
char** databaseFilenames;

int cliMetric;
int cliDirection;


int printUsage(){
    printf("Usage:\n");
    printf("\t./rbe <metric> <direction> <rule_database1> <rule_database2> ... <rule_databaseN>\n");

    return 0;
}


int parseCliArgs(int argc, char** argv){
    if (argc < 4){
        printf("Not enough args supplied.\n");
        printUsage();
        return 1;
    }

    numberOfDatabaseFiles = argc - 3;
    databaseFilenames = (char**) malloc(sizeof(char*) * numberOfDatabaseFiles);

    cliMetric = atoi(argv[1]);
    if (cliMetric < 0){
        printf("Metric must be a non-negative integer.\n");
        printUsage();
        return 1;
    }

    cliDirection = atoi(argv[2]);
    switch(cliDirection){
        case -1:
        case 1:
            break;
        default:
            printf("Direction must be either -1 or 1.\n");
            printUsage();
            return 1;
    }

    for (int i=3; i<argc; i++){
        databaseFilenames[i-3] = argv[i];
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

    DBG("Awaiting input tokens...\n");


    while (1){
        char* line = NULL;
        size_t lineLength;

        size_t bytesRead = getline(&line, &lineLength, stdin);

        // EOF reached
        if (bytesRead == -1){
            free(line);
            return 0;
        }

        if (bytesRead > 0){
            line[bytesRead - 1]  = '\0';
        }

        // break the line up into tokens at spaces
        int numberOfInputTokens = 1;
        char** inputTokens;
        for (int i=0; i<bytesRead; i++){
            if (line[i] == ' '){
                numberOfInputTokens++;
            }
        }

        inputTokens = (char**) malloc(sizeof(char*) * numberOfInputTokens);

        DBG("Received input. Parsing...\n");
        DBG("Input:\n%s\n", line);
        DBG("Parsing into %d tokens\n", numberOfInputTokens);

        int tokenStart = 0;
        int currentInputToken = 0;
        for (int i=0; i<bytesRead; i++){
            if (line[i] == ' '){
                int tokenLength = i - tokenStart;
                inputTokens[currentInputToken] = (char*) malloc(sizeof(char) * (tokenLength + 1));

                strncpy(inputTokens[currentInputToken], line+tokenStart, tokenLength);
                inputTokens[currentInputToken][tokenLength] = '\0';

                tokenStart = i + 1;
                currentInputToken++;
            }
        }
        int tokenLength = bytesRead - tokenStart;
        inputTokens[currentInputToken] = (char*) malloc(sizeof(char) * (tokenLength + 1));

        strncpy(inputTokens[currentInputToken], line+tokenStart, tokenLength);
        inputTokens[currentInputToken][tokenLength] = '\0';

        
        DBG("Executing engine on input...\n");

        int newLength;
        char** result = Engine_execute(engine, inputTokens, numberOfInputTokens, cliMetric, cliDirection, &newLength);

        DBG("FINAL RESULT:\n");
        for (int i=0; i<newLength; i++){
            printf("%s ", result[i]);
        }
        printf("\n");

        fflush(stdout);
        free(line);
        free(inputTokens);
    }

    return 0;
}


