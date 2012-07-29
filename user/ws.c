
#include <inc/determinism.h>
#include <inc/fork_nondet.h>
#include <inc/fs_nondet.h>

#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <sys/stat.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MONGOOSE_VERSION "3.0"
#define MAX_OPTIONS 40
#define TODO do { printf("TODO %s %d\n", __func__, __LINE__); \
exit(1); } while (0)

#define closesocket(a) close(a)
#define isbyte(n) ((n) >= 0 && (n) <= 255)
#define cry(...) 
#define DEBUG_TRACE(x) do { \
    printf("**** %lu.%s.%d: ", (unsigned long)time(NULL), __func__, __LINE__); \
    printf x; printf("\n"); } while (0)

#define INVALID_SOCKET (-1)
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static char config_file[PATH_MAX];  // Set by process_command_line_arguments()
#if !defined(CONFIG_FILE)
#define CONFIG_FILE "mongoose.conf"
#endif /* !CONFIG_FILE */
#define DIRSEP '/'

// Snatched from OpenSSL includes. I put the prototypes here to be independent
// from the OpenSSL source installation. Having this, mongoose + SSL can be
// built on any system with binary SSL libraries installed.
typedef struct ssl_st SSL;
typedef struct ssl_method_st SSL_METHOD;
typedef struct ssl_ctx_st SSL_CTX;

struct mg_context;     // Handle for the HTTP service itself
struct mg_connection;  // Handle for the individual connection
void mg_stop(struct mg_context *ctx);

static void mg_strlcpy(register char *dst, register const char *src, size_t n) {
  for (; *src != '\0' && n > 1; n--) {
    *dst++ = *src++;
  }
  *dst = '\0';
}

const char *mg_version(void) {
  return MONGOOSE_VERSION;
}

static int lowercase(const char *s) {
  return tolower(* (const unsigned char *) s);
}

static int mg_strncasecmp(const char *s1, const char *s2, size_t len) {
  int diff = 0;

  if (len > 0)
    do {
      diff = lowercase(s1++) - lowercase(s2++);
    } while (diff == 0 && s1[-1] != '\0' && --len > 0);

  return diff;
}

static int mg_strcasecmp(const char *s1, const char *s2) {
  int diff;

  do {
    diff = lowercase(s1++) - lowercase(s2++);
  } while (diff == 0 && s1[-1] != '\0');

  return diff;
}

static char * mg_strndup(const char *ptr, size_t len) {
  char *p;

  if ((p = (char *) malloc(len + 1)) != NULL) {
    mg_strlcpy(p, ptr, len + 1);
  }

  return p;
}

static char * mg_strdup(const char *str) {
  return mg_strndup(str, strlen(str));
}

// Like snprintf(), but never returns negative value, or the value
// that is larger than a supplied buffer.
// Thanks to Adam Zeldis to pointing snprintf()-caused vulnerability
// in his audit report.
static int mg_vsnprintf(struct mg_connection *conn, char *buf, size_t buflen,
                        const char *fmt, va_list ap) {
  int n;

  if (buflen == 0)
    return 0;

  n = vsnprintf(buf, buflen, fmt, ap);

  if (n < 0) {
    cry(conn, "vsnprintf error");
    n = 0;
  } else if (n >= (int) buflen) {
    cry(conn, "truncating vsnprintf buffer: [%.*s]",
        n > 200 ? 200 : n, buf);
    n = (int) buflen - 1;
  }
  buf[n] = '\0';

  return n;
}

static int mg_snprintf(struct mg_connection *conn, char *buf, size_t buflen,
                       const char *fmt, ...) {
  va_list ap;
  int n;

  va_start(ap, fmt);
  n = mg_vsnprintf(conn, buf, buflen, fmt, ap);
  va_end(ap);

  return n;
}

// Skip the characters until one of the delimiters characters found.
// 0-terminate resulting word. Skip the delimiter and following whitespaces if any.
// Advance pointer to buffer to the next word. Return found 0-terminated word.
// Delimiters can be quoted with quotechar.
static char *skip_quoted(char **buf, const char *delimiters, const char *whitespace, char quotechar) {
  char *p, *begin_word, *end_word, *end_whitespace;

  begin_word = *buf;
  end_word = begin_word + strcspn(begin_word, delimiters);

  /* Check for quotechar */
  if (end_word > begin_word) {
    p = end_word - 1;
    while (*p == quotechar) {
      /* If there is anything beyond end_word, copy it */
      if (*end_word == '\0') {
        *p = '\0';
        break;
      } else {
        size_t end_off = strcspn(end_word + 1, delimiters);
        memmove (p, end_word, end_off + 1);
        p += end_off; /* p must correspond to end_word - 1 */
        end_word += end_off + 1;
      }
    }
    for (p++; p < end_word; p++) {
      *p = '\0';
    }
  }

  if (*end_word == '\0') {
    *buf = end_word;
  } else {
    end_whitespace = end_word + 1 + strspn(end_word + 1, whitespace);

    for (p = end_word; p < end_whitespace; p++) {
      *p = '\0';
    }

    *buf = end_whitespace;
  }

  return begin_word;
}

// Simplified version of skip_quoted without quote char
// and whitespace == delimiters
static char *skip(char **buf, const char *delimiters) {
  return skip_quoted(buf, delimiters, delimiters, 0);
}

enum {
  CGI_EXTENSIONS, CGI_ENVIRONMENT, PUT_DELETE_PASSWORDS_FILE, CGI_INTERPRETER,
  PROTECT_URI, AUTHENTICATION_DOMAIN, SSI_EXTENSIONS, ACCESS_LOG_FILE,
  SSL_CHAIN_FILE, ENABLE_DIRECTORY_LISTING, ERROR_LOG_FILE,
  GLOBAL_PASSWORDS_FILE, INDEX_FILES,
  ENABLE_KEEP_ALIVE, ACCESS_CONTROL_LIST, MAX_REQUEST_SIZE,
  EXTRA_MIME_TYPES, LISTENING_PORTS,
  DOCUMENT_ROOT, SSL_CERTIFICATE, NUM_THREADS, RUN_AS_USER,
  NUM_OPTIONS
};

// Describes a string (chunk of memory).
struct vec {
  const char *ptr;
  size_t len;
};

struct mg_request_info {
    void *user_data;       // User-defined pointer passed to mg_start()
    char *request_method;  // "GET", "POST", etc
    char *uri;             // URL-decoded URI
    char *http_version;    // E.g. "1.0", "1.1"
    char *query_string;    // \0 - terminated
    char *remote_user;     // Authenticated user
    char *log_message;     // Mongoose error log message
    long remote_ip;        // Client's IP address
    int remote_port;       // Client's port
    int status_code;       // HTTP reply status code
    int is_ssl;            // 1 if SSL-ed, 0 if not
    int num_headers;       // Number of headers
    struct mg_header {
        char *name;          // HTTP header name
        char *value;         // HTTP header value
    } http_headers[64];    // Maximum 64 headers
};
// Various events on which user-defined function is called by Mongoose.
enum mg_event {
    MG_NEW_REQUEST,   // New HTTP request has arrived from the client
    MG_HTTP_ERROR,    // HTTP error must be returned to the client
    MG_EVENT_LOG,     // Mongoose logs an event, request_info.log_message
    MG_INIT_SSL       // Mongoose initializes SSL. Instead of mg_connection *,
    // SSL context is passed to the callback function.
};

// Prototype for the user-defined function. Mongoose calls this function
// on every event mentioned above.
//
// Parameters:
//   event: which event has been triggered.
//   conn: opaque connection handler. Could be used to read, write data to the
//         client, etc. See functions below that accept "mg_connection *".
//   request_info: Information about HTTP request.
//
// Return:
//   If handler returns non-NULL, that means that handler has processed the
//   request by sending appropriate HTTP reply to the client. Mongoose treats
//   the request as served.
//   If callback returns NULL, that means that callback has not processed
//   the request. Handler must not send any data to the client in this case.
//   Mongoose proceeds with request handling as if nothing happened.
typedef void * (*mg_callback_t)(enum mg_event event,
        struct mg_connection *conn,
        const struct mg_request_info *request_info);

static struct mg_context *start_mongoose(int argc, char *argv[]);
struct mg_context *mg_start(mg_callback_t user_callback, void *user_data,
                            const char **options);

// Unified socket address. For IPv6 support, add IPv6 address structure
// in the union u.
struct usa {
    socklen_t len;
    union {
        struct sockaddr sa;
        struct sockaddr_in sin;
    } u;
};

typedef int SOCKET;
/* Describes listening socket, or socket which was accept()-ed by the master
   thread and queued for future handling by the worker thread. */
struct socket {
    struct socket *next;  // Linkage
    SOCKET sock;          // Listening socket
    struct usa lsa;       // Local socket address
    struct usa rsa;       // Remote socket address
    int is_ssl;           // Is socket SSL-ed
    int is_proxy;
};

static const char *config_options[] = {
  "C", "cgi_extensions", ".cgi,.pl,.php",
  "E", "cgi_environment", NULL,
  "G", "put_delete_passwords_file", NULL,
  "I", "cgi_interpreter", NULL,
  "P", "protect_uri", NULL,
  "R", "authentication_domain", "mydomain.com",
  "S", "ssi_extensions", ".shtml,.shtm",
  "a", "access_log_file", NULL,
  "c", "ssl_chain_file", NULL,
  "d", "enable_directory_listing", "yes",
  "e", "error_log_file", NULL,
  "g", "global_passwords_file", NULL,
  "i", "index_files", "index.html,index.htm,index.cgi",
  "k", "enable_keep_alive", "no",
  "l", "access_control_list", NULL,
  "M", "max_request_size", "16384",
  "m", "extra_mime_types", NULL,
  "p", "listening_ports", "8080",
  "r", "document_root",  ".",
  "s", "ssl_certificate", NULL,
  "t", "num_threads", "10",
  "u", "run_as_user", NULL,
  NULL
};
#define ENTRIES_PER_CONFIG_OPTION 3

#define NTHREADS 10
#define THREAD_RUNNING 0
#define THREAD_WAITING 1
#define THREAD_FAULTED 2
struct thread_state
{
    int state; /* 0=running, 1=waiting for data, 2 = faulted */
    char fin[20]; /* "/threads/N_in" N is thread number */
    char fout[20]; /* "/threads/N_out" N is thread number */
    struct mg_connection *conn;
};

struct mg_context {
    int stop_flag;       // Should we stop event loop
    //SSL_CTX *ssl_ctx;             // SSL context
    char *config[NUM_OPTIONS];    // Mongoose configuration parameters
    mg_callback_t user_callback;  // User-defined callback function
    void *user_data;              // User-defined data

    struct socket *listening_sockets;

    int num_threads;  // Number of threads
    //pthread_mutex_t mutex;     // Protects (max|num)_threads
    //pthread_cond_t  cond;      // Condvar for tracking workers terminations

    /* We won't actually use the queue. */
    struct socket queue[20];   // Accepted sockets
    int sq_head;      // Head of the socket queue
    int sq_tail;      // Tail of the socket queue
    //pthread_cond_t sq_full;    // Singaled when socket is produced
    //pthread_cond_t sq_empty;   // Signaled when socket is consumed
    struct thread_state threads[NTHREADS]; /* Indexed by tid */
    int next_thread; /* Next available thread id. */
    int oldest_thread; /* Oldest thread id that is in progress. */
};
struct mg_context *g_ctx;
static char server_name[40];        // Set by init_server_name()

struct mg_connection {
    struct socket client;       // Connected client
    struct mg_context *ctx;
    /* Fields above this line are not sent to the worker threads. */
    /* Fields from here down are sent to the worker threads. */
    struct mg_connection *peer; // Remote target in proxy mode
    struct mg_request_info request_info;
    //SSL *ssl;                   // SSL descriptor
    time_t birth_time;          // Time connection was accepted
    int64_t num_bytes_sent;     // Total bytes sent to client
    int64_t content_len;        // Content-Length header value
    int64_t consumed_content;   // How many bytes of content is already read
    int request_len;            // Size of the request + headers in a buffer
    int data_len;               // Total size of data in a buffer
    pid_t tid;
    int buf_size;               // Buffer size
    char buf[0];                // Buffer for received data
};

/* When we send data to a chid, we don't send certain parts of the struct. */
#define deterministic_mg_conn(conn) \
    ((void*)(conn) + sizeof(struct socket) + sizeof(struct mg_context*))
#define deterministic_mg_conn_size(conn) \
    (conn->buf_size + sizeof(*conn) - sizeof(struct socket) - \
     sizeof(struct mg_context*))

static void signal_handler(int sig_num) {
    g_ctx->stop_flag = sig_num;
}

const char **mg_get_valid_option_names(void) {
  return config_options;
}

static int get_option_index(const char *name) {
  int i;

  for (i = 0; config_options[i] != NULL; i += ENTRIES_PER_CONFIG_OPTION) {
    if (strcmp(config_options[i], name) == 0 ||
        strcmp(config_options[i + 1], name) == 0) {
      return i / ENTRIES_PER_CONFIG_OPTION;
    }
  }
  return -1;
}

const char *mg_get_option(const struct mg_context *ctx, const char *name) {
  int i;
  if ((i = get_option_index(name)) == -1) {
    return NULL;
  } else if (ctx->config[i] == NULL) {
    return "";
  } else {
    return ctx->config[i];
  }
}

static void init_server_name(void) {
  snprintf(server_name, sizeof(server_name), "Mongoose web server v.%s",
           mg_version());
}

static void die(const char *fmt, ...) {
  va_list ap;
  char msg[200];

  va_start(ap, fmt);
  vsnprintf(msg, sizeof(msg), fmt, ap);
  va_end(ap);

  fprintf(stderr, "%s\n", msg);

  exit(EXIT_FAILURE);
}

/* Copy FS state and set a child in motion. Returns 0 on success. */
static int runchild(pid_t childid)
{
    int rc;
    if ((rc = dfs_put(childid)))
        return rc;
    if (DETERMINE_S_RUNNING !=
            (rc = dput(childid, DETERMINE_START, NULL, 0, NULL)))
        return rc;
    return 0;
}

static int syncchild(pid_t childid, int run)
{
    int out_fd, nread;
    int child_state;
    char buf[512];
    static off_t out_at = 0;
    child_state = dfs_childstate(childid);
    if (child_state < 0)
        return -1;
    if (0 != dfs_get(childid))
        return -2;

    /* Forward __stdout to the console. */
    out_fd = dfs_open("/__stdout", DFS_O_RDWR);
    if (out_fd < 0)
        return -3;
    if (out_at != dfs_seek(out_fd, out_at, DSEEK_SET))
        return -4;
    while (0 < (nread = dfs_read(out_fd, buf, sizeof(buf))))
    {
        write(1, buf, nread);
    }
    out_at = dfs_tell(out_fd);
    dfs_close(1);
    if (1 != dfs_open("/__stdout", DFS_O_APPEND))
        return -5;
    if (0 != dfs_close(out_fd))
        return -6;

    /* Did child exit? */
    if ((DPROC_EXITED | DPROC_FAULTED) & child_state)
        return -7;
    if (DPROC_INPUT & child_state)
    {
        /* Pass input to the child's stdin. */
        int n = read(0, buf, sizeof(buf));
        int fd_stdin = dfs_open("/__stdin", DFS_O_APPEND);
        if (fd_stdin < 0)
            return -9;
        dfs_write(fd_stdin, buf, n);
        dfs_close(fd_stdin);
    }
    dfs_put(childid);
    if (run)
    {
        if (DETERMINE_S_RUNNING != dput(childid, DETERMINE_START, NULL, 0, NULL))
            return -10;
    }
    return 0;
}

// A helper function for traversing comma separated list of values.
// It returns a list pointer shifted to the next value, of NULL if the end
// of the list found.
// Value is stored in val vector. If value has form "x=y", then eq_val
// vector is initialized to point to the "y" part, and val vector length
// is adjusted to point only to "x".
static const char *next_option(const char *list, struct vec *val,
                               struct vec *eq_val) {
  if (list == NULL || *list == '\0') {
    /* End of the list */
    list = NULL;
  } else {
    val->ptr = list;
    if ((list = strchr(val->ptr, ',')) != NULL) {
      /* Comma found. Store length and shift the list ptr */
      val->len = list - val->ptr;
      list++;
    } else {
      /* This value is the last one */
      list = val->ptr + strlen(val->ptr);
      val->len = list - val->ptr;
    }

    if (eq_val != NULL) {
      /*
       * Value has form "x=y", adjust pointers and lengths
       * so that val points to "x", and eq_val points to "y".
       */
      eq_val->len = 0;
      eq_val->ptr = (const char *) memchr(val->ptr, '=', val->len);
      if (eq_val->ptr != NULL) {
        eq_val->ptr++;  /* Skip over '=' character */
        eq_val->len = val->ptr + val->len - eq_val->ptr;
        val->len = (eq_val->ptr - val->ptr) - 1;
      }
    }
  }
  return list;
}

static void close_all_listening_sockets(struct mg_context *ctx) {
  struct socket *sp, *tmp;
  for (sp = ctx->listening_sockets; sp != NULL; sp = tmp) {
    tmp = sp->next;
    (void) closesocket(sp->sock);
    free(sp);
  }
}

// Valid listening port specification is: [ip_address:]port[s|p]
// Examples: 80, 443s, 127.0.0.1:3128p, 1.2.3.4:8080sp
static int parse_port_string(const struct vec *vec, struct socket *so) {
  struct usa *usa = &so->lsa;
  int a, b, c, d, port, len;

  // MacOS needs that. If we do not zero it, subsequent bind() will fail.
  memset(so, 0, sizeof(*so));

  if (sscanf(vec->ptr, "%d.%d.%d.%d:%d%n", &a, &b, &c, &d, &port, &len) == 5) {
    // IP address to bind to is specified
    usa->u.sin.sin_addr.s_addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
  } else if (sscanf(vec->ptr, "%d%n", &port, &len) == 1) {
    // Only port number is specified. Bind to all addresses
    usa->u.sin.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    return 0;
  }
  assert(len > 0 && len <= (int) vec->len);

  if (strchr("sp,", vec->ptr[len]) == NULL) {
    return 0;
  }

  so->is_ssl = vec->ptr[len] == 's';
  so->is_proxy = vec->ptr[len] == 'p';
  usa->len = sizeof(usa->u.sin);
  usa->u.sin.sin_family = AF_INET;
  usa->u.sin.sin_port = htons((uint16_t) port);

  return 1;
}

static void set_close_on_exec(int fd) {
  (void) fcntl(fd, F_SETFD, FD_CLOEXEC);
}

static int set_ports_option(struct mg_context *ctx) {
  const char *list = ctx->config[LISTENING_PORTS];
  int reuseaddr = 1, success = 1;
  SOCKET sock;
  struct vec vec;
  struct socket so, *listener;

  while (success && (list = next_option(list, &vec, NULL)) != NULL) {
    if (!parse_port_string(&vec, &so)) {
      cry(fc(ctx), "%s: %.*s: invalid port spec. Expecting list of: %s",
          __func__, vec.len, vec.ptr, "[IP_ADDRESS:]PORT[s|p]");
      success = 0;
    //} else if (so.is_ssl && ctx->ssl_ctx == NULL) {
    //  cry(fc(ctx), "Cannot add SSL socket, is -ssl_cert option set?");
    //  success = 0;
    } else if ((sock = socket(PF_INET, SOCK_STREAM, 6)) == INVALID_SOCKET ||
               // On Windows, SO_REUSEADDR is recommended only for
               // broadcast UDP sockets
               setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                          sizeof(reuseaddr)) != 0 ||
               bind(sock, &so.lsa.u.sa, so.lsa.len) != 0 ||
               listen(sock, 20) != 0) {
      closesocket(sock);
      cry(fc(ctx), "%s: cannot bind to %.*s: %s", __func__,
          vec.len, vec.ptr, strerror(ERRNO));
      success = 0;
    } else if ((listener = (struct socket *)
                calloc(1, sizeof(*listener))) == NULL) {
      closesocket(sock);
      cry(fc(ctx), "%s: %s", __func__, strerror(ERRNO));
      success = 0;
    } else {
      *listener = so;
      listener->sock = sock;
      set_close_on_exec(listener->sock);
      listener->next = ctx->listening_sockets;
      ctx->listening_sockets = listener;
    }
  }

  if (!success) {
    close_all_listening_sockets(ctx);
  }

  return success;
}

// Verify given socket address against the ACL.
// Return -1 if ACL is malformed, 0 if address is disallowed, 1 if allowed.
static int check_acl(struct mg_context *ctx, const struct usa *usa) {
  int a, b, c, d, n, mask, allowed;
  char flag;
  uint32_t acl_subnet, acl_mask, remote_ip;
  struct vec vec;
  const char *list = ctx->config[ACCESS_CONTROL_LIST];

  if (list == NULL) {
    return 1;
  }

  (void) memcpy(&remote_ip, &usa->u.sin.sin_addr, sizeof(remote_ip));

  // If any ACL is set, deny by default
  allowed = '-';

  while ((list = next_option(list, &vec, NULL)) != NULL) {
    mask = 32;

    if (sscanf(vec.ptr, "%c%d.%d.%d.%d%n", &flag, &a, &b, &c, &d, &n) != 5) {
      cry(fc(ctx), "%s: subnet must be [+|-]x.x.x.x[/x]", __func__);
      return -1;
    } else if (flag != '+' && flag != '-') {
      cry(fc(ctx), "%s: flag must be + or -: [%s]", __func__, vec.ptr);
      return -1;
    } else if (!isbyte(a)||!isbyte(b)||!isbyte(c)||!isbyte(d)) {
      cry(fc(ctx), "%s: bad ip address: [%s]", __func__, vec.ptr);
      return -1;
    } else if (sscanf(vec.ptr + n, "/%d", &mask) == 0) {
      // Do nothing, no mask specified
    } else if (mask < 0 || mask > 32) {
      cry(fc(ctx), "%s: bad subnet mask: %d [%s]", __func__, n, vec.ptr);
      return -1;
    }

    acl_subnet = (a << 24) | (b << 16) | (c << 8) | d;
    acl_mask = mask ? 0xffffffffU << (32 - mask) : 0;

    if (acl_subnet == (ntohl(remote_ip) & acl_mask)) {
      allowed = flag;
    }
  }

  return allowed == '+';
}

/* If tids!=NULL, then this array (which is assumed to be large enough) will be
 * filled with the thread ids of the threads that are synced. */
static int sync_children(struct mg_context *ctx, int towait, pid_t *tids)
{
    /* Wait for the oldest threads to finish. */
    int stopped = 0;
    while (ctx->next_thread > ctx->oldest_thread && towait > 0)
    {
        //dsyncchild(ctx->oldest_thread % NTHREADS); /* TODO check status for fault, */
        int rc = syncchild(ctx->oldest_thread % NTHREADS, 0);
        if (tids)
            tids[stopped] = ctx->oldest_thread % NTHREADS;

        ++ctx->oldest_thread;
        while (ctx->oldest_thread > NTHREADS)
        {
            ctx->oldest_thread -= NTHREADS;
            ctx->next_thread -= NTHREADS;
        }
        ++stopped;
        --towait;
    }
    return stopped;
}


static pid_t get_thread(struct mg_context *ctx)
{
    int thread = -1;
    if (ctx->next_thread - ctx->oldest_thread >= NTHREADS)
    {
        if (1 != sync_children(ctx, 1, NULL))
            return -1;
    }
    thread = ctx->next_thread % NTHREADS;
    ++ctx->next_thread;
    return thread;
}

static void reset_per_request_attributes(struct mg_connection *conn) {
  struct mg_request_info *ri = &conn->request_info;

  // Reset request info attributes. DO NOT TOUCH is_ssl, remote_ip, remote_port
  if (ri->remote_user != NULL) {
    free((void *) ri->remote_user);
  }
  ri->remote_user = ri->request_method = ri->uri = ri->http_version = NULL;
  ri->num_headers = 0;
  ri->status_code = -1;

  conn->num_bytes_sent = conn->consumed_content = 0;
  conn->content_len = -1;
  conn->request_len = conn->data_len = 0;
}

// Check whether full request is buffered. Return:
//   -1  if request is malformed
//    0  if request is not yet fully buffered
//   >0  actual request length, including last \r\n\r\n
static int get_request_len(const char *buf, int buflen) {
  const char *s, *e;
  int len = 0;

  DEBUG_TRACE(("buf: %p, len: %d", buf, buflen));
  for (s = buf, e = s + buflen - 1; len <= 0 && s < e; s++)
    // Control characters are not allowed but >=128 is.
    if (!isprint(* (const unsigned char *) s) && *s != '\r' &&
        *s != '\n' && * (const unsigned char *) s < 128) {
      len = -1;
    } else if (s[0] == '\n' && s[1] == '\n') {
      len = (int) (s - buf) + 2;
    } else if (s[0] == '\n' && &s[1] < e &&
        s[1] == '\r' && s[2] == '\n') {
      len = (int) (s - buf) + 3;
    }

  return len;
}

// Read from IO channel - opened file descriptor, socket, or SSL descriptor.
// Return number of bytes read.
static int pull(FILE *fp, SOCKET sock, SSL *ssl, char *buf, int len) {
  int nread;

  if (ssl != NULL) {
    die("SSL Not implemented");
    //nread = SSL_read(ssl, buf, len);
  } else if (fp != NULL) {
    // Use read() instead of fread(), because if we're reading from the CGI
    // pipe, fread() may block until IO buffer is filled up. We cannot afford
    // to block and must pass all read bytes immediately to the client.
    nread = read(fileno(fp), buf, (size_t) len);
    if (ferror(fp))
      nread = -1;
  } else {
    nread = recv(sock, buf, (size_t) len, 0);
  }

  return nread;
}

// Keep reading the input (either opened file descriptor fd, or socket sock,
// or SSL descriptor ssl) into buffer buf, until \r\n\r\n appears in the
// buffer (which marks the end of HTTP request). Buffer buf may already
// have some data. The length of the data is stored in nread.
// Upon every read operation, increase nread by the number of bytes read.
static int read_request(FILE *fp, SOCKET sock, SSL *ssl, char *buf, int bufsiz,
                        int *nread) {
  int n, request_len;

  request_len = 0;
  while (*nread < bufsiz && request_len == 0) {
    n = pull(fp, sock, ssl, buf + *nread, bufsiz - *nread);
    if (n <= 0) {
      break;
    } else {
      *nread += n;
      request_len = get_request_len(buf, *nread);
    }
  }

  return request_len;
}

static int set_non_blocking_mode(SOCKET sock) {
  int flags;

  flags = fcntl(sock, F_GETFL, 0);
  (void) fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  return 0;
}

static void close_socket_gracefully(SOCKET sock) {
  char buf[BUFSIZ];
  int n;

  // Send FIN to the client
  (void) shutdown(sock, SHUT_WR);
  set_non_blocking_mode(sock);

  // Read and discard pending data. If we do not do that and close the
  // socket, the data in the send buffer may be discarded. This
  // behaviour is seen on Windows, when client keeps sending data
  // when server decide to close the connection; then when client
  // does recv() it gets no data back.
  do {
    n = pull(NULL, sock, NULL, buf, sizeof(buf));
  } while (n > 0);

  // Now we know that our FIN is ACK-ed, safe to close
  (void) closesocket(sock);
}

static void close_connection(struct mg_connection *conn) {
  /*if (conn->ssl) {
    SSL_free(conn->ssl);
    conn->ssl = NULL;
  }*/

  if (conn->client.sock != INVALID_SOCKET) {
    close_socket_gracefully(conn->client.sock);
  }
}

// Write data to the IO channel - opened file descriptor, socket or SSL
// descriptor. Return number of bytes written.
static int64_t push(FILE *fp, SOCKET sock, SSL *ssl, const char *buf,
                    int64_t len) {
  int64_t sent;
  int n, k;

  sent = 0;
  while (sent < len) {

    /* How many bytes we send in this iteration */
    k = len - sent > INT_MAX ? INT_MAX : (int) (len - sent);

    /*if (ssl != NULL) {
      n = SSL_write(ssl, buf + sent, k);
    } else*/ if (fp != NULL) {
      n = fwrite(buf + sent, 1, (size_t)k, fp);
      if (ferror(fp))
        n = -1;
    } else {
      n = send(sock, buf + sent, (size_t)k, 0);
    }

    if (n < 0)
      break;

    sent += n;
  }

  return sent;
}

int mg_write(struct mg_connection *conn, const void *buf, size_t len) {
  return (int) push(NULL, conn->client.sock, /*conn->ssl,*/NULL,
      (const char *) buf, (int64_t) len);
}

// Return HTTP header value, or NULL if not found.
static const char *get_header(const struct mg_request_info *ri,
                              const char *name) {
  int i;

  for (i = 0; i < ri->num_headers; i++)
    if (!mg_strcasecmp(name, ri->http_headers[i].name))
      return ri->http_headers[i].value;

  return NULL;
}

const char *mg_get_header(const struct mg_connection *conn, const char *name) {
  return get_header(&conn->request_info, name);
}

static void *call_user(struct mg_connection *conn, enum mg_event event)
{
    return NULL;
}

// HTTP 1.1 assumes keep alive if "Connection:" header is not set
// This function must tolerate situations when connection info is not
// set up, for example if request parsing failed.
static int should_keep_alive(const struct mg_connection *conn) {
  const char *http_version = conn->request_info.http_version;
  const char *header = mg_get_header(conn, "Connection");
  return 0;
  //return (header == NULL && http_version && !strcmp(http_version, "1.1")) ||
  //    (header != NULL && !mg_strcasecmp(header, "keep-alive"));
}

static void process_new_connection(struct mg_connection *conn) {
  struct mg_context *ctx = conn->ctx;
  struct mg_request_info *ri = &conn->request_info;
  int keep_alive_enabled;
  const char *cl;
  int fd, ret;
  size_t sz;

  //keep_alive_enabled = !strcmp(conn->ctx->config[ENABLE_KEEP_ALIVE], "yes");
  keep_alive_enabled = 0;

  //do {
    reset_per_request_attributes(conn);

    // If next request is not pipelined, read it in
    if ((conn->request_len = get_request_len(conn->buf, conn->data_len)) == 0) {
      conn->request_len = read_request(NULL, conn->client.sock,
              /*conn->ssl*/ NULL, conn->buf, conn->buf_size, &conn->data_len);
    }
    assert(conn->data_len >= conn->request_len);

    sz = deterministic_mg_conn_size(conn);
    DEBUG_TRACE(("SZ=%d %08lx %d",sz,ctx,conn->tid));
    /* Now, send the request off to the child. */
    fd = dfs_open(ctx->threads[conn->tid].fin, DFS_O_WRONLY);
    if (fd < 0)
    {
        die("Open failed (%s, rc=%d)\n", ctx->threads[conn->tid].fin, fd);
        return;
    }
    dfs_write(fd, &sz, sizeof(int));
    if (sz != (ret = dfs_write(fd, deterministic_mg_conn(conn), sz)))
    {
        dfs_close(fd);
        die("Failed to write entire connection sequence to child. "
                "Wrote %d / %d bytes.", ret, sz);
    }
    dfs_close(fd);
    /* Set child in motion. */
    if ((ret = runchild(conn->tid)))
        die("Failed to run child thread rc=%d\n", ret);
    DEBUG_TRACE(("Started child worker tid=%d", conn->tid));
    ctx->threads[conn->tid].state = THREAD_RUNNING;

    /*
    if (conn->request_len == 0 && conn->data_len == conn->buf_size) {
        die("no send_http_error");
      send_http_error(conn, 413, "Request Too Large", "");
      return;
    } if (conn->request_len <= 0) {
      return;  // Remote end closed the connection
    }

    // Nul-terminate the request cause parse_http_request() uses sscanf
    conn->buf[conn->request_len - 1] = '\0';
    if (!parse_http_request(conn->buf, ri) ||
        (!conn->client.is_proxy && !is_valid_uri(ri->uri))) {
      // Do not put garbage in the access log, just send it back to the client
      send_http_error(conn, 400, "Bad Request",
          "Cannot parse HTTP request: [%.*s]", conn->data_len, conn->buf);
    } else if (strcmp(ri->http_version, "1.0") &&
               strcmp(ri->http_version, "1.1")) {
      // Request seems valid, but HTTP version is strange
      send_http_error(conn, 505, "HTTP version not supported", "");
      log_access(conn);
    } else {
      // Request is valid, handle it
      cl = get_header(ri, "Content-Length");
      conn->content_len = cl == NULL ? -1 : strtoll(cl, NULL, 10);
      conn->birth_time = time(NULL);
      if (conn->client.is_proxy) {
        handle_proxy_request(conn);
      } else {
        handle_request(conn);
      }
      log_access(conn);
      discard_current_request_from_buffer(conn);
    }
    */
    // conn->peer is not NULL only for SSL-ed proxy connections
  //} while (conn->peer || (keep_alive_enabled && should_keep_alive(conn)));
}

static void produce_socket(struct mg_context *ctx, const struct socket *sp) {
    //(void) pthread_mutex_lock(&ctx->mutex);

    // If the queue is full, wait
    /*while (ctx->sq_head - ctx->sq_tail >= (int) ARRAY_SIZE(ctx->queue)) {
        (void) pthread_cond_wait(&ctx->sq_empty, &ctx->mutex);
    }
    assert(ctx->sq_head - ctx->sq_tail < (int) ARRAY_SIZE(ctx->queue));
    */

    // Copy socket to the queue and increment head
    //ctx->queue[ctx->sq_head % ARRAY_SIZE(ctx->queue)] = *sp;
    //ctx->sq_head++;
    /* Assign socket to thread if one is available. Otherwise, wait. */
    struct mg_connection *conn;
    int buf_size = atoi(ctx->config[MAX_REQUEST_SIZE]);
    pid_t tid = get_thread(ctx);
    DEBUG_TRACE(("get_thread got %d\n", tid));
    if (tid < 0)
        /* TODO wait for other children incase they were processing requests. */
        die("Unable to find available thread.");

    /* Read in the socket data, then pass that to the child worker. */
    conn = calloc(1, sizeof(*conn) + buf_size);
    ctx->threads[tid].conn = conn;
    conn->ctx = ctx;
    conn->tid = tid;
    conn->buf_size = buf_size;
    conn->birth_time = time(NULL);
    conn->client = *sp;
    //conn->ctx = ctx;
    conn->request_info.remote_port = ntohs(conn->client.rsa.u.sin.sin_port);
    memcpy(&conn->request_info.remote_ip,
           &conn->client.rsa.u.sin.sin_addr.s_addr, 4);
    conn->request_info.remote_ip = ntohl(conn->request_info.remote_ip);
    conn->request_info.is_ssl = conn->client.is_ssl;

    //if (!conn->client.is_ssl ||
    //    (conn->client.is_ssl && sslize(conn, SSL_accept))) {
      process_new_connection(conn);
    //}

    //close_connection(conn);

    //DEBUG_TRACE(("queued socket %d", sp->sock));

    //(void) pthread_cond_signal(&ctx->sq_full);
    //(void) pthread_mutex_unlock(&ctx->mutex);
}

static void add_to_set(SOCKET fd, fd_set *set, int *max_fd) {
    FD_SET(fd, set);
    if (fd > (SOCKET) *max_fd) {
        *max_fd = (int) fd;
    }
}

static void accept_new_connection(const struct socket *listener,
        struct mg_context *ctx) {
    struct socket accepted;
    int allowed;

    accepted.rsa.len = sizeof(accepted.rsa.u.sin);
    accepted.lsa = listener->lsa;
    accepted.sock = accept(listener->sock, &accepted.rsa.u.sa, &accepted.rsa.len);
    if (accepted.sock != INVALID_SOCKET) {
        allowed = check_acl(ctx, &accepted.rsa);
        if (allowed) {
            // Put accepted socket structure into the queue
            DEBUG_TRACE(("accepted socket %d", accepted.sock));
            accepted.is_ssl = listener->is_ssl;
            accepted.is_proxy = listener->is_proxy;
            produce_socket(ctx, &accepted);
        } else {
            cry(fc(ctx), "%s: %s is not allowed to connect",
                    __func__, inet_ntoa(accepted.rsa.u.sin.sin_addr));
            (void) closesocket(accepted.sock);
        }
    }
}

char outbuf[0x10000];
static void mg_master(struct mg_context *ctx)
{

    int i;
    for (i = 0; i < NTHREADS; ++i)
    {
        ctx->threads[i].state = THREAD_WAITING;
        snprintf(ctx->threads[i].fin, sizeof(ctx->threads[i].fin),
                "/threads/%d_in", i);
        snprintf(ctx->threads[i].fout, sizeof(ctx->threads[i].fout),
                "/threads/%d_out", i);
    }
    while (ctx->stop_flag == 0) {
        fd_set read_set;
        struct timeval tv;
        struct socket *sp;
        int max_fd = -1, synced;
        pid_t tid;

        FD_ZERO(&read_set);
        // Add listening sockets to the read set
        for (sp = ctx->listening_sockets; sp != NULL; sp = sp->next) {
            add_to_set(sp->sock, &read_set, &max_fd);
        }

        tv.tv_sec = 0;
        tv.tv_usec = 200 * 1000;

        if (select(max_fd + 1, &read_set, NULL, NULL, &tv) < 0) {
        } else {
            for (sp = ctx->listening_sockets; sp != NULL; sp = sp->next) {
                if (FD_ISSET(sp->sock, &read_set)) {
                    accept_new_connection(sp, ctx);
                }
            }
        }
        /* Check for finished threads. */
        while ((synced = sync_children(ctx, 1, &tid) > 0))
        {
            struct mg_connection *conn = ctx->threads[tid].conn;
            int fd;
            DEBUG_TRACE(("Synced child %d\n", tid));
            fd = dfs_open(ctx->threads[tid].fout, DFS_O_RDONLY);
            if (fd >= 0)
            {
                int nread = 0;
                dfs_seek(fd, 0, SEEK_SET);
                while ((nread = dfs_read(fd, outbuf, sizeof(outbuf))) > 0)
                {
                    DEBUG_TRACE(("wrote %d bytes",nread));
                    mg_write(conn, outbuf, nread);
                }
                dfs_close(fd);
            }
            close_connection(conn);
            free(conn);
        }
    }
    DEBUG_TRACE(("stopping workers"));

    // Stop signal received: somebody called mg_stop. Quit.
    close_all_listening_sockets(ctx);

    // Wakeup workers that are waiting for connections to handle.
    //pthread_cond_broadcast(&ctx->sq_full);

    // Wait until all threads finish
    //(void) pthread_mutex_lock(&ctx->mutex);
    // while (ctx->num_threads > 0) {
    //     (void) pthread_cond_wait(&ctx->cond, &ctx->mutex);
    // }
    //(void) pthread_mutex_unlock(&ctx->mutex);

    // All threads exited, no sync is needed. Destroy mutex and condvars
    //(void) pthread_mutex_destroy(&ctx->mutex);
    //(void) pthread_cond_destroy(&ctx->cond);
    //(void) pthread_cond_destroy(&ctx->sq_empty);
    //(void) pthread_cond_destroy(&ctx->sq_full);

    // Signal mg_stop() that we're done
    ctx->stop_flag = 2;

    DEBUG_TRACE(("exiting"));
}

int main(int argc, char**argv)
{
    int i, fd;
    assert(0 == become_deterministic());
    assert(0 == dfs_init_clean());
    assert(0 == _init_dfs());
    assert(0 == _init_io());
    /* Create file for each worker to use to transfer data
       between parent and child. */
    if ((fd = dfs_mkdir("/threads")))
    {
        fprintf(stderr, "Could not create DFS /threads (rc=%d)\n", fd);
        exit(EXIT_FAILURE);
    }
    dfs_close(fd);
    for (i = 0; i < 10; ++i)
    {
        char buf[20];
        snprintf(buf, sizeof(buf), "/threads/%d_in", i);
        fd = dfs_open(buf, DFS_O_CREAT);
        if (fd < 0)
        {
            fprintf(stderr, "Could not create file %s (rc=%d)\n", buf, fd);
            exit(EXIT_FAILURE);
        }
        dfs_close(fd);
        snprintf(buf, sizeof(buf), "/threads/%d_out", i);
        fd = dfs_open(buf, DFS_O_CREAT);
        if (fd < 0)
        {
            fprintf(stderr, "Could not create file %s (rc=%d)\n", buf, fd);
            exit(EXIT_FAILURE);
        }
        dfs_close(fd);
    }
    for (i = 0; i < 10; ++i)
    {
        if (!dfork(i, DETERMINE_START))
        {
            char buf[5];
            snprintf(buf, sizeof(buf), "%d", i);
            execl("root/ws_worker", "root/ws_worker", buf, NULL);
        }
        dsyncchild(i); /* Child waits for FS. */
        assert(0 == dfs_put(i));
        dput(i, DETERMINE_START, NULL, 0, NULL); /* Child sets up basic data structures. */
        syncchild(i, 0);
    }

    init_server_name();
    g_ctx = start_mongoose(argc, argv);
    printf("%s started on port(s) %s with web root [%s]\n",
            server_name, mg_get_option(g_ctx, "listening_ports"),
            mg_get_option(g_ctx, "document_root"));
    mg_master(g_ctx);
    mg_stop(g_ctx);
#if 0
    while (1)
    {
        for (i = 0; i < 10; ++i)
        {
            int rc, fd, dlen;
            char buf[15], data[100];
            snprintf(buf, sizeof(buf), "/threads/%d", i);
            fd = dfs_open(buf, DFS_O_WRONLY);
            if (fd < 0)
            {
                fprintf(stderr, "Could not open DFS %s (rc=%d)\n", buf, fd);
                exit(EXIT_FAILURE);
            }
            dlen = snprintf(data, sizeof(data), "Your data is %.5f\n",
                    (float)(clock()) / CLOCKS_PER_SEC);
            dfs_seek(fd, 0, DSEEK_SET);
            dfs_write(fd, data, dlen);
            dfs_close(fd);
            if ((rc = syncchild(i)))
            {
                printf("Rc=%d, child=%d\n",rc,i);
                goto end;
            }
        }
    }
#endif
end:
    /* Kill all children. */
    for (i = 0; i < 10; ++i)
        dkill(i);

    printf("Parent finished.\n");
    return 0;
}

static void show_usage_and_exit(void) {
  const char **names;
  int i;

  fprintf(stderr, "Mongoose version %s (c) Sergey Lyubka\n", mg_version());
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  mongoose -A <htpasswd_file> <realm> <user> <passwd>\n");
  fprintf(stderr, "  mongoose <config_file>\n");
  fprintf(stderr, "  mongoose [-option value ...]\n");
  fprintf(stderr, "OPTIONS:\n");

  names = mg_get_valid_option_names();
  for (i = 0; names[i] != NULL; i += 3) {
    fprintf(stderr, "  -%s %s (default: \"%s\")\n",
            names[i], names[i + 1], names[i + 2] == NULL ? "" : names[i + 2]);
  }
  fprintf(stderr, "See  http://code.google.com/p/mongoose/wiki/MongooseManual"
          " for more details.\n");
  fprintf(stderr, "Example:\n  mongoose -s cert.pem -p 80,443s -d no\n");
  exit(EXIT_FAILURE);
}

static void free_context(struct mg_context *ctx) {
  int i;

  // Deallocate config parameters
  for (i = 0; i < NUM_OPTIONS; i++) {
    if (ctx->config[i] != NULL)
      free(ctx->config[i]);
  }

  // Deallocate SSL context
  //if (ctx->ssl_ctx != NULL) {
  //  SSL_CTX_free(ctx->ssl_ctx);
  //}
#ifndef NO_SSL
  //if (ssl_mutexes != NULL) {
  //  free(ssl_mutexes);
  //}
#endif // !NO_SSL

  // Deallocate context itself
  free(ctx);
}

void mg_stop(struct mg_context *ctx) {
  ctx->stop_flag = 1;

  // Wait until mg_fini() stops
  //while (ctx->stop_flag != 2) {
  //  (void) sleep(0);
  //}
  free_context(ctx);

}

struct mg_context *mg_start(mg_callback_t user_callback, void *user_data,
                            const char **options) {
  struct mg_context *ctx;
  const char *name, *value, *default_value;
  int i;

  // Allocate context and initialize reasonable general case defaults.
  // TODO(lsm): do proper error handling here.
  ctx = (struct mg_context *) calloc(1, sizeof(*ctx));
  ctx->user_callback = user_callback;
  ctx->user_data = user_data;

  while (options && (name = *options++) != NULL) {
    if ((i = get_option_index(name)) == -1) {
      cry(fc(ctx), "Invalid option: %s", name);
      free_context(ctx);
      return NULL;
    } else if ((value = *options++) == NULL) {
      cry(fc(ctx), "%s: option value cannot be NULL", name);
      free_context(ctx);
      return NULL;
    }
    ctx->config[i] = mg_strdup(value);
    DEBUG_TRACE(("[%s] -> [%s]", name, value));
  }

  // Set default value if needed
  for (i = 0; config_options[i * ENTRIES_PER_CONFIG_OPTION] != NULL; i++) {
    default_value = config_options[i * ENTRIES_PER_CONFIG_OPTION + 2];
    if (ctx->config[i] == NULL && default_value != NULL) {
      ctx->config[i] = mg_strdup(default_value);
      DEBUG_TRACE(("Setting default: [%s] -> [%s]",
                   config_options[i * ENTRIES_PER_CONFIG_OPTION + 1],
                   default_value));
    }
  }

  /*
  // NOTE(lsm): order is important here. SSL certificates must
  // be initialized before listening ports. UID must be set last.
  if (!set_gpass_option(ctx) ||
#if !defined(NO_SSL)
      !set_ssl_option(ctx) ||
#endif
      !set_ports_option(ctx) ||
      !set_uid_option(ctx) ||
      !set_acl_option(ctx)) {
    free_context(ctx);
    return NULL;
  }
*/
  if (!set_ports_option(ctx))
  {
      free_context(ctx);
      return NULL;
  }

  // Ignore SIGPIPE signal, so if browser cancels the request, it
  // won't kill the whole process.
  (void) signal(SIGPIPE, SIG_IGN);

  //(void) pthread_mutex_init(&ctx->mutex, NULL);
  //(void) pthread_cond_init(&ctx->cond, NULL);
  //(void) pthread_cond_init(&ctx->sq_empty, NULL);
  //(void) pthread_cond_init(&ctx->sq_full, NULL);

  // Start master (listening) thread
  //start_thread(ctx,struct mc_context *thread_func_t) master_thread, ctx);

  // Start worker threads
  /*for (i = 0; i < atoi(ctx->config[NUM_THREADS]); i++) {
    if (start_thread(ctx, (mg_thread_func_t) worker_thread, ctx) != 0) {
      cry(fc(ctx), "Cannot start worker thread: %d", ERRNO);
    } else {
      ctx->num_threads++;
    }
  }*/

  return ctx;
}

static void verify_document_root(const char *root) {
  const char *p, *path;
  char buf[PATH_MAX];
  struct stat st;

  path = root;
  if ((p = strchr(root, ',')) != NULL && (size_t) (p - root) < sizeof(buf)) {
    memcpy(buf, root, p - root);
    buf[p - root] = '\0';
    path = buf;
  }

  if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode)) {
    die("Invalid root directory: [%s]: %s", root, strerror(errno));
  }
}

static char *sdup(const char *str) {
  char *p;
  if ((p = (char *) malloc(strlen(str) + 1)) != NULL) {
    strcpy(p, str);
  }
  return p;
}

static void set_option(char **options, const char *name, const char *value) {
  int i;

  if (!strcmp(name, "document_root") || !(strcmp(name, "r"))) {
    verify_document_root(value);
  }

  for (i = 0; i < MAX_OPTIONS - 3; i++) {
    if (options[i] == NULL) {
      options[i] = sdup(name);
      options[i + 1] = sdup(value);
      options[i + 2] = NULL;
      break;
    }
  }

  if (i == MAX_OPTIONS - 3) {
    die("%s", "Too many options specified");
  }
}
static void process_command_line_arguments(char *argv[], char **options) {
  char line[512], opt[512], val[512], *p;
  FILE *fp = NULL;
  size_t i, line_no = 0;

  options[0] = NULL;

  // Should we use a config file ?
  if (argv[1] != NULL && argv[2] == NULL) {
    snprintf(config_file, sizeof(config_file), "%s", argv[1]);
  } else if ((p = strrchr(argv[0], DIRSEP)) == NULL) {
    // No command line flags specified. Look where binary lives
    snprintf(config_file, sizeof(config_file), "%s", CONFIG_FILE);
  } else {
    snprintf(config_file, sizeof(config_file), "%.*s%c%s",
             (int) (p - argv[0]), argv[0], DIRSEP, CONFIG_FILE);
  }

  fp = fopen(config_file, "r");

  // If config file was set in command line and open failed, exit
  if (argv[1] != NULL && argv[2] == NULL && fp == NULL) {
    die("Cannot open config file %s: %s", config_file, strerror(errno));
  }

  // Load config file settings first
  if (fp != NULL) {
    fprintf(stderr, "Loading config file %s\n", config_file);

    // Loop over the lines in config file
    while (fgets(line, sizeof(line), fp) != NULL) {

      line_no++;

      // Ignore empty lines and comments
      if (line[0] == '#' || line[0] == '\n')
        continue;

      if (sscanf(line, "%s %[^\r\n#]", opt, val) != 2) {
        die("%s: line %d is invalid", config_file, (int) line_no);
      }
      set_option(options, opt, val);
    }

    (void) fclose(fp);
  }

  // Now handle command line flags. They override config file settings.
  for (i = 1; argv[i] != NULL; i += 2) {
    if (argv[i][0] != '-' || argv[i + 1] == NULL) {
      show_usage_and_exit();
    }
    set_option(options, &argv[i][1], argv[i + 1]);
  }
}

static struct mg_context *start_mongoose(int argc, char *argv[]) {
  char *options[MAX_OPTIONS];
  int i;

  /* Edit passwords file if -A option is specified */
  if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'A') {
    if (argc != 6) {
      show_usage_and_exit();
    }
    // exit(mg_modify_passwords_file(argv[2], argv[3], argv[4], argv[5]) ?
    //     EXIT_SUCCESS : EXIT_FAILURE);
    TODO;
  }

  /* Show usage if -h or --help options are specified */
  if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
    show_usage_and_exit();
  }

  /* Update config based on command line arguments */
  process_command_line_arguments(argv, options);

  /* Setup signal handler: quit on Ctrl-C */
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  //TODO

  /* Start Mongoose */
  struct mg_context *ctx = mg_start(NULL, NULL, (const char **) options);
  for (i = 0; options[i] != NULL; i++) {
    free(options[i]);
  }

  if (ctx == NULL) {
      TODO;
    //die("%s", "Failed to start Mongoose. Maybe some options are "
    //    "assigned bad values?\nTry to run with '-e error_log.txt' "
    //    "and check error_log.txt for more information.");
  }
  return ctx;
}

