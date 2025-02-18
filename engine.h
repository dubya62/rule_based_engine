#ifndef ENGINE_H
#define ENGINE_H

#include "structures.h"

#include "database.h"

// initialize a new Engine
Engine* Engine_init(int numberOfDatabaseFiles, char** databaseFilenames);


#endif
