// SLIP special characters
#define SLIP_END            0300
#define SLIP_ESC            0333
#define SLIP_ESC_END        0334
#define SLIP_ESC_ESC        0335

// Prototypes
void slip_send(char * data, int length); 
int slip_poll(char * data, int maxlength); 

