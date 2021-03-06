/*  set the optimisation type*/

#define NO  1  // Normal Operation
#define RO  0 // Resource Optimized

#define BILINEAR 1
#define NN 0

//Number of rows of input image to be stored
#define NUM_STORE_ROWS 100

//Number of rows of input image after which output image processing must start
#define START_PROC 50

//transform type 0-NN 1-BILINEAR
#define INTERPOLATION 1

//transform type 0-AFFINE 1-PERSPECTIVE
#define TRANSFORM_TYPE 0
#define XF_USE_URAM false
