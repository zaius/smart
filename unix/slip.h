// SLIP special characters
#define SLIP_END            0300
#define SLIP_ESC            0333
#define SLIP_ESC_END        0334
#define SLIP_ESC_ESC        0335

// Prototypes
size_t slip_encode(char ** dest, char * source, size_t length); 
int slip_poll(char * data, int maxlength); 

