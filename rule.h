#ifndef RULE_H
#define RULE_H

#include "structures.h"

// initialize a new Rule
Rule* Rule_init(char* ruleString);

// Execute a rule
char** Rule_execute(Rule* instance, char** tokens, int numberofTokens, int metric, int direction, int* substitutions, int* newNumberOfTokens, int startOffset, int startingClause);

int Rule_cacheBestMetrics(Rule* instance);

#endif
