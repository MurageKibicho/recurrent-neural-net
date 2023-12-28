#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

#include "std_conf.h"
#include "lstm.h"
#include "layers.h"
#include "utilities.h"


#define ITERATIONS  1000
#define NO_EPOCHS   0 

lstm_model_t *model = NULL, *layer1 = NULL, *layer2 = NULL;
lstm_model_t **model_layers;
lstm_model_parameters_t params;
set_t set;

//gcc -o3 -Ofast -msse3 src/lstm.c src/layers.c src/utilities.c src/main.c -lm -o m.o && ./m.o


int main()
{
	srand( time ( NULL ) );
	char *fileName = "bert.txt";
	int c;
	unsigned int fileSize = 0, sz = 0;
	int *X_train, *Y_train;
	FILE * fp;
	memset(&params, 0, sizeof(params));

	params.iterations = ITERATIONS;
	params.epochs = NO_EPOCHS;
	params.loss_moving_avg = LOSS_MOVING_AVG;
	params.learning_rate = STD_LEARNING_RATE;
	params.momentum = STD_MOMENTUM;
	params.lambda = STD_LAMBDA;
	params.softmax_temp = SOFTMAX_TEMP;
	params.mini_batch_size = MINI_BATCH_SIZE;
	params.gradient_clip_limit = GRADIENT_CLIP_LIMIT;
	params.learning_rate_decrease = STD_LEARNING_RATE_DECREASE;
	params.stateful = STATEFUL;
	params.beta1 = 0.9;
	params.beta2 = 0.999;
	params.gradient_fit = GRADIENTS_FIT;
	params.gradient_clip = GRADIENTS_CLIP;
	params.decrease_lr = DECREASE_LR;
	params.model_regularize = MODEL_REGULARIZE;
	params.layers = LAYERS;
	params.neurons = NEURONS;
	params.optimizer = OPTIMIZE_ADAM;
	// Interaction configuration with the training of the network
	params.print_progress = PRINT_PROGRESS;
	params.print_progress_iterations = PRINT_EVERY_X_ITERATIONS;
	params.print_progress_sample_output = PRINT_SAMPLE_OUTPUT;
	params.print_progress_to_file = PRINT_SAMPLE_OUTPUT_TO_FILE;
	params.print_progress_number_of_chars = NUMBER_OF_CHARS_TO_DISPLAY_DURING_TRAINING;
	params.print_sample_output_to_file_arg = PRINT_SAMPLE_OUTPUT_TO_FILE_ARG;
	params.print_sample_output_to_file_name = PRINT_SAMPLE_OUTPUT_TO_FILE_NAME;
	params.store_progress_every_x_iterations = STORE_PROGRESS_EVERY_X_ITERATIONS;
	params.store_progress_file_name = PROGRESS_FILE_NAME;
	params.store_network_name_raw = STD_LOADABLE_NET_NAME;
	params.store_network_name_json = STD_JSON_NET_NAME;
	params.store_char_indx_map_name = JSON_KEY_NAME_SET;

	/*Prep lstm*/
	InitializeVocabulary(&set);
	fp = fopen(fileName, "r");assert(fp != NULL);
	while((c = fgetc(fp)) != EOF)
	{
		AddCharacterToVocabulary(&set, (char)c);
		fileSize+=1;
	}
	fclose(fp);
	
	X_train = calloc(fileSize+1, sizeof(int));assert(X_train != NULL);
	X_train[fileSize] = X_train[0];
	Y_train = &X_train[1];
	
	fp = fopen(fileName, "r");
	while((c = fgetc(fp)) != EOF)
	{
		X_train[sz++] = ConvertCharacterToIndex(&set,c);
	}
	fclose(fp);
	
	/* Allocating space for a new model */
	model_layers = calloc(params.layers, sizeof(lstm_model_t*));
	assert(model_layers != NULL);
   	for(unsigned int p = 0; p < params.layers; p++)
   	{
   		//All layers have the same training parameters
   		int X;
   		int N = params.neurons;
   		int Y;
   		if(params.layers == 1)
   		{
   			X = CountVocabularySize(&set);
   			Y = CountVocabularySize(&set);
   		}
   		else
   		{
   			if(p == 0)
   			{
   				Y = CountVocabularySize(&set);
   				X = params.neurons;
   			}
   			else if(p == params.layers - 1)
   			{
   				Y = params.neurons;
   				X = CountVocabularySize(&set);
   			}
   			else
   			{
   				Y = params.neurons;
   				X = params.neurons;
   			}
		}
		lstm_init_model(X, N, Y, &model_layers[p], 0, &params); 
	}
	double loss = 0.0f;
	assert(params.layers > 0);
	lstm_train(model_layers,&params,&set,fileSize,X_train,Y_train,params.layers,&loss);
	printf("Loss after training: %lf\n", loss);
  	
  	free(model_layers);
  	free(X_train);
	return 0;
}
