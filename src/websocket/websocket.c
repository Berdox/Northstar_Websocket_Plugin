#include "websocket/websocket.h"
#include "interfaces/sys.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

struct lws_context *g_lws_ctx = NULL;
static WsConn g_conns[MAX_WS_CONNECTIONS];
static CRITICAL_SECTION g_conns_lock;

// ---- lws forward declare ----
static int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len);

static struct lws_protocols g_protocols[] = {
    { "ns-ws", ws_callback, 0, MAX_MESSAGE_LEN },
    { NULL, NULL, 0, 0 }
};

// ---- init / shutdown ----

void ws_init(void) {
    InitializeCriticalSection(&g_conns_lock);
    memset(g_conns, 0, sizeof(g_conns));
    for (int i = 0; i < MAX_WS_CONNECTIONS; i++)
    {
        InitializeCriticalSection(&g_conns[i].queue_lock);
        InitializeCriticalSection(&g_conns[i].send_lock);
    }

    struct lws_context_creation_info info = {0};
    info.port      = CONTEXT_PORT_NO_LISTEN;
    info.protocols = g_protocols;
    info.options   = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.gid = info.uid = -1;

    g_lws_ctx = lws_create_context(&info);
    if (!g_lws_ctx)
        ns_log(LOG_ERR, "[ws] failed to create lws context");
}

void ws_shutdown(void) {
    if (g_lws_ctx) {
        lws_context_destroy(g_lws_ctx);
        g_lws_ctx = NULL;
    }
    for (int i = 0; i < MAX_WS_CONNECTIONS; i++) {
        DeleteCriticalSection(&g_conns[i].queue_lock);
        DeleteCriticalSection(&g_conns[i].send_lock);
    }
    DeleteCriticalSection(&g_conns_lock);
}

void ws_service(void) {
    if (g_lws_ctx)
        lws_service(g_lws_ctx, 0);
}

// ---- helpers ----

static WsConn *find_conn(const char *name) {
    for (int i = 0; i < MAX_WS_CONNECTIONS; i++)
        if (g_conns[i].in_use && strcmp(g_conns[i].name, name) == 0)
            return &g_conns[i];
    return NULL;
}

static WsConn *alloc_conn(const char *name) {
    for (int i = 0; i < MAX_WS_CONNECTIONS; i++) {
        if (!g_conns[i].in_use) {
            memset(&g_conns[i], 0, sizeof(WsConn));
            strncpy(g_conns[i].name, name, MAX_SOCKET_NAME_LEN - 1);
            g_conns[i].in_use = true;
            return &g_conns[i];
        }
    }
    return NULL;
}

static void enqueue_recv(WsConn *conn, const char *data, size_t len) {
    EnterCriticalSection(&conn->queue_lock);
    int next = (conn->queue_tail + 1) % MAX_QUEUED_MESSAGES;
    if (next != conn->queue_head) { // not full
        conn->queue[conn->queue_tail].data = malloc(len + 1);
        if (conn->queue[conn->queue_tail].data) {
            memcpy(conn->queue[conn->queue_tail].data, data, len);
            conn->queue[conn->queue_tail].data[len] = '\0';
            conn->queue[conn->queue_tail].len = len;
            conn->queue_tail = next;
        }
    }
    LeaveCriticalSection(&conn->queue_lock);
}

static void enqueue_send(WsConn *conn, const char *data, size_t len) {
    EnterCriticalSection(&conn->send_lock);
    int next = (conn->send_tail + 1) % MAX_QUEUED_MESSAGES;
    if (next != conn->send_head) {
        conn->send_queue[conn->send_tail].data = malloc(len + 1);
        if (conn->send_queue[conn->send_tail].data) {
            memcpy(conn->send_queue[conn->send_tail].data, data, len);
            conn->send_queue[conn->send_tail].data[len] = '\0';
            conn->send_queue[conn->send_tail].len = len;
            conn->send_tail = next;
        }
    }
    // Tell lws this wsi has data to write
    if (conn->wsi)
        lws_callback_on_writable(conn->wsi);
    LeaveCriticalSection(&conn->send_lock);
}

// ---- header parser ----
// Input format: "Key|#!#|Value|#!#|Key2|#!#|Value2|#!#|"
// Splits into key/value pairs and adds to lws connect info via lws_add_http_header_by_name
// We store parsed pairs in a flat array for use during connect
typedef struct { char key[256]; char val[512]; } Header;

static int parse_headers(const char *header_str, Header *out, int max) {
    int count = 0;
    const char *sep = "|#!#|";
    size_t seplen = strlen(sep);
    char buf[16384];
    strncpy(buf, header_str, sizeof(buf) - 1);

    char *p = buf;
    while (*p && count < max) {
        char *end = strstr(p, sep);
        if (!end) break;
        *end = '\0';
        strncpy(out[count].key, p, sizeof(out[count].key) - 1);
        p = end + seplen;

        end = strstr(p, sep);
        if (!end) break;
        *end = '\0';
        strncpy(out[count].val, p, sizeof(out[count].val) - 1);
        p = end + seplen;
        count++;
    }
    return count;
}

// ---- connect ----

bool ws_connect(const char *name, const char *url, const char *headers,
                int timeout_sec, bool keep_alive) {
    EnterCriticalSection(&g_conns_lock);

    WsConn *existing = find_conn(name);
    if (existing) {
        if (keep_alive && existing->connected) {
            LeaveCriticalSection(&g_conns_lock);
            return true; // already open, honour keep_alive
        }
        // close old one
        existing->in_use = false;
        if (existing->wsi)
            lws_close_reason(existing->wsi, LWS_CLOSE_STATUS_NORMAL, NULL, 0);
        existing->wsi = NULL;
    }

    WsConn *conn = alloc_conn(name);
    LeaveCriticalSection(&g_conns_lock);

    if (!conn) { ns_log(LOG_ERR, "[ws] connection pool full"); return false; }

    // Parse URL
    char host[256] = {0}, path[512] = "/";
    int port = 80, use_ssl = 0;
    const char *after = url;

    if (strncmp(url, "wss://", 6) == 0) { use_ssl = 1; port = 443; after = url + 6; }
    else if (strncmp(url, "ws://", 5) == 0) { after = url + 5; }

    const char *slash = strchr(after, '/');
    if (slash) {
        size_t hlen = slash - after;
        strncpy(host, after, hlen < sizeof(host) ? hlen : sizeof(host) - 1);
        strncpy(path, slash, sizeof(path) - 1);
    } else {
        strncpy(host, after, sizeof(host) - 1);
    }
    char *colon = strchr(host, ':');
    if (colon) { port = atoi(colon + 1); *colon = '\0'; }

    // Parse headers — store on conn for use in CONNECTING callback
    // libwebsockets doesn't let you set headers in client_connect_info directly for ws://
    // We'll use the CLIENT_APPEND_HANDSHAKE_HEADER callback path instead
    // Store parsed headers on conn temporarily (we'll re-parse in callback)
    // For simplicity, copy the raw header string onto the conn userdata
    // (conn pointer is passed as userdata)

    struct lws_client_connect_info ci = {0};
    ci.context        = g_lws_ctx;
    ci.address        = host;
    ci.port           = port;
    ci.path           = path;
    ci.host           = host;
    ci.origin         = host;
    ci.protocol       = g_protocols[0].name;
    ci.ssl_connection = use_ssl ? LCCSCF_USE_SSL : 0;
    ci.userdata       = conn; // find conn back in callback by pointer

    // Store raw header string for use in APPEND_HANDSHAKE_HEADER callback
    // We abuse frag_buf temporarily since we haven't connected yet
    strncpy(conn->frag_buf, headers ? headers : "", MAX_MESSAGE_LEN - 1);

    conn->wsi = lws_client_connect_via_info(&ci);
    if (!conn->wsi) {
        ns_log(LOG_ERR, "[ws] lws_client_connect_via_info failed");
        conn->in_use = false;
        return false;
    }

    // Wait up to timeout_sec for connection (blocking poll — called before game loop)
    DWORD deadline = GetTickCount() + (DWORD)(timeout_sec * 1000);
    while (!conn->connected && !conn->error && GetTickCount() < deadline)
        lws_service(g_lws_ctx, 50);

    if (!conn->connected) {
        ns_log(LOG_WARN, "[ws] connection timed out");
        conn->in_use = false;
        return false;
    }
    return true;
}

// ---- write ----

bool ws_write(const char *name, const char *message) {
    WsConn *conn = find_conn(name);
    if (!conn || !conn->connected) return false;
    enqueue_send(conn, message, strlen(message));
    return true;
}

// ---- disconnect ----

void ws_disconnect(const char *name) {
    WsConn *conn = find_conn(name);
    if (!conn) return;
    conn->in_use = false;
    if (conn->wsi)
        lws_close_reason(conn->wsi, LWS_CLOSE_STATUS_NORMAL, NULL, 0);
    conn->wsi = NULL;
}

// ---- read (drains the queue) ----

int ws_read(const char *name, char out[][MAX_MESSAGE_LEN], int max_out) {
    WsConn *conn = find_conn(name);
    if (!conn) return 0;

    int count = 0;
    EnterCriticalSection(&conn->queue_lock);
    while (conn->queue_head != conn->queue_tail && count < max_out) {
        WsMessage *m = &conn->queue[conn->queue_head];
        strncpy(out[count], m->data, MAX_MESSAGE_LEN - 1);
        free(m->data);
        m->data = NULL;
        conn->queue_head = (conn->queue_head + 1) % MAX_QUEUED_MESSAGES;
        count++;
    }
    LeaveCriticalSection(&conn->queue_lock);
    return count;
}

// ---- get open sockets ----

int ws_get_open(char out[][MAX_SOCKET_NAME_LEN], int max_out) {
    int count = 0;
    for (int i = 0; i < MAX_WS_CONNECTIONS && count < max_out; i++)
        if (g_conns[i].in_use && g_conns[i].connected)
            strncpy(out[count++], g_conns[i].name, MAX_SOCKET_NAME_LEN - 1);
    return count;
}

// ---- lws callback ----

static int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len) {
    // lws passes our ci.userdata as the first per-session data if per_session_data_size=0
    // We recover the conn by scanning (wsi is unique)
    WsConn *conn = NULL;
    for (int i = 0; i < MAX_WS_CONNECTIONS; i++)
        if (g_conns[i].in_use && g_conns[i].wsi == wsi)
            { conn = &g_conns[i]; break; }

    switch (reason) {
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: {
        // Inject custom headers from frag_buf
        if (!conn) break;
        Header hdrs[MAX_HEADERS];
        int n = parse_headers(conn->frag_buf, hdrs, MAX_HEADERS);
        unsigned char **p   = (unsigned char **)in;
        unsigned char  *end = (*p) + len;
        for (int i = 0; i < n; i++) {
            if (lws_add_http_header_by_name(wsi,
                    (const unsigned char *)hdrs[i].key,
                    (const unsigned char *)hdrs[i].val,
                    (int)strlen(hdrs[i].val), p, end))
                ns_logf(LOG_WARN, "[ws] header '%s' truncated", hdrs[i].key);
        }
        // Clear frag_buf now that headers are sent
        conn->frag_buf[0] = '\0';
        conn->frag_len = 0;
        break;
    }

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        if (conn) { conn->connected = true; conn->error = false; }
        ns_log(LOG_INFO, "[ws] connected");
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE: {
        if (!conn) break;
        bool is_final = lws_is_final_fragment(wsi);
        // Accumulate fragments
        size_t space = MAX_MESSAGE_LEN - conn->frag_len - 1;
        size_t copy  = len < space ? len : space;
        memcpy(conn->frag_buf + conn->frag_len, in, copy);
        conn->frag_len += copy;
        if (is_final) {
            conn->frag_buf[conn->frag_len] = '\0';
            enqueue_recv(conn, conn->frag_buf, conn->frag_len);
            conn->frag_len = 0;
        }
        break;
    }

    case LWS_CALLBACK_CLIENT_WRITEABLE: {
        if (!conn) break;
        EnterCriticalSection(&conn->send_lock);
        if (conn->send_head != conn->send_tail) {
            WsMessage *m = &conn->send_queue[conn->send_head];
            unsigned char *buf = malloc(LWS_PRE + m->len);
            if (buf) {
                memcpy(buf + LWS_PRE, m->data, m->len);
                lws_write(wsi, buf + LWS_PRE, m->len, LWS_WRITE_TEXT);
                free(buf);
            }
            free(m->data); m->data = NULL;
            conn->send_head = (conn->send_head + 1) % MAX_QUEUED_MESSAGES;
            // If more messages queued, request another writable callback
            if (conn->send_head != conn->send_tail)
                lws_callback_on_writable(wsi);
        }
        LeaveCriticalSection(&conn->send_lock);
        break;
    }

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        ns_logf(LOG_ERR, "[ws] connection error: %s", in ? (char*)in : "unknown");
        if (conn) { conn->error = true; conn->connected = false; conn->wsi = NULL; }
        break;

    case LWS_CALLBACK_CLIENT_CLOSED:
        if (conn) { conn->connected = false; conn->wsi = NULL; conn->in_use = false; }
        ns_log(LOG_INFO, "[ws] closed");
        break;

    default: break;
    }
    return 0;
}