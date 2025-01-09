#ifndef _H_PACKET_STREAM
#define _H_PACKET_STREAM

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#ifdef WII
#include <network.h>
#else
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif
#endif

#ifdef REVISION_177
#define NO_ISAAC
#endif

#ifndef NO_ISAAC
#include "lib/isaac.h"
#endif

#ifndef NO_RSA
#include "rsa.h" // Include rsa-tiny header
#endif

#if !defined(WIN32) && !defined(WII)
#define HAVE_SIGNALS
#endif

#define USERNAME_LENGTH 20
#define PASSWORD_LENGTH 20

#define PACKET_BUFFER_LENGTH 5000

typedef struct PacketStream PacketStream;

#include "mudclient.h"
#include "utility.h"

#ifdef REVISION_177
void init_packet_stream_global(void);
#endif

// PacketStream structure
struct PacketStream {
    int socket;                     // Network socket
    int closed;                     // Flag for stream closure
    int delay;                      // Delay counter
    int length;                     // Length of current packet
    int max_read_tries;             // Max number of read retries
    int8_t packet_data[PACKET_BUFFER_LENGTH]; // Packet buffer
    int packet_end;                 // End of the current packet
    int packet_max_length;          // Max packet length
    int packet_start;               // Start of the current packet
    int read_tries;                 // Number of read retries
    int socket_exception;           // Flag for socket exception
    char *socket_exception_message; // Exception message
    int8_t available_buffer[PACKET_BUFFER_LENGTH]; // Available buffer
    int available_length;           // Length of available data
    int available_offset;           // Offset in available buffer

#ifndef NO_RSA
    struct rsa rsa_pub_key;  // RSA public key structure
#endif

#ifndef NO_ISAAC
    struct isaac isaac_in;          // ISAAC input context
    struct isaac isaac_out;         // ISAAC output context
    int isaac_ready;                // Flag indicating ISAAC readiness
#endif
};

// Function prototypes
void packet_stream_new(PacketStream *packet_stream, mudclient *mud);
int packet_stream_available_bytes(PacketStream *packet_stream, int length);
int packet_stream_read_bytes(PacketStream *packet_stream, int length,
                             int8_t *buffer);
int packet_stream_write_bytes(PacketStream *packet_stream, int8_t *buffer,
                              int offset, int length);
int packet_stream_read_byte(PacketStream *packet_stream);
int packet_stream_has_packet(PacketStream *packet_stream);
int packet_stream_read_packet(PacketStream *packet_stream, int8_t *buffer);
void packet_stream_new_packet(PacketStream *packet_stream, ClientOpcode opcode);
int packet_stream_write_packet(PacketStream *packet_stream, int i);
void packet_stream_send_packet(PacketStream *packet_stream);
void packet_stream_put_bytes(PacketStream *packet_stream, void *src, int offset,
                             int length);
void packet_stream_put_byte(PacketStream *packet_stream, int i);
void packet_stream_put_short(PacketStream *packet_stream, int i);
void packet_stream_put_int(PacketStream *packet_stream, int i);
void packet_stream_put_long(PacketStream *packet_stream, int64_t i);
void packet_stream_put_string(PacketStream *packet_stream, char *s);

#ifdef REVISION_177
void packet_stream_put_password(PacketStream *packet_stream, int session_id,
                                char *password);
#else
void packet_stream_put_login_block(PacketStream *packet_stream,
                                   const char *username, const char *password,
                                   uint32_t *isaac_keys, uint32_t uuid);
#endif

int packet_stream_get_byte(PacketStream *packet_stream);
int packet_stream_get_short(PacketStream *packet_stream);
int packet_stream_get_int(PacketStream *packet_stream);
int64_t packet_stream_get_long(PacketStream *packet_stream);
int packet_stream_flush_packet(PacketStream *packet_stream);
void packet_stream_close(PacketStream *packet_stream);

#endif
