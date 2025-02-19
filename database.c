
#include <stdlib.h>

#include "debug.h"
#include "structures.h"

#include "rule.h"
#include "database.h"

///////////////////////////////////////////////////
// Private Functions

char* getFileString(FILE* fp, int* fileLength){
    // get file length
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    DBG("fileSize = %ld\n", fileSize);

    // read entire file into a string
    char* fileString = (char*) malloc(sizeof(char) * (fileSize+1));
    fread(fileString, fileSize, 1, fp);
    fileString[fileSize] = '\0';

    if (fileLength != NULL){
        *fileLength = fileSize;
    }

    return fileString;
}

// Given a Database instance and file pointer, 
// parse the file into an array of Rules
// and update the instance accordingly
int Database_parse(Database* instance, FILE* fp){
    DBG("Parsing Database File...\n");

    int numberOfRules = 0;
    Rule** rules = NULL;

    // get file contents
    int fileLength;
    char* fileString = getFileString(fp, &fileLength);
    DBG("File contents:\n");
    DBG("%s\n", fileString);

    // separate the file at semicolons and parse each section as a rule
    int ruleStart = 0;
    for (int i=0; i<fileLength; i++){
        if (fileString[i] == ';'){
            fileString[i] = '\0';
            numberOfRules++;
            rules = realloc(rules, sizeof(Rule*) * numberOfRules);
            rules[numberOfRules-1] = Rule_init(fileString+ruleStart);
            ruleStart = i + 1;
        }
    }

    instance->numberOfRules = numberOfRules;
    instance->rules = rules;

    // memory cleanup
    free(fileString);

    DBG("Finished Parsing Database File...\n");
    return 0;
}


///////////////////////////////////////////////////
// Public Functions

// initialize a new Engine
Database* Database_init(char* filename){
    Database* result = malloc(sizeof(Database));

    DBG("Opening database file: %s\n", filename);

    FILE* fp = fopen(filename, "r");

    // if the file failed to open
    if (fp == NULL){
        PANIC("ERROR: File %s does not exist or could not be opened.\n", filename);
    }

    DBG("File opened successfully.\n");

    // parse the database file into an array of rules
    Database_parse(result, fp);

    // memory cleanup
    fclose(fp);

    DBG("Database fully initialized! (%d Rules)\n", result->numberOfRules);

    return result;
}


