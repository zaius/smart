/**
 * Internet Zero - Unix Gateway
 *
 * \file unix/slip.h
 * \author David Kelso - david@kelso.id.au
 * \brief SLIP encoder/decoder header
 */

// SLIP special characters
/// Character used to signal the start or end of a frame
#define SLIP_END            0xC0
/// Character used to escape the use of special characters in the data
#define SLIP_ESC            0xDB
/// Character used to replace a SLIP_END character in the data
#define SLIP_ESC_END        0xDC
/// Character used to replace a SLIP_ESC character in the data
#define SLIP_ESC_ESC        0xDD

// Prototypes

/**
 * slip_encode -
 * The tunnel device always gives a full packet so we can just run the
 * full packet through a complete encoding.
 * \param dest an array pointer that will be set to point to the data
 * \param source an array of data to be SLIP encoded
 * \param length the length of the source array
 * \return the size of the destination array pointed to by dest
 */
size_t slip_encode(uint8_t * dest, size_t dest_size, uint8_t * source, size_t source_size);

/**
 * slip_add_data -
 * Since a read on the serial device can never promise a full packet,
 * this function copies all the data that may be read over a period of
 * time and inserts it into a contiguous buffer in order to look for
 * an entire packet. it returns the number of full packets that are
 * available as a result of calling the slip_add_data function and the
 * youngest decoded packets can be obtained by calling slip_retrieve
 * \param source an array of data to add to the buffer
 * \param length the length of the source array
 * \returns the amount of ready packets
 */
int slip_add_data(uint8_t * source, size_t length);

/**
 * slip_retrieve - 
 * Retrieves a packet worth of decoded data that has previously been added
 * with slip_add_data
 *
 * \param dest an array pointer that will be set to point to the data
 * \returns The size of the array of data
 */
size_t slip_retrieve(uint8_t * dest, size_t dest_size);

