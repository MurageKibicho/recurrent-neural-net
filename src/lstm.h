
#ifndef LSTM_H
#define LSTM_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "set.h"
#include "utilities.h"

#include "layers.h"
#include "assert.h"

#define OPTIMIZE_ADAM                         0
#define OPTIMIZE_GRADIENT_DESCENT             1

#define LSTM_MAX_LAYERS                       10

#define BINARY_FILE_VERSION                   1

typedef struct lstm_model_parameters_t {
  // For progress monitoring
  double loss_moving_avg;
  // For gradient descent
  double learning_rate;
  double momentum;
  double lambda;
  double softmax_temp;
  double beta1;
  double beta2;
  int gradient_clip;
  int gradient_fit;
  int optimizer;
  int model_regularize;
  int stateful;
  int decrease_lr;
  double learning_rate_decrease;

  // How many layers
  unsigned int layers;
  // How many neurons this layer has
  unsigned int neurons;

  // Output configuration for interactivity
  long print_progress_iterations;
  int  print_progress_sample_output;
  int  print_progress;
  int  print_progress_to_file;
  int  print_progress_number_of_chars;
  char *print_sample_output_to_file_name;
  char *print_sample_output_to_file_arg;
  int  store_progress_every_x_iterations;
  char *store_progress_file_name;
  int  store_network_every;
  char *store_network_name_raw;
  char *store_network_name_json;
  char *store_char_indx_map_name;

  // General parameters
  unsigned int mini_batch_size;
  double gradient_clip_limit;
  unsigned long iterations;
  unsigned long epochs;
} lstm_model_parameters_t;

typedef struct lstm_model_t
{
  unsigned int X; /**< Number of input nodes */
  unsigned int N; /**< Number of neurons */
  unsigned int Y; /**< Number of output nodes */
  unsigned int S; /**< lstm_model_t.X + lstm_model_t.N */

  // Parameters
  lstm_model_parameters_t * params;

  // The model
  double* Wf;
  double* Wi;
  double* Wc;
  double* Wo;
  double* Wy;
  double* bf;
  double* bi;
  double* bc;
  double* bo;
  double* by;

  // cache
  double* dldh;
  double* dldho;
  double* dldhf;
  double* dldhi;
  double* dldhc;
  double* dldc;

  double* dldXi;
  double* dldXo;
  double* dldXf;
  double* dldXc;

  // Gradient descent momentum
  double* Wfm;
  double* Wim;
  double* Wcm;
  double* Wom;
  double* Wym;
  double* bfm;
  double* bim;
  double* bcm;
  double* bom;
  double* bym;

} lstm_model_t;

typedef struct lstm_values_cache_t {
  double* probs;
  double* probs_before_sigma;
  double* c;
  double* h;
  double* c_old;
  double* h_old;
  double* X;
  double* hf;
  double* hi;
  double* ho;
  double* hc;
  double* tanh_c_cache;
} lstm_values_cache_t;

typedef struct lstm_values_state_t {
  double* c;
  double* h;
} lstm_values_state_t;

typedef struct lstm_values_next_cache_t {
  double* dldh_next;
  double* dldc_next;
  double* dldY_pass;
} lstm_values_next_cache_t;

/**
* Initialize a new model
* @param X number of inputs
* @param N number of nodes
* @param Y number of outputs
* @param model_to_be_set the model object that is to be set
* @param zero if set to 0, the model will have zeros as weights,\
otherwise random initialization
* @param param model parameters
* @return 0 on success, negative values on errors
*/ 
int InitializeLSTMModel(int X, int N, int Y, 
  lstm_model_t** model_to_be_set, int zeros,
  lstm_model_parameters_t *params);
/**
* Set all weights in a model to zero
* @param model model to be set to zero
*/ 
void lstm_zero_the_model(lstm_model_t *model);
void lstm_zero_d_next(lstm_values_next_cache_t * d_next,
  int outputs, int neurons);
void lstm_cache_container_set_start(lstm_values_cache_t *cache, int neurons);

/**
* Free a model
* @param lstm model to be freed
*/ 
void DestroyLSTMModel(lstm_model_t *lstm);
/**
* Compute the output of a network
* @param model model to be used, must been initialized with \ref InitializeLSTMModel
* \see InitializeLSTMModel
*/ 
void lstm_forward_propagate(lstm_model_t *model, double *input, 
  lstm_values_cache_t *cache_in, lstm_values_cache_t *cache_out, int softmax);
void lstm_backward_propagate(lstm_model_t*, double*, int, lstm_values_next_cache_t*, lstm_values_cache_t*, lstm_model_t*, lstm_values_next_cache_t*);

void lstm_values_state_init(lstm_values_state_t** d_next_to_set, int N);
void lstm_values_next_state_free(lstm_values_state_t* d_next);

lstm_values_cache_t*  lstm_cache_container_init(int X, int N, int Y);
void lstm_cache_container_free(lstm_values_cache_t*);
void lstm_values_next_cache_init(lstm_values_next_cache_t**, int N, int X);
void lstm_values_next_cache_free(lstm_values_next_cache_t*);
void sum_gradients(lstm_model_t*, lstm_model_t*);



/**
* This is the entry point to the realm of black magic.
* Trains the network.
* \see InitializeLSTMModel
* @param model The model that is to be used, must have been \
initialzed with \ref InitializeLSTMModel.
* @param params Various parameters determining the training process
* @param set The feature-to-index mapping. 
* @param training_points length of the training data array \p X
* @param X input observations
* @param Y output observations (typically &X[1], so that X[0] -> Y[0]: 'h' -> 'e', \
if X[...] = 'hello' => Y[...] = 'ello ').
* @param layers number of layers in the network, the number of models \p model \
is pointing to. Internally if layers is L, then input is given to model[L-1] and \
output collected at model[0].
* @param loss the value of the loss function, put under a smoothing \
moving average filter, after the training has been completed.
*/ 
void lstm_train(lstm_model_t** model, lstm_model_parameters_t*params,
  set_t* set, unsigned int training_points, int *X, int *Y, unsigned int layers,
  double *loss);
/**
* If you are training on textual data, this function can be used 
* to sample and output from the network directly to stdout. 
* \see InitializeLSTMModel
* \see lstm_train
* @param model The model that is to be used, must have been \
initialzed with \ref InitializeLSTMModel.
* @param set The feature-to-index mapping. 
* @param first input seed, the rest will "follow" to stdout.
* @param samples_to_display How many observations to write to stdout
* @param layers how many layers this network has
*/ 
void lstm_output_string_layers(lstm_model_t ** model_layers, set_t* set,
  int first, int samples_to_display, int layers);



#endif
