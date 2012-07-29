
#include <inc/ctype.h>
#include <inc/determinism_pure.h>
#include <inc/fs.h>
#include <inc/assert.h>
#include <inc/io.h>
#include <inc/mman.h>
#include <inc/syscall.h>

#define BUFSIZ 0x1000

#define PASSWORDS_FILE_NAME ".htpasswd"
#define DEBUG_TRACE(x) do { \
    printf("**** %s.%d: ", __func__, __LINE__); \
    printf x; printf("\n"); } while (0)
#define cry(...)

typedef long int time_t; /* This is not portable! */

// Various events on which user-defined function is called by Mongoose.
enum mg_event {
  MG_NEW_REQUEST,   // New HTTP request has arrived from the client
  MG_HTTP_ERROR,    // HTTP error must be returned to the client
  MG_EVENT_LOG,     // Mongoose logs an event, request_info.log_message
  MG_INIT_SSL       // Mongoose initializes SSL. Instead of mg_connection *,
                    // SSL context is passed to the callback function.
};

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

// Structure used by mg_stat() function. Uses 64 bit file length.
struct mgstat {
  int is_directory;  // Directory marker
  int64_t size;      // File size
  time_t mtime;      // Modification time
};

struct vec
{
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

struct packet_info
{
    int size; /* <= 0 indicates some error, > 0 is the size of the packet data
                 including this field. */
    struct mg_connection {
        struct mg_connection *peer; // Remote target in proxy mode
        struct mg_request_info request_info;
        //SSL *ssl;                   // SSL descriptor
        //struct socket client;       // Connected client
        time_t birth_time;          // Time connection was accepted
        int64_t num_bytes_sent;     // Total bytes sent to client
        int64_t content_len;        // Content-Length header value
        int64_t consumed_content;   // How many bytes of content is already read
        int request_len;            // Size of the request + headers in a buffer
        int data_len;               // Total size of data in a buffer
        int buf_size;               // Buffer size
        char buf[0]; /* Bytes from here on out are the packet data. */
    } conn;
};

pid_t id;
char input_file[20];
char output_file[20];
int outfd;
/* Sync with parent (wait for more data, flush print buffers, etc). */
static void sync(void)
{
    flush_printf_buffer();
    dret();
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

// Mongoose allows to specify multiple directories to serve,
// like /var/www,/~bob=/home/bob. That means that root directory depends on URI.
// This function returns root dir for given URI.
static int get_document_root(const struct mg_connection *conn,
                             struct vec *document_root) {
  const char *root, *uri;
  int len_of_matched_uri;
  struct vec uri_vec, path_vec;

  uri = conn->request_info.uri;
  len_of_matched_uri = 0;
  /* TODO root = next_option(conn->ctx->config[DOCUMENT_ROOT], document_root, NULL);

  while ((root = next_option(root, &uri_vec, &path_vec)) != NULL) {
    if (memcmp(uri, uri_vec.ptr, uri_vec.len) == 0) {
      *document_root = path_vec;
      len_of_matched_uri = uri_vec.len;
      break;
    }
  }*/

  //return len_of_matched_uri;
  document_root->ptr = "/www";
  document_root->len = 4;
  return 4;
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
    //cry(conn, "vsnprintf error");
    n = 0;
  } else if (n >= (int) buflen) {
    //cry(conn, "truncating vsnprintf buffer: [%.*s]",
    //    n > 200 ? 200 : n, buf);
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

int mg_write(struct mg_connection *conn, const void *buf, size_t len) {
  return dfs_write(outfd, buf, len);
  //return (int) push(NULL, conn->client.sock, conn->ssl,
  //    (const char *) buf, (int64_t) len);
}

int mg_printf(struct mg_connection *conn, const char *fmt, ...) {
  char buf[BUFSIZ];
  int len;
  va_list ap;

  va_start(ap, fmt);
  len = mg_vsnprintf(conn, buf, sizeof(buf), fmt, ap);
  va_end(ap);

  return mg_write(conn, buf, (size_t)len);
}

static void *call_user(struct mg_connection *conn, enum mg_event event)
{
    return NULL;
}

// HTTP 1.1 assumes keep alive if "Connection:" header is not set
// This function must tolerate situations when connection info is not
// set up, for example if request parsing failed.
static int should_keep_alive(const struct mg_connection *conn) {
  /*const char *http_version = conn->request_info.http_version;
  const char *header = mg_get_header(conn, "Connection");
  return (header == NULL && http_version && !strcmp(http_version, "1.1")) ||
      (header != NULL && !mg_strcasecmp(header, "keep-alive"));
      */
  return 0;
}

static const char *suggest_connection_header(const struct mg_connection *conn) {
  return should_keep_alive(conn) ? "keep-alive" : "close";
}

static void send_http_error(struct mg_connection *conn, int status,
                            const char *reason, const char *fmt, ...) {
  char buf[BUFSIZ];
  va_list ap;
  int len;

  conn->request_info.status_code = status;

  if (call_user(conn, MG_HTTP_ERROR) == NULL) {
    buf[0] = '\0';
    len = 0;

    /* Errors 1xx, 204 and 304 MUST NOT send a body */
    if (status > 199 && status != 204 && status != 304) {
      len = mg_snprintf(conn, buf, sizeof(buf), "Error %d: %s", status, reason);
      //cry(conn, "%s", buf);
      buf[len++] = '\n';

      va_start(ap, fmt);
      len += mg_vsnprintf(conn, buf + len, sizeof(buf) - len, fmt, ap);
      va_end(ap);
    }
    DEBUG_TRACE(("[%s]", buf));

    mg_printf(conn, "HTTP/1.1 %d %s\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %d\r\n"
              "Connection: %s\r\n\r\n", status, reason, len,
              suggest_connection_header(conn));
    conn->num_bytes_sent += mg_printf(conn, "%s", buf);
  }
}

static void convert_uri_to_file_name(struct mg_connection *conn,
                                     const char *uri, char *buf,
                                     size_t buf_len) {
  struct vec vec;
  int match_len;

  match_len = get_document_root(conn, &vec);
  mg_snprintf(conn, buf, buf_len, "%.*s%s", vec.len, vec.ptr, uri + match_len);

  DEBUG_TRACE(("[%s] -> [%s], [%.*s]", uri, buf, (int) vec.len, vec.ptr));
}

static int mg_stat(const char *path, struct mgstat *stp) {
  struct stat st;
  int ok;

  if (stat(path, &st) == 0) {
    ok = 0;
    stp->size = st.st_size;
    stp->mtime = st.st_mtime;
    stp->is_directory = S_ISDIR(st.st_mode);
  } else {
    ok = -1;
  }

  return ok;
}

// Authorize against the opened passwords file. Return 1 if authorized.
static int authorize(struct mg_connection *conn, int fd) {
  struct ah ah;
  char line[256], f_user[256], ha1[256], f_domain[256], buf[BUFSIZ];

  /*if (!parse_auth_header(conn, buf, sizeof(buf), &ah)) {
    return 0;
  }

  // Loop over passwords file
  while (fgets(line, sizeof(line)) != NULL) {
    if (sscanf(line, "%[^:]:%[^:]:%s", f_user, f_domain, ha1) != 3) {
      continue;
    }

    if (!strcmp(ah.user, f_user) &&
        !strcmp(conn->ctx->config[AUTHENTICATION_DOMAIN], f_domain))
      return check_password(
            conn->request_info.request_method,
            ha1, ah.uri, ah.nonce, ah.nc, ah.cnonce, ah.qop,
            ah.response);
  }*/

  return 0;
}

// Return 1 if request is authorised, 0 otherwise.
static int check_authorization(struct mg_connection *conn, const char *path) {
  char fname[PATH_MAX];
  struct vec uri_vec, filename_vec;
  const char *list;
  int authorized, fd = -1;

  authorized = 1;

  //TODO config list = conn->ctx->config[PROTECT_URI];
  while ((list = next_option(list, &uri_vec, &filename_vec)) != NULL) {
    if (!memcmp(conn->request_info.uri, uri_vec.ptr, uri_vec.len)) {
      (void) mg_snprintf(conn, fname, sizeof(fname), "%.*s",
          filename_vec.len, filename_vec.ptr);
      if ((fd = dfs_open(fname, DFS_O_RDONLY)) < 0) {
        cry(conn, "%s: cannot open %s: fd", __func__, fname, fd ;
      }
      break;
    }
  }

  if (fd < 0) {
    //TODO
    //fd = open_auth_file(conn, path);
  }

  if (fd >= 0) {
    authorized = authorize(conn, fd);
    dfs_close(fd);
  }

  return authorized;
}

// URL-decode input buffer into destination buffer.
// 0-terminate the destination buffer. Return the length of decoded data.
// form-url-encoded data differs from URI encoding in a way that it
// uses '+' as character for space, see RFC 1866 section 8.2.1
// http://ftp.ics.uci.edu/pub/ietf/html/rfc1866.txt
static size_t url_decode(const char *src, size_t src_len, char *dst,
                         size_t dst_len, int is_form_url_encoded) {
  size_t i, j;
  int a, b;
#define HEXTOI(x) (isdigit(x) ? x - '0' : x - 'W')

  for (i = j = 0; i < src_len && j < dst_len - 1; i++, j++) {
    if (src[i] == '%' &&
        isxdigit(* (const unsigned char *) (src + i + 1)) &&
        isxdigit(* (const unsigned char *) (src + i + 2))) {
      a = tolower(* (const unsigned char *) (src + i + 1));
      b = tolower(* (const unsigned char *) (src + i + 2));
      dst[j] = (char) ((HEXTOI(a) << 4) | HEXTOI(b));
      i += 2;
    } else if (is_form_url_encoded && src[i] == '+') {
      dst[j] = ' ';
    } else {
      dst[j] = src[i];
    }
  }

  dst[j] = '\0'; /* Null-terminate the destination */

  return j;
}

// This is the heart of the Mongoose's logic.
// This function is called when the request is read, parsed and validated,
// and Mongoose must decide what action to take: serve a file, or
// a directory, or call embedded function, etcetera.
struct packet_info *packet;
static int process_request(void)
{
    struct mg_connection *conn = &packet->conn;
    struct mg_request_info *ri = &conn->request_info;
    char path[PATH_MAX];
    int uri_len;
    struct mgstat st;
    outfd = dfs_open(output_file, DFS_O_WRONLY);
    if (outfd < 0)
        return outfd;

    if ((conn->request_info.query_string = strchr(ri->uri, '?')) != NULL) {
        *conn->request_info.query_string++ = '\0';
    }
    uri_len = strlen(ri->uri);
    (void) url_decode(ri->uri, (size_t)uri_len, ri->uri, (size_t)(uri_len + 1), 0);
    remove_double_dots_and_double_slashes(ri->uri);
    convert_uri_to_file_name(conn, ri->uri, path, sizeof(path));

    DEBUG_TRACE(("%s", ri->uri));
    if (!check_authorization(conn, path)) {
        send_authorization_request(conn);
    } else if (call_user(conn, MG_NEW_REQUEST) != NULL) {
        // Do nothing, callback has served the request
    } else if (strstr(path, PASSWORDS_FILE_NAME)) {
        // Do not allow to view passwords files
        send_http_error(conn, 403, "Forbidden", "Access Forbidden");
    } else if (conn->ctx->config[DOCUMENT_ROOT] == NULL) {
        send_http_error(conn, 404, "Not Found", "Not Found");
    } else if ((!strcmp(ri->request_method, "PUT") ||
                !strcmp(ri->request_method, "DELETE")) &&
            (conn->ctx->config[PUT_DELETE_PASSWORDS_FILE] == NULL ||
             !is_authorized_for_put(conn))) {
        send_authorization_request(conn);
    } else if (!strcmp(ri->request_method, "PUT")) {
        put_file(conn, path);
    } else if (!strcmp(ri->request_method, "DELETE")) {
        if (mg_remove(path) == 0) {
            send_http_error(conn, 200, "OK", "");
        } else {
            send_http_error(conn, 500, http_500_error, "remove(%s): %s", path,
                    strerror(ERRNO));
        }
    } else if (mg_stat(path, &st) != 0) {
        send_http_error(conn, 404, "Not Found", "%s", "File not found");
    } else if (st.is_directory && ri->uri[uri_len - 1] != '/') {
        (void) mg_printf(conn,
                "HTTP/1.1 301 Moved Permanently\r\n"
                "Location: %s/\r\n\r\n", ri->uri);
    } else if (st.is_directory &&
            !substitute_index_file(conn, path, sizeof(path), &st)) {
        if (!mg_strcasecmp(conn->ctx->config[ENABLE_DIRECTORY_LISTING], "yes")) {
            handle_directory_request(conn, path);
        } else {
            send_http_error(conn, 403, "Directory Listing Denied",
                    "Directory listing denied");
        }
    } else if (match_extension(path, conn->ctx->config[CGI_EXTENSIONS])) {
        if (strcmp(ri->request_method, "POST") &&
                strcmp(ri->request_method, "GET")) {
            send_http_error(conn, 501, "Not Implemented",
                    "Method %s is not implemented", ri->request_method);
        } else {
            send_http_error(conn, 501, "Not Implemented",
                    "CGI is not implemented", ri->request_method);
            //handle_cgi_request(conn, path);
            //exit(EXIT_FAILURE);
        }
    } else if (match_extension(path, conn->ctx->config[SSI_EXTENSIONS])) {
        handle_ssi_file_request(conn, path);
    } else if (is_not_modified(conn, &st)) {
        send_http_error(conn, 304, "Not Modified", "");
    } else {
        handle_file_request(conn, path, &st);
    }
}
dfs_close(outfd);
return 0;
}

int main(int argc, char **argv, char **envp)
{
    dret(); /* Get FS, etc from parent. */
    id = (int)strtol(argv[1], NULL, 10);
    snprintf(input_file, sizeof(input_file), "/threads/%d_in", id);
    snprintf(output_file, sizeof(output_file), "/threads/%d_out", id);
    printf("Worker %d created.\"\n", id);
    sync();
    /* Now, we are waiting for input from the parent. */
    while (1)
    {
        int size;
        int fd = dfs_open(input_file, DFS_O_RDONLY);
        if (fd < 0)
            break;
        /* Read the size of the packet, then the data itself. */
        dfs_read(fd, &size, sizeof(int));
        packet = malloc(size + sizeof(int));
        if (!packet)
            break;
        packet->size = size;
        dfs_read(fd, &packet->conn, packet->size);
        dfs_close(fd);
        process_request();
        /* Now, sync back up with the parent. The parent will send whatever is
         * in /threads/N_out to the client. */
        free(packet);
        sync();
    }
    return 0;
}

