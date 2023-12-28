#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>
#define SET_MAX_CHARS	1000

//https://stackoverflow.com/questions/17764661/multiple-definition-of-linker-error
//Use static to prevent multiple definitions error
typedef struct set_t
{
	char values[SET_MAX_CHARS];
	int free[SET_MAX_CHARS];
} set_t;


static void InitializeVocabulary(set_t * set) 
{
	for(int i = 0; i < SET_MAX_CHARS; i++)
	{
		set->values[i] = '\0';
		set->free[i] = 1;
	}
}

static int AddCharacterToVocabulary(set_t * set, char c)
{
	for(int i = 0; i < SET_MAX_CHARS; i++)
	{
		if((char)set->values[i] == c && set->free[i] == 0)
		{
			return i;		
		}
		if(set->free[i])
		{
			set->values[i] = c;
			set->free[i] = 0;
			return 0;
		}
	}
	return -1;
}

static char ConvertIndexToCharacter(set_t* set, int index)
{
	if(index >= SET_MAX_CHARS)
	{
		return '\0';
	}
	return (char) set->values[index];
}

static int ConvertCharacterToIndex(set_t* set, char c) 
{
	for(int i = 0; i < SET_MAX_CHARS; i++)
	{
		if(set->values[i] == (int) c && set->free[i] == 0)
		{
			return i;
		}
	}
	return -1;
}

static int ChooseBestProbabilityFromSet(set_t* set, double* probs)
{
	double sum = 0.0f;
	double randomValue = 0.0f;
	randomValue = ((double) rand())/RAND_MAX;
	for(int i = 0; i < SET_MAX_CHARS; i++)
	{
		sum += probs[i];
		if(sum - randomValue > 0)
		{
			return set->values[i];	
		}
	}
//PrintVocabulary(set,probs);
  return 0;
}

static int CountVocabularySize(set_t* set) 
{
	int i = 0;
	while(set->free[i] == 0)
	{
		i++;
	}
	if(i < SET_MAX_CHARS)
	{
		return i;
	}
	return 0;
}

static void PrintVocabulary(set_t* set, double* probs)
{
	int i = 0;
	while(set->values[i] != 0 && i < SET_MAX_CHARS )
	{
		if(set->values[i] == '\n')
		{
			printf("[ newline:  %lf ]\n", probs[i]);
		}
		else
		{
			printf("[ %c:     %lf ]\n", set->values[i], probs[i]);
		}
		i+=1;
	}
}
