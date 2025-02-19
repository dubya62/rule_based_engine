#ifndef ENGINE_H
#define ENGINE_H

#include "structures.h"

#include "database.h"

// initialize a new Engine
Engine* Engine_init(int numberOfDatabaseFiles, char** databaseFilenames);

// execute the engine on an array of strings
char** Engine_execute(Engine* instance, char** tokens, int numberOfTokens, int metric, int direction, int* newLength);

#endif
