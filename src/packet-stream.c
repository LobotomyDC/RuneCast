#include "packet-stream.h"

#ifdef HAVE_SIGNALS
#include <signal.h>
#endif

#ifdef _WIN32
#define close closesocket
#define ioctl ioctlsocket

static int winsock_init = 0;
#endif

#ifdef DREAMCAST
  #include <fcntl.h>
  #include <unistd.h>
  #include <kos/net.h>
#endif

#ifdef WII
#define socket(x, y, z) net_socket(x, y, z)
#define gethostbyname net_gethostbyname
#define setsockopt net_setsockopt
#define connect net_connect
#define close net_close
#define write net_write
#define recv net_recv
#endif

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

#ifdef HAVE_SIGNALS
void on_signal_do_nothing(int dummy);

void on_signal_do_nothing(int dummy) { (void)dummy; }
#endif

void packet_stream_new(PacketStream *packet_stream, mudclient *mud) {
#ifdef WIN32
    if (!winsock_init) {
        WSADATA wsa_data = {0};
        int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);

        if (ret < 0) {
            mud_error("WSAStartup() error: %d\n", WSAGetLastError());
            exit(1);
        }
        winsock_init = 1;
    }
#endif

    memset(packet_stream, 0, sizeof(PacketStream));

    packet_stream->max_read_tries = 1000;

#ifdef REVISION_177
    /*packet_stream->decode_key = 3141592;
    packet_stream->encode_key = 3141592;*/
#endif

#ifndef NO_RSA
    if (rsa_init(&packet_stream->rsa,
        mud->rsa_exponent, mud->rsa_modulus) < 0) {
            mud_error("rsa_init failed\n");
            exit(1);
    }
#endif

    int ret = 0;

/*#ifdef WII
    char local_ip[16] = {0};
    char gateway[16] = {0};
    char netmask[16] = {0};

    ret = if_config(local_ip, netmask, gateway, TRUE, 20);

    if (ret < 0) {
        mud_error("if_config(): %d\n", ret);
        exit(1);
    }
#endif*/

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(mud->port);

#if defined(WIN9X) || defined(WII)
    struct hostent *host_addr = gethostbyname(mud->server);

    if (host_addr) {
        memcpy(&server_addr.sin_addr, host_addr->h_addr_list[0],
               sizeof(struct in_addr));
    }
#else
    struct addrinfo hints = {0};
    struct addrinfo *result = {0};

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int status = getaddrinfo(mud->server, NULL, &hints, &result);

    if (status != 0) {
     //   mud_error("getaddrinfo(): %s\n", gai_strerror(status));  // Bruce.
     mud_error("getaddrinfo() failed with error code: %d\n", status);



        packet_stream->closed = 1;
        return;
    }

    for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
        struct sockaddr_in *addr = (struct sockaddr_in *)rp->ai_addr;

        if (addr != NULL) {
            memcpy(&server_addr.sin_addr, &addr->sin_addr,
                   sizeof(struct in_addr));
            break;
        }
    }

    freeaddrinfo(result);
#endif

#ifdef __SWITCH__
    socketInitializeDefault();
#endif

    packet_stream->socket = socket(AF_INET, SOCK_STREAM, 0);
    int buf = 64 * 1024;  /* 64 KB */
    setsockopt(packet_stream->socket, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(buf));
    setsockopt(packet_stream->socket, SOL_SOCKET, SO_SNDBUF, &buf, sizeof(buf));

    if (packet_stream->socket < 0) {
        mud_error("socket error: %s (%d)\n", strerror(errno), errno);
        packet_stream_close(packet_stream);
        return;
    }

    int set = 1;

#ifdef DREAMCAST
    int flags = fcntl(packet_stream->socket, F_GETFL, 0);
    if (flags < 0) {
        mud_error("fcntl F_GETFL failed: %s\n", strerror(errno));
        packet_stream_close(packet_stream);
        return;
    }
    if (fcntl(packet_stream->socket, F_SETFL, flags | O_NONBLOCK) < 0) {
        mud_error("fcntl F_SETFL failed: %s\n", strerror(errno));
        packet_stream_close(packet_stream);
        return;
    }
#endif

#ifdef TCP_NODELAY
    int one = 1;
    setsockopt(packet_stream->socket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

#endif

#ifdef EMSCRIPTEN
    int attempts_ms = 0;

    do {
        if (attempts_ms >= 5000) {
            break;
        }

        ret = connect(packet_stream->socket, (struct sockaddr *)&server_addr,
                      sizeof(server_addr));

        if (errno == 30) {
            ret = 0;
            break;
        } else if (errno != EINPROGRESS && errno != 7) {
            /* not sure what 7 is, but i'm not worried about portability
             * since this is explicitly for emscripten */
            break;
        }

        delay_ticks(100);
        attempts_ms += 100;
    } while (ret == -1);
#else

#ifdef FIONBIO
    ret = ioctl(packet_stream->socket, FIONBIO, &set);

    if (ret < 0) {
        mud_error("ioctl() error: %d\n", ret);
        exit(1);
    }
#endif

#ifdef HAVE_SIGNALS
    (void)signal(SIGPIPE, on_signal_do_nothing);
#endif

    ret = connect(packet_stream->socket, (struct sockaddr *)&server_addr,
                  sizeof(server_addr));

    if (ret == -1) {
#ifdef WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
        if (errno == EINPROGRESS) {
#endif
            struct timeval timeout = {0};
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;

            fd_set write_fds;
            FD_ZERO(&write_fds);
            FD_SET(packet_stream->socket, &write_fds);

            ret = select(packet_stream->socket + 1, NULL, &write_fds, NULL,
                         &timeout);

            if (ret > 0) {
                socklen_t lon = sizeof(int);
                int valopt = 0;

                if (getsockopt(packet_stream->socket, SOL_SOCKET, SO_ERROR,
                               (void *)(&valopt), &lon) < 0) {
                    mud_error("getsockopt() error:  %s (%d)\n", strerror(errno),
                              errno);

                    exit(1);
                }

                if (valopt > 0) {
                    ret = -1;
                    errno = valopt;
                } else {
                    ret = 0;
                }
            } else if (ret == 0) {
                mud_error("connect() timeout\n");
                packet_stream_close(packet_stream);
                return;
            }
        }
    }
#endif /* not EMSCRIPTEN */

    if (ret < 0 && errno != 0) {
        mud_error("connect() error: %s (%d)\n", strerror(errno), errno);
        packet_stream_close(packet_stream);
        return;
    }

    packet_stream->closed = 0;
    packet_stream->packet_end = 3;
    packet_stream->packet_max_length = 5000;
}

int packet_stream_available_bytes(PacketStream *ps, int length) {
    if (ps->available_length >= length) {
        return 1;
    }
    int bytes = recv(ps->socket,
                     ps->available_buffer + ps->available_offset +
                         ps->available_length,
                     length - ps->available_length,
                     0);
    if (bytes > 0) {
        ps->available_length += bytes;
    } else if (bytes == 0) {
        ps->closed = 1;
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            ps->closed = 1;
        }
    }
    return ps->available_length >= length;
}

int packet_stream_read_bytes(PacketStream *packet_stream, int length,
                             int8_t *buffer) {
    if (packet_stream->closed) {
        return -1;
    }

    if (packet_stream->available_length > 0) {
        int copy_length;

        if (length > packet_stream->available_length) {
            copy_length = packet_stream->available_length;
        } else {
            copy_length = length;
        }

        memcpy(buffer,
               packet_stream->available_buffer +
                   packet_stream->available_offset,
               copy_length);

        length -= copy_length;

        packet_stream->available_length -= copy_length;

        if (packet_stream->available_length == 0) {
            packet_stream->available_offset = 0;
        } else {
            packet_stream->available_offset += copy_length;
        }
    }

    /* how many ticks we've been waiting to read for */
    int read_duration = 0;

    int offset = 0;

    while (length > 0) {
        int bytes = recv(packet_stream->socket, buffer + offset, length, 0);
        if (bytes > 0) {
            length -= bytes;
            offset += bytes;
        } else if (bytes == 0) {
            packet_stream->closed = 1;
            return -1;
        } else {
            read_duration += 1;

            if (read_duration >= 5000) {
                packet_stream_close(packet_stream);
                return -1;
            } else {
                delay_ticks(1);
            }
        }
    }

    return 0;
}

int packet_stream_write_bytes(PacketStream *packet_stream, int8_t *buffer,
                              int offset, int length) {
    if (!packet_stream->closed) {
#if defined(WIN32) || defined(__SWITCH__)
        return send(packet_stream->socket, buffer + offset, length, 0);
#else
        return write(packet_stream->socket, buffer + offset, length);
#endif
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
#if 0
    int i = packet_stream->packet_data[packet_stream->packet_start + 2] & 0xff;

    packet_stream->packet_data[packet_stream->packet_start + 2] =
        (int8_t)(i + packet_stream->decode_key);

    int opcode_friend = packet_stream->opcode_friend;

    packet_stream->encode_threat_index =
        (packet_stream->encode_threat_index + opcode_friend) % THREAT_LENGTH;

    char threat_character = SPOOKY_THREAT[packet_stream->encode_threat_index];

    packet_stream->encode_key =
        packet_stream->encode_key * 3 + (int)threat_character + opcode_friend &
        0xffff;
#endif

    int length = packet_stream->packet_end - packet_stream->packet_start - 2;

    if (length >= 160) {
        packet_stream->packet_data[packet_stream->packet_start] =
            (160 + (length / 256)) & 0xff;

        packet_stream->packet_data[packet_stream->packet_start + 1] =
            length & 0xff;
    } else {
        packet_stream->packet_data[packet_stream->packet_start] = length & 0xff;
        packet_stream->packet_end--;

        packet_stream->packet_data[packet_stream->packet_start + 1] =
            packet_stream->packet_data[packet_stream->packet_end];
    }

    packet_stream->packet_start = packet_stream->packet_end;
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
static void packet_stream_put_rsa(PacketStream *packet_stream,
                                  void *input, size_t input_len) {
    uint8_t result[64] = {0};
    int res_len;

    res_len = rsa_crypt(&packet_stream->rsa,
                  input, input_len, result, sizeof(result));
    if (res_len < 0) {
        mud_error("failed to rsa_crypt\n");
        return;
    }

    packet_stream_put_byte(packet_stream, sizeof(result));

    /* in java's BigInteger byte array array, zeros at the beginning are
     * ignored unless they're being used to indicate the MSB for sign. since
     * the byte array lengths range from 63-65 and we always want a positive
     * integer, we can make result_length 65 and begin with up to two 0 bytes */
    for (size_t i = 0; i < (sizeof(result) - res_len); ++i) {
        packet_stream_put_byte(packet_stream, 0);
    }
    for (int i = 0; i < res_len; ++i) {
        packet_stream_put_byte(packet_stream, result[i]);
    }
}
#endif

#ifndef REVISION_177
void packet_stream_put_login_block(PacketStream *packet_stream,
                                   const char *username, const char *password,
                                   uint32_t *isaac_keys, uint32_t uuid) {
    uint8_t input_block[16 + (sizeof(uint32_t) * 4) + 4 + USERNAME_LENGTH +
                        PASSWORD_LENGTH];

    size_t username_len = strlen(username);
    size_t password_len = strlen(password);
    uint8_t *p = input_block;

#if !defined(NO_ISAAC) || !defined(NO_RSA)
    *(p++) = '\n'; /* Magic for sanity checks by the server. */
#endif

#ifndef NO_ISAAC
    memset(packet_stream->isaac_in.randrsl, 0,
           sizeof(packet_stream->isaac_in.randrsl));

    memset(packet_stream->isaac_out.randrsl, 0,
           sizeof(packet_stream->isaac_out.randrsl));
#endif

    for (unsigned int i = 0; i < 4; ++i) {
#ifndef NO_ISAAC
        packet_stream->isaac_in.randrsl[i] = isaac_keys[i];
        packet_stream->isaac_out.randrsl[i] = isaac_keys[i];
#endif
        write_unsigned_int(p, 0, isaac_keys[i]);
        p += 4;
    }

    write_unsigned_int(p, 0, uuid);
    p += 4;

    memcpy(p, username, username_len);
    p += username_len;
    *(p++) = '\n';

    memcpy(p, password, password_len);
    p += password_len;
    *(p++) = '\n';

#ifndef NO_ISAAC
    isaac_init(&packet_stream->isaac_in, 1);
    isaac_init(&packet_stream->isaac_out, 1);
    packet_stream->isaac_ready = 1;
#endif

#ifndef NO_RSA
    packet_stream_put_rsa(packet_stream, input_block, p - input_block);
#else
    packet_stream_put_byte(packet_stream, p - input_block);
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
    int i = packet_stream_get_short(packet_stream);
    int j = packet_stream_get_short(packet_stream);
    int k = packet_stream_get_short(packet_stream);
    int l = packet_stream_get_short(packet_stream);

    return ((int64_t)i << 48) + ((int64_t)j << 32) + ((int64_t)k << 16) + l;
}

void packet_stream_close(PacketStream *packet_stream) {
    if (packet_stream->socket > -1) {
        close(packet_stream->socket);
        packet_stream->socket = -1;
    }

    packet_stream->closed = 1;
}
