
#include <stdlib.h>

#include "debug.h"
#include "structures.h"

#include "database.h"

///////////////////////////////////////////////////
// Private Functions

// Given a Database instance and file pointer, 
// parse the file into an array of Rules
// and update the instance accordingly
int Database_parse(Database* instance, FILE* fp){
    DBG("Parsing Database File...\n");

    
    

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

    return result;
}


