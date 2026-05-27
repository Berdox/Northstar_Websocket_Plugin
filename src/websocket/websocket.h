#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <windows.h>
#endif

#include <libwebsockets.h>
#include <stdbool.h>
#include <stddef.h>
#include <windows.h>

#define MAX_WS_CONNECTIONS   32
#define MAX_QUEUED_MESSAGES  256
#define MAX_MESSAGE_LEN      65536
#define MAX_SOCKET_NAME_LEN  128
#define MAX_HEADERS          32

typedef struct {
    char *data;
    size_t len;
} WsMessage;

typedef struct {
    char             name[MAX_SOCKET_NAME_LEN];
    struct lws      *wsi;
    bool             in_use;
    bool             connected;
    bool             error;

    // Inbound message queue (plugin fills, Squirrel drains)
    WsMessage        queue[MAX_QUEUED_MESSAGES];
    int              queue_head;
    int              queue_tail;
    CRITICAL_SECTION queue_lock;

    // Outbound send queue (Squirrel fills, lws callback drains)
    WsMessage        send_queue[MAX_QUEUED_MESSAGES];
    int              send_head;
    int              send_tail;
    CRITICAL_SECTION send_lock;

    // Frame reassembly buffer
    char             frag_buf[MAX_MESSAGE_LEN];
    size_t           frag_len;
} WsConn;

void ws_init(void);
void ws_shutdown(void);
void ws_service(void);  // call from RunFrame

// PL_ function implementations
bool   ws_connect(const char *name, const char *url, const char *headers,
                  int timeout_sec, bool keep_alive);
bool   ws_write(const char *name, const char *message);
void   ws_disconnect(const char *name);
int    ws_read(const char *name, char out[][MAX_MESSAGE_LEN], int max_out); // returns count
int    ws_get_open(char out[][MAX_SOCKET_NAME_LEN], int max_out);

extern struct lws_context *g_lws_ctx;

#endif