
#include <stdlib.h>

#include "debug.h"
#include "structures.h"

#include "database.h"
#include "engine.h"

///////////////////////////////////////////////////
// Private Functions


///////////////////////////////////////////////////
// Public Functions

// initialize a new Engine
Engine* Engine_init(int numberOfDatabaseFiles, char** databaseFilenames){
    Engine* result = malloc(sizeof(Engine));

    // initialize the database files
    result->databases = malloc(sizeof(Database*) * numberOfDatabaseFiles);
    for (int i=0; i<numberOfDatabaseFiles; i++){
        result->databases[i] = Database_init(databaseFilenames[i]);
    }

    return result;
}

