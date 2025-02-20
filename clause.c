#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "debug.h"
#include "structures.h"

#include "clause.h"

///////////////////////////////////////////
// Private Functions

int notValidMetric(char c){
    switch(c){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
        case '_':
        case '-':
        case ' ':
        case '\t':
            return 0;
    }
    return 1;
}


float parseMetric(char* metricString){
    // parse a metric value. need to ignore comments, spaces, tabs, newlines
    int n = strlen(metricString);

    if (!n){
        return -1.0;
    }

    int placementIndex = 0;
    char* cleaned = (char*) malloc(sizeof(char) * (n + 1));


    int comment = 0;
    int i = 0;
    while (i < n){
        switch(metricString[i]){
            case '#':
                comment = 1;
                break;
            case '\n':
                comment = 0;
                break;
            case ' ':
            case '\t':
                break;
            case '_':
                free(cleaned);
                return -1.0;
            default:
                // add the character to the string
                if (!comment){
                    cleaned[placementIndex] = metricString[i];
                    placementIndex++;
                }
                break;
        }
        i++;
    }
    // take care of null char
    cleaned[placementIndex] = '\0';

    // try to convert the string to a float or throw an error
    char* endptr = NULL;
    float theFloat = strtof(cleaned, &endptr);
    if (endptr == cleaned){
        PANIC("ERROR: invalid metric value in clause: %s\n", cleaned);
    }

    // memory cleanup
    free(cleaned);

    return theFloat;
}


int Clause_parse(Clause* instance, char* clauseString){
    int n = strlen(clauseString);

    int numberOfTokens = 0;
    char** tokens = NULL;
    int numberOfMetrics = 0;
    float* metrics = NULL;

    int i = 0;
    int quotes = 0;
    int comment = 0;
    int tokenStart = 0;
    int done = 0;
    int backslashes = 0;

    // parse tokens
    while (i < n){
        switch(clauseString[i]){
            case '\\':
                backslashes++;
                break;
            case '"':
                if (!comment && backslashes % 2 == 0){
                    quotes ^= 1;
                    if (quotes){
                        tokenStart = i + 1;
                    } else {
                        // add the current token
                        int tokenSize = i - tokenStart;
                        char* theToken = (char*) malloc(sizeof(char) * (tokenSize + 1));
                        strncpy(theToken, clauseString+tokenStart, tokenSize);
                        theToken[tokenSize] = '\0';

                        numberOfTokens++;
                        tokens = realloc(tokens, numberOfTokens * sizeof(char*));
                        tokens[numberOfTokens-1] = theToken;
                    }
                }
                break;
            case '#':
                if (!quotes){
                    comment = 1;
                }
                break;
            case '\n':
                comment = 0;
                break;
            case ' ':
                if (quotes && !comment){
                    // add the current token
                    int tokenSize = i - tokenStart;
                    char* theToken = (char*) malloc(sizeof(char) * (tokenSize + 1));
                    strncpy(theToken, clauseString+tokenStart, tokenSize);
                    theToken[tokenSize] = '\0';

                    numberOfTokens++;
                    tokens = realloc(tokens, numberOfTokens * sizeof(char*));
                    tokens[numberOfTokens-1] = theToken;

                    tokenStart = i + 1;

                }
                break;
            case '~':
                if (!quotes && !comment){
                    done = 1;
                }
                break;
        }
        if (clauseString[i] != '\\'){
            backslashes = 0;
        }
        i++;
        if (done){
            break;
        }
    }

    DBG("Finshed parsing tokens (found %d)\n", numberOfTokens);

    // parse the metrics
    int metricStart = i;
    while (i < n){
        switch (clauseString[i]){
            case '#':
                comment = 1;
                break;
            case '\n':
                comment = 0;
                break;
            case ':':
                if (!comment){
                    numberOfMetrics++;
                    metrics = realloc(metrics, sizeof(float) * numberOfMetrics);
                    clauseString[i] = '\0';
                    metrics[numberOfMetrics-1] = parseMetric(clauseString + metricStart);
                    metricStart = i + 1;
                }
                break;
            case '_':
                break;
            default:
                if (!comment && notValidMetric(clauseString[i])){
                    PANIC("ERROR: Invalid metric value for clause: %s", clauseString);
                }
                break;
        }
        i++;
    }

    if (metricStart < n){
        numberOfMetrics++;
        metrics = realloc(metrics, sizeof(float) * numberOfMetrics);
        metrics[numberOfMetrics-1] = parseMetric(clauseString + metricStart);
    }

    DBG("Finshed parsing metrics (found %d)\n", numberOfMetrics);

    instance->numberOfTokens = numberOfTokens;
    instance->tokens = tokens;
    instance->numberOfMetrics = numberOfMetrics;
    instance->metrics = metrics;

    return 0;
}

///////////////////////////////////////////
// Public Functions

// create new clause from string
Clause* Clause_init(char* clauseString){
    Clause* result = (Clause*) malloc(sizeof(Clause));

    DBG("Initializing clause: %s\n", clauseString);

    // parse the tokens as strings
    Clause_parse(result, clauseString);

    DBG("Displaying Tokens and metrics:\n");
    for (int i=0; i<result->numberOfTokens; i++){
        DBG("token: %s\n", result->tokens[i]);
    }
    for (int i=0; i<result->numberOfMetrics; i++){
        DBG("metric: %f\n", result->metrics[i]);
    }

    DBG("Clause is fully initialized...\n");

    return result;
}


int Clause_createMatcher(Clause* instance){
    Matcher* result = (Matcher*) malloc(sizeof(Matcher));

    result->minRepetitions = (int*) malloc(sizeof(int) * instance->numberOfTokens);
    result->maxRepetitions = (int*) malloc(sizeof(int) * instance->numberOfTokens);
    result->variableAccesses = (int*) malloc(sizeof(int) * instance->numberOfTokens);
    result->internalVariables = (int*) malloc(sizeof(int) * instance->numberOfTokens);
    result->numberOfMatchingTokens = (int*) malloc(sizeof(int) * instance->numberOfTokens); 
    result->matchingTokens = (char***) malloc(sizeof(char**) * instance->numberOfTokens);

    DBG("Creating Matcher...\n");
    for (int i=0; i<instance->numberOfTokens; i++){

        char* currentToken = instance->tokens[i];
        DBG("checking token: %s\n", currentToken);

        result->minRepetitions[i] = 1;
        result->maxRepetitions[i] = 1;
        result->variableAccesses[i] = -1;
        result->internalVariables[i] = -1;

        result->numberOfMatchingTokens[i] = 0;
        result->matchingTokens[i] = NULL;

        int tokenLength = strlen(currentToken);
        
        int placementIndex = 0;
        char* newToken = (char*) malloc(sizeof(char) * (tokenLength+1));

        int numberOfMatchingTokens = 0;
        char** matchingTokens = NULL;

        int anyAllowed = 0;

        int backslashes = 0;
        int j = 0;
        int done = 0;
        while (currentToken[j] != '\0'){
            DBG("  char: %c\n", currentToken[j]);
            switch(currentToken[j]){
                case '\\':
                    backslashes++;
                    if (backslashes % 2 == 0){
                        newToken[placementIndex] = currentToken[j];
                        placementIndex++;
                        backslashes = 0;
                    }
                    break;
                case '.':
                    if (backslashes % 2 == 0){
                        result->matchingTokens[i] = NULL;
                        anyAllowed = 1;
                    } else {
                        newToken[placementIndex] = currentToken[j];
                        placementIndex++;
                    }
                    break;
                case '+':
                    if (backslashes % 2 == 0){
                        result->minRepetitions[i] = 1;
                        result->maxRepetitions[i] = INT_MAX;
                    } else {
                        newToken[placementIndex] = currentToken[j];
                        placementIndex++;
                    }
                    break;
                case '*':
                    if (backslashes % 2 == 0){
                        result->minRepetitions[i] = 0;
                        result->maxRepetitions[i] = INT_MAX;
                    } else {
                        newToken[placementIndex] = currentToken[j];
                        placementIndex++;
                    }
                    break;
                case '$':
                    if (backslashes % 2 == 0){
                        int theVarnum = atoi(currentToken+j+1);
                        result->variableAccesses[i] = theVarnum;
                        done = 1;
                    } else {
                        newToken[placementIndex] = currentToken[j];
                        placementIndex++;
                    }
                    break;
                case '{':
                    if (backslashes % 2 == 0){
                        int k = j;
                        int commaIndex = k+1;
                        while (currentToken[k] != '\0'){
                            if (currentToken[k] == ','){
                                currentToken[k] = '\0';
                                result->minRepetitions[i] = atoi(currentToken+j);
                                commaIndex = k+1;
                            } else if (currentToken[k] == '}'){
                                currentToken[k] = '\0';
                                result->maxRepetitions[i] = atoi(currentToken+commaIndex);
                                k++;
                                break;
                            }
                            k++;
                        }
                        j = k;
                    } else {
                        newToken[placementIndex] = currentToken[j];
                        placementIndex++;
                    }
                    break;
                case '|':
                    if (backslashes % 2 == 0){
                        numberOfMatchingTokens++;
                        matchingTokens = realloc(matchingTokens, sizeof(char*) * numberOfMatchingTokens);
                        matchingTokens[numberOfMatchingTokens-1] = newToken;
                        newToken[0] = '\0';
                        placementIndex = 0;
                    } else {
                        newToken[placementIndex] = currentToken[j];
                        placementIndex++;
                    }
                    break;
                case '#':
                    if (backslashes % 2 == 0){
                        result->internalVariables[i] = atoi(currentToken+j+1);
                    } else {
                        newToken[placementIndex] = currentToken[j];
                        placementIndex++;
                    }
                    break;
                default:
                    newToken[placementIndex] = currentToken[j];
                    placementIndex++;
                    break;
            }

            if (currentToken[j] != '\\'){
                backslashes = 0;
            }

            if (done){
                break;
            }

            j++;
        }
        newToken[placementIndex] = '\0';



        free(instance->tokens[i]);
        instance->tokens[i] = newToken;

        if (!anyAllowed){
            numberOfMatchingTokens++;
            matchingTokens = realloc(matchingTokens, sizeof(char*) * numberOfMatchingTokens);
            matchingTokens[numberOfMatchingTokens-1] = newToken;
        }

        result->numberOfMatchingTokens[i] = numberOfMatchingTokens;
        result->matchingTokens[i] = matchingTokens;

        DBG("\tminRepetitions = %d\n\tmaxRepetitions = %d\n\tvariableAccesses = %d\n\tinternalVariables = %d\n\tnumberOfMatchingTokens = %d\n\tmatchingTokens = %p\n", result->minRepetitions[i], result->maxRepetitions[i], result->variableAccesses[i], result->internalVariables[i], result->numberOfMatchingTokens[i], result->matchingTokens[i]);
        DBG("\tnewToken: %s\n", newToken);
    }


    instance->matcher = result;
    return 0;
}


// TODO: THESE ARE THE MOST PERFORMANCE CRITICAL FUNCTIONS
    // it would be good to come back later and make it more efficient

int tokenMatches(Matcher* matcher, char* token, int currentRepetition){
    DBG("Checking token match...\n");
    DBG("\t%d matching tokens\n", matcher->numberOfMatchingTokens[currentRepetition]);
    DBG("\ttoken to match: %s\n", token);
    if (matcher->matchingTokens[currentRepetition] == NULL || matcher->numberOfMatchingTokens[currentRepetition] == 0){
        DBG("TOKEN MATCHES (ANY)\n");
        return 1;
    }

    for (int i=0; i<matcher->numberOfMatchingTokens[currentRepetition]; i++){
        if (!strcmp(matcher->matchingTokens[currentRepetition][i], token)){
            DBG("TOKEN MATCHES\n");
            return 1;
        }
    }

    DBG("TOKEN DOES NOT MATCH\n");
    return 0;
}

// Attempt to match to the start of the given tokens
MatchResult* Clause_matchHelper(Clause* instance, char** tokens, int numberOfTokens){
    int currentRepetition = 0;
    int* repetitions = (int*) malloc(sizeof(int) * instance->numberOfTokens);
    for (int i=0; i<instance->numberOfTokens; i++){
        repetitions[i] = 0;
    }

    int latestToken = 0;

    Matcher* matcher = instance->matcher;

    int wentBack;

    while (1){
        // if child fails, increment parent, then check
        DBG("Matching until reaching min repetitions %d (%d)...\n", currentRepetition, matcher->minRepetitions[currentRepetition]);
        DBG("latestToken: %d\n", latestToken);
        while (repetitions[currentRepetition] < matcher->minRepetitions[currentRepetition]){
            do {
                DBG("latestToken: %d\n", latestToken);
                DBG("tokenMatchs(matcher, %s, %d)\n", tokens[latestToken], currentRepetition);
                wentBack = 0;
                if (!tokenMatches(matcher, tokens[latestToken], currentRepetition)){
                    DBG("Going back to the previous repetition...\n");

                    latestToken -= repetitions[currentRepetition];
                    repetitions[currentRepetition] = 0;
                    currentRepetition--;

                    wentBack = 1;

                    if (currentRepetition < 0){
                        DBG("No matches possible %d\n", currentRepetition);
                        return NULL;
                    }
                } 
                if (wentBack){
                    DBG("We went back. Reevalutating...\n");
                } else {
                    DBG("We did not go back.\n");
                    repetitions[currentRepetition]++;
                    latestToken++;
                    DBG("This token's repetitions++ %d (%d)\n", currentRepetition, repetitions[currentRepetition]);
                    if (repetitions[currentRepetition] > matcher->maxRepetitions[currentRepetition]){
                        DBG("Too many repetitions. Going back %d (%d)", currentRepetition, matcher->maxRepetitions[currentRepetition]);
                        // too many repetitions. go back
                        latestToken -= repetitions[currentRepetition];
                        repetitions[currentRepetition] = 0;
                        currentRepetition--;

                        wentBack = 1;

                        if (currentRepetition < 0){
                            DBG("No matches possible %d\n", currentRepetition);
                            return NULL;
                        }
                    }
                }
            } while (wentBack);
        }
        DBG("Made it to the minimum number of repetitions %d (%d)...\n", currentRepetition, matcher->minRepetitions[currentRepetition]);

        // we have made it to the minimum. go to the child and restart the process for it
        DBG("Moving to child...\n");
        currentRepetition++;
        DBG("Child index: %d\tNumber of tokens: %d\n", currentRepetition, instance->numberOfTokens);
        if (currentRepetition >= instance->numberOfTokens){
            DBG("Reached a match...\n");
            MatchResult* result = (MatchResult*) malloc(sizeof(MatchResult));
            result->length = latestToken;

            int lastVariable = 0;
            for (int i=0; i<instance->numberOfTokens; i++){
                if (matcher->variableAccesses[i] > lastVariable){
                    lastVariable = matcher->variableAccesses[i];
                }
            }

            result->numberOfVariables = lastVariable+1;
            result->variableBindingLengths = (int*) malloc(sizeof(int) * result->numberOfVariables);
            result->variableBindings = (char***) malloc(sizeof(char**) * result->numberOfVariables);

            for (int i=0; i<result->numberOfVariables; i++){
                result->variableBindingLengths[i] = -1;
            }

            DBG("Binding variables...\n")
            int matchOffset = 0;
            for (int i=0; i<instance->numberOfTokens; i++){
                if (matcher->variableAccesses[i] != -1 && repetitions[i] > 0){
                    DBG("Found variable (%d) that needs binding (index = %d, repetitions = %d, matchOffset = %d)...\n", matcher->variableAccesses[i], i, repetitions[i], matchOffset);
                    result->variableBindingLengths[matcher->variableAccesses[i]] = repetitions[i];
                    result->variableBindings[matcher->variableAccesses[i]] = (char**) malloc(sizeof(char*) * repetitions[i]);
                    // TODO: actually save the string values as bindings
                    DBG("Saving the binding...\n");
                    for (int j=0; j<repetitions[i]; j++){
                        DBG("Added %s to binding.\n", instance->tokens[matchOffset+j]);
                        result->variableBindings[matcher->variableAccesses[i]][j] = tokens[matchOffset+j];
                    }
                }
                matchOffset += repetitions[i];
            }

            return result;
        }
        
    }

    free(repetitions);
    return NULL;
}

// Attempt to match this clause to an array of strings
// If no match is possible, return NULL
MatchResult* Clause_match(Clause* instance, char** tokens, int numberOfTokens, int startOffset){
    DBG("Attempting to match clause to tokens...\n");
    // perform pattern matching using the Matcher
    MatchResult* result;

    // for a match to happen, every one of the matcher's tokens should match with the input
    for (int i=startOffset; i<numberOfTokens; i++){
        result = Clause_matchHelper(instance, tokens+i, numberOfTokens-1);
        if (result != NULL){
            DBG("This clause has a match...\n");
            result->offset = i;
            return result;
        }
    }


    return NULL;
}


