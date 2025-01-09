#include "packet-stream.h"
#include <kos.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "rsa.h" // Include rsatiny header


#if 0
char *SPOOKY_THREAT =
    "All RuneScape code and data, including this message, are copyright 2003 "
    "Jagex Ltd. Unauthorised reproduction in any form is strictly prohibited.  "
    "The RuneScape network protocol is copyright 2003 Jagex Ltd and is "
    "protected by international copyright laws. The RuneScape network protocol "
    "also incorporates a copy protection mechanism to prevent unauthorised "
    "access or use of our servers. Attempting to break, bypass or duplicate "
    "this mechanism is an infringement of the Digital Millienium Copyright Act "
    "and may lead to prosecution. Decompiling, or reverse-engineering the "
    "RuneScape code in any way is strictly prohibited. RuneScape and Jagex are "
    "registered trademarks of Jagex Ltd. You should not be reading this "
    "message, you have been warned...";

int THREAT_LENGTH = 0;

int OPCODE_ENCRYPTION[] = {
    124, 345, 953, 124, 634, 636, 661, 127, 177, 295, 559, 384, 321, 679, 871,
    592, 679, 347, 926, 585, 681, 195, 785, 679, 818, 115, 226, 799, 925, 852,
    194, 966, 32,  3,   4,   5,   6,   7,   8,   9,   40,  1,   2,   3,   4,
    5,   6,   7,   8,   9,   50,  444, 52,  3,   4,   5,   6,   7,   8,   9,
    60,  1,   2,   3,   4,   5,   6,   7,   8,   9,   70,  1,   2,   3,   4,
    5,   6,   7,   8,   9,   80,  1,   2,   3,   4,   5,   6,   7,   8,   9,
    90,  1,   2,   3,   4,   5,   6,   7,   8,   9,   100, 1,   2,   3,   4,
    5,   6,   7,   8,   9,   110, 1,   2,   3,   4,   5,   6,   7,   8,   9,
    120, 1,   2,   3,   4,   5,   6,   7,   8,   9,   130, 1,   2,   3,   4,
    5,   6,   7,   8,   9,   140, 1,   2,   3,   4,   5,   6,   7,   8,   9,
    150, 1,   2,   3,   4,   5,   6,   7,   8,   9,   160, 1,   2,   3,   4,
    5,   6,   7,   8,   9,   170, 1,   2,   3,   4,   5,   6,   7,   8,   9,
    180, 1,   2,   3,   4,   5,   6,   7,   8,   9,   694, 235, 846, 834, 300,
    200, 298, 278, 247, 286, 346, 144, 23,  913, 812, 765, 432, 176, 935, 452,
    542, 45,  346, 65,  637, 62,  354, 123, 34,  912, 812, 834, 698, 324, 872,
    912, 438, 765, 344, 731, 625, 783, 176, 658, 128, 854, 489, 85,  6,   865,
    43,  573, 132, 527, 235, 434, 658, 912, 825, 298, 753, 282, 652, 439, 629,
    945};

int get_client_opcode_friend(int opcode) {
    switch (opcode) {
    case CLIENT_LOGIN:
        return CLIENT_LOGIN_FRIEND;
    case CLIENT_RECONNECT:
        return CLIENT_RECONNECT_FRIEND;
    }

    return -1;
}

void init_packet_stream_global(void) { THREAT_LENGTH = strlen(SPOOKY_THREAT); }
#endif
void packet_stream_new(PacketStream *packet_stream, mudclient *mud) {
    memset(packet_stream, 0, sizeof(PacketStream));
    packet_stream->max_read_tries = 1000;

#ifndef NO_RSA
    // Initialize the RSA public key
    rsa_init(&packet_stream->rsa_pub_key, mud->rsa_exponent, mud->rsa_modulus);

    // Validate the RSA public key
    if (strlen(mud->rsa_modulus) == 0 || strlen(mud->rsa_exponent) == 0) {
        dbglog(DBG_ERROR, "Invalid RSA key: modulus or exponent missing.\n");
        exit(1);
    }
#endif

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(mud->port);
    struct hostent *host = gethostbyname(mud->server);

    if (!host) {
        dbglog(DBG_ERROR, "Failed to resolve server: %s\n", mud->server);
        packet_stream->closed = 1;
        return;
    }

    memcpy(&server_addr.sin_addr, host->h_addr, sizeof(struct in_addr));

    packet_stream->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (packet_stream->socket < 0) {
        dbglog(DBG_ERROR, "Socket creation failed: %s\n", strerror(errno));
        packet_stream_close(packet_stream);
        return;
    }

    // Retry logic
    int retries = 5;
    int delay_ms = 1000;
    int attempt = 0;
    int connected = 0;

    while (attempt < retries) {
        if (connect(packet_stream->socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            connected = 1;
            break;
        }

        dbglog(DBG_WARNING, "Connection attempt %d failed: %s\n", attempt + 1, strerror(errno));
        attempt++;
        thd_sleep(delay_ms);
    }

    if (!connected) {
        dbglog(DBG_ERROR, "All connection attempts failed after %d retries.\n", retries);
        packet_stream_close(packet_stream);
        return;
    }

    dbglog(DBG_INFO, "Connected to RuneScape server: %s\n", mud->server);

    packet_stream->closed = 0;
    packet_stream->packet_end = 3;
    packet_stream->packet_max_length = PACKET_BUFFER_LENGTH;
}

int packet_stream_available_bytes(PacketStream *packet_stream, int length) {
    if (packet_stream->available_length >= length) {
        return 1;
    }

    int bytes = recv(packet_stream->socket,
                     packet_stream->available_buffer + packet_stream->available_offset + packet_stream->available_length,
                     length - packet_stream->available_length, 0);

    if (bytes <= 0) {
        return 0; // Failure
    }

    packet_stream->available_length += bytes;
    return (packet_stream->available_length >= length);
}

int packet_stream_read_bytes(PacketStream *packet_stream, int length, int8_t *buffer) {
    if (packet_stream->closed) {
        return -1;
    }

    int remaining = length;

    while (remaining > 0) {
        int to_copy = packet_stream->available_length < remaining
                          ? packet_stream->available_length
                          : remaining;

        if (to_copy > 0) {
            memcpy(buffer, packet_stream->available_buffer + packet_stream->available_offset, to_copy);
            buffer += to_copy;
            remaining -= to_copy;
            packet_stream->available_length -= to_copy;

            if (packet_stream->available_length == 0) {
                packet_stream->available_offset = 0;
            } else {
                packet_stream->available_offset += to_copy;
            }
        } else {
            int bytes = recv(packet_stream->socket, packet_stream->available_buffer, PACKET_BUFFER_LENGTH, 0);
            if (bytes <= 0) {
                dbglog(DBG_ERROR, "Socket recv failed: %s\n", strerror(errno));
                packet_stream->closed = 1;
                return -1;
            }

            packet_stream->available_length = bytes;
            packet_stream->available_offset = 0;
        }
    }

    return 0;
}

int packet_stream_write_bytes(PacketStream *packet_stream, int8_t *buffer, int offset, int length) {
    if (!packet_stream->closed) {
        return send(packet_stream->socket, buffer + offset, length, 0);
    }
    return -1;
}

int packet_stream_read_byte(PacketStream *packet_stream) {
    if (packet_stream->closed) {
        return -1;
    }

    int8_t byte;

    if (packet_stream_read_bytes(packet_stream, 1, &byte) > -1) {
        return (uint8_t)byte & 0xff;
    }

    return -1;
}

int packet_stream_has_packet(PacketStream *packet_stream) {
    return packet_stream->packet_start > 0;
}

int packet_stream_read_packet(PacketStream *packet_stream, int8_t *buffer) {
    packet_stream->read_tries++;

    if (packet_stream->max_read_tries > 0 &&
        packet_stream->read_tries > packet_stream->max_read_tries) {
        packet_stream->socket_exception = 1;
        packet_stream->socket_exception_message = "time-out";
        packet_stream->max_read_tries += packet_stream->max_read_tries;

        return 0;
    }

    if (packet_stream->length == 0 &&
        packet_stream_available_bytes(packet_stream, 2)) {
        packet_stream->length = packet_stream_read_byte(packet_stream);

        if (packet_stream->length >= 160) {
            packet_stream->length = (packet_stream->length - 160) * 256 +
                                    (packet_stream_read_byte(packet_stream));
        }
    }

    if (packet_stream->length > 0 &&
        packet_stream_available_bytes(packet_stream, packet_stream->length)) {
        if (packet_stream->length >= 160) {
            if (packet_stream_read_bytes(packet_stream, packet_stream->length,
                                         buffer) < 0) {
                return 0;
            }
        } else {
            buffer[packet_stream->length - 1] =
                packet_stream_read_byte(packet_stream);

            if (packet_stream->length > 1) {
                if (packet_stream_read_bytes(
                        packet_stream, packet_stream->length - 1, buffer) < 0) {
                    return 0;
                }
            }
        }

        int i = packet_stream->length;

        packet_stream->length = 0;
        packet_stream->read_tries = 0;

        return i;
    }

    return 0;
}

void packet_stream_new_packet(PacketStream *packet_stream,
                              ClientOpcode opcode) {
#if 0
    packet_stream->opcode_friend = get_client_opcode_friend(opcode);
#endif

    if (packet_stream->packet_start >
        ((packet_stream->packet_max_length * 4) / 5)) {
        if (packet_stream_write_packet(packet_stream, 0) < 0) {
            packet_stream->socket_exception = 1;
            packet_stream->socket_exception_message = "failed to write packet";
        }
    }

#ifndef NO_ISAAC
    if (packet_stream->isaac_ready) {
        opcode = opcode + isaac_next(&packet_stream->isaac_out);
    }
#endif

    packet_stream->packet_data[packet_stream->packet_start + 2] = opcode & 0xff;
    packet_stream->packet_data[packet_stream->packet_start + 3] = 0;
    packet_stream->packet_end = packet_stream->packet_start + 3;
}


/*int packet_stream_decode_opcode(PacketStream *packet_stream, int opcode) {
    int index = (opcode - packet_stream->decode_key) & 255;
    int decoded_opcode = OPCODE_ENCRYPTION[index];

    packet_stream->decode_threat_index =
        (packet_stream->decode_threat_index + decoded_opcode) % THREAT_LENGTH;

    char threat_character = SPOOKY_THREAT[packet_stream->decode_threat_index];

    packet_stream->decode_key = (packet_stream->decode_key * 3 +
                                 (int)threat_character + decoded_opcode) &
                                0xffff;

    return decoded_opcode;
}*/

int packet_stream_write_packet(PacketStream *packet_stream, int i) {
    if (packet_stream->socket_exception) {
        packet_stream->packet_start = 0;
        packet_stream->packet_end = 3;
        packet_stream->socket_exception = 0;

        mud_error("socket exception: %s\n",
                  packet_stream->socket_exception_message);

        return -1;
    }

    packet_stream->delay++;

    if (packet_stream->delay < i) {
        return 0;
    }

    if (packet_stream->packet_start > 0) {
        packet_stream->delay = 0;

        if (packet_stream_write_bytes(packet_stream, packet_stream->packet_data,
                                      0, packet_stream->packet_start) < 0) {
            return -1;
        }
    }

    packet_stream->packet_start = 0;
    packet_stream->packet_end = 3;

    return 0;
}

void packet_stream_send_packet(PacketStream *packet_stream) {
    int length = packet_stream->packet_end - packet_stream->packet_start;

    if (length > PACKET_BUFFER_LENGTH) {
        dbglog(DBG_ERROR, "Packet length exceeds buffer size\n");
        return;
    }

    if (send(packet_stream->socket, packet_stream->packet_data + packet_stream->packet_start, length, 0) <= 0) {
        dbglog(DBG_ERROR, "Failed to send packet: %s\n", strerror(errno));
        packet_stream->closed = 1;
        return;
    }

    packet_stream->packet_start = 0;
    packet_stream->packet_end = 3; // Reset to default start
}

int packet_stream_flush_packet(PacketStream *packet_stream) {
    packet_stream_send_packet(packet_stream);
    return packet_stream_write_packet(packet_stream, 0);
}

void packet_stream_put_bytes(PacketStream *packet_stream, void *src, int offset,
                             int length) {
    uint8_t *p = src;

    memcpy(packet_stream->packet_data + packet_stream->packet_end, p + offset,
           length);

    packet_stream->packet_end += length;
}

void packet_stream_put_byte(PacketStream *packet_stream, int i) {
    packet_stream->packet_data[packet_stream->packet_end++] = i & 0xff;
}

void packet_stream_put_short(PacketStream *packet_stream, int i) {
    packet_stream->packet_data[packet_stream->packet_end++] = (i >> 8) & 0xff;
    packet_stream->packet_data[packet_stream->packet_end++] = i & 0xff;
}

void packet_stream_put_int(PacketStream *packet_stream, int i) {
    packet_stream->packet_data[packet_stream->packet_end++] = (i >> 24) & 0xff;
    packet_stream->packet_data[packet_stream->packet_end++] = (i >> 16) & 0xff;
    packet_stream->packet_data[packet_stream->packet_end++] = (i >> 8) & 0xff;
    packet_stream->packet_data[packet_stream->packet_end++] = i & 0xff;
}

void packet_stream_put_long(PacketStream *packet_stream, int64_t i) {
    packet_stream_put_int(packet_stream, (int32_t)(i >> 32));
    packet_stream_put_int(packet_stream, (int32_t)i);
}

void packet_stream_put_string(PacketStream *packet_stream, char *s) {
    packet_stream_put_bytes(packet_stream, (int8_t *)s, 0, strlen(s));
}

#ifndef NO_RSA
static void packet_stream_put_rsa(PacketStream *packet_stream, const void *input, size_t input_len) {
    unsigned char encrypted[512]; // Adjust size for your key
    size_t encrypted_len;

    // Encrypt the input using rsa_tiny
    encrypted_len = rsa_crypt(
        &packet_stream->rsa_pub_key, input, input_len, encrypted, sizeof(encrypted)
    );

    if (encrypted_len <= 0) {
        dbglog(DBG_ERROR, "RSA encryption failed\n");
        return;
    }

    // Add the encrypted block to the packet stream
    packet_stream_put_byte(packet_stream, (int)encrypted_len); // Store length
    memcpy(&packet_stream->packet_data[packet_stream->packet_end], encrypted, encrypted_len);
    packet_stream->packet_end += encrypted_len;
}
#endif

#ifndef REVISION_177
void packet_stream_put_login_block(PacketStream *packet_stream, const char *username, const char *password,
                                   uint32_t *isaac_keys, uint32_t uuid) {
    uint8_t input_block[128] = {0};  // Adjusted size for RuneScape Classic
    uint8_t *p = input_block;

#ifndef NO_ISAAC
    for (int i = 0; i < 4; i++) {
        packet_stream->isaac_in.randrsl[i] = isaac_keys[i];
        packet_stream->isaac_out.randrsl[i] = isaac_keys[i];
        write_unsigned_int(p, 0, isaac_keys[i]);
        p += 4;
    }
#endif

    write_unsigned_int(p, 0, uuid);
    p += 4;

    size_t username_len = strlen(username);
    size_t password_len = strlen(password);

    if (username_len + password_len + 2 > sizeof(input_block)) {
        dbglog(DBG_ERROR, "Username and password too long for input block\n");
        return;
    }

    memcpy(p, username, username_len);
    p += username_len;
    *(p++) = '\n';

    memcpy(p, password, password_len);
    p += password_len;
    *(p++) = '\n';

#ifndef NO_RSA
    packet_stream_put_rsa(packet_stream, input_block, p - input_block);
#else
    packet_stream_put_bytes(packet_stream, input_block, 0, p - input_block);
#endif
}
#endif

#ifdef REVISION_177
void packet_stream_put_password(PacketStream *packet_stream, int session_id,
                                char *password) {
    int8_t encoded[15] = {0};

    int password_length = strlen(password);
    int password_index = 0;

    while (password_index < password_length) {
        encoded[0] = (int8_t)(1 + ((float)rand() / (float)RAND_MAX) * 127);
        encoded[1] = (int8_t)(((float)rand() / (float)RAND_MAX) * 256);
        encoded[2] = (int8_t)(((float)rand() / (float)RAND_MAX) * 256);
        encoded[3] = (int8_t)(((float)rand() / (float)RAND_MAX) * 256);

        write_unsigned_int(encoded, 4, session_id);

        for (int i = 0; i < 7; i++) {
            if (password_index + i >= password_length) {
                encoded[8 + i] = 32;
            } else {
                encoded[8 + i] = (int8_t)password[password_index + i];
            }
        }

        password_index += 7;
#ifndef NO_RSA
        packet_stream_put_rsa(packet_stream, encoded, sizeof(encoded));
#else
        packet_stream_put_byte(packet_stream, sizeof(encoded));
        packet_stream_put_bytes(packet_stream, encoded, 0, sizeof(encoded));
#endif
    }
}
#endif

int packet_stream_get_byte(PacketStream *packet_stream) {
    return packet_stream_read_byte(packet_stream);
}

int packet_stream_get_short(PacketStream *packet_stream) {
    int i = packet_stream_get_byte(packet_stream);
    int j = packet_stream_get_byte(packet_stream);

    return i * 256 + j;
}

int packet_stream_get_int(PacketStream *packet_stream) {
    int i = packet_stream_get_short(packet_stream);
    int j = packet_stream_get_short(packet_stream);

    return i * 65536 + j;
}

int64_t packet_stream_get_long(PacketStream *packet_stream) {
    int64_t result = 0;
    for (int i = 0; i < 4; i++) {
        result = (result << 16) | packet_stream_get_short(packet_stream);
    }
    return result;
}

void packet_stream_close(PacketStream *packet_stream) {
    if (packet_stream->socket > -1) {
        close(packet_stream->socket);
        packet_stream->socket = -1;
    }
    packet_stream->closed = 1;
}
