#ifndef CLAUSE_H
#define CLAUSE_H

#include "structures.h"

// initialize a new Rule
Clause* Clause_init(char* clauseString);

// Create a matcher for the Clause 
int Clause_createMatcher(Clause* instance);

// Attempt to match tokens to this clause
MatchResult* Clause_match(Clause* instance, char** tokens, int numberOfTokens);

#endif
