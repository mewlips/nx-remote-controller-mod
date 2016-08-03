#include "api_server.h"
#include "executor.h"
#include "input.h"
#include "liveview.h"
#include "mongoose.h"
#include "nx_model.h"
#include "osd.h"
#include "util.h"

static const char *s_http_port = "80";
static struct mg_serve_http_opts s_http_server_opts;

static void handle_camera_api(struct mg_connection *nc,
                              struct http_message *hm,
                              struct mg_str *func)
{
    if (mg_vcmp(func, "version") == 0) {
        mg_printf(nc, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
        mg_printf_http_chunk(nc, "{ \"model\": \"%s\", \"fw_ver\": \"%s\" }",
                             get_nx_model_name(), get_nx_model_version());
        mg_send_http_chunk(nc, "", 0); // Send empty chunk, the end of response
    } else if (mg_vcmp(func, "info") == 0) {
        int cpu_usage = 50; // TODO: %
        int free_mem = 20000; // TODO: KB
        int free_sdcard = 500; // TODO: MB

        mg_printf(nc, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
        mg_printf_http_chunk(nc, "{ \"cpu\": %d, \"mem\": %d, \"sdcard\": %d }",
                             cpu_usage, free_mem, free_sdcard);
        mg_send_http_chunk(nc, "", 0);
    } else {
        print_log("");
        // TODO: 404
    }
}

static void handle_liveview_api(struct mg_connection *nc,
                                struct http_message *hm,
                                struct mg_str *func)
{
    if (mg_vcmp(func, "get") == 0) {
        liveview_send(nc, hm);
    }
}

static void handle_osd_api(struct mg_connection *nc,
                           struct http_message *hm,
                           struct mg_str *func)
{
    if (mg_ncasecmp(func->p, "get", 3) == 0) {
        osd_send(nc, hm);
    }
}

static void handle_input_api(struct mg_connection *nc,
                             struct http_message *hm,
                             struct mg_str *func)
{
    static const struct mg_str inject_prefix = MG_MK_STR("inject?");
    struct mg_str command_start;
    char command[128];
    char *p;

    if (mg_ncasecmp(func->p, "inject?", 7) == 0) {
        command_start.p = func->p + inject_prefix.len;
        command_start.len = func->len - inject_prefix.len;
        p = strstr(command_start.p, " ");
        strncpy(command, command_start.p, p - command_start.p);
        command[p - command_start.p] = '\0';
        p = strstr(command, "=");
        if (p != NULL) {
            *p = ' ';
        }
        p = strstr(command, "-");
        if (p != NULL) {
            *p = ' ';
        }
        print_log("command = %s", command);
        input_inject(command);

        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
    }
}

static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix) {
    return uri->len > prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static void ev_handler(struct mg_connection *nc, int ev, void *p)
{
    static const struct mg_str api_prefix = MG_MK_STR("/api/v1/");
    static const struct mg_str camera_prefix = MG_MK_STR("camera/");
    static const struct mg_str liveview_prefix = MG_MK_STR("liveview/");
    static const struct mg_str osd_prefix = MG_MK_STR("osd/");
    static const struct mg_str input_prefix = MG_MK_STR("input/");
    struct http_message *hm = (struct http_message *) p;
    struct mg_str uri;
    struct mg_str func;

    switch (ev) {
        case MG_EV_HTTP_REQUEST:
            if (has_prefix(&hm->uri, &api_prefix)) {
                uri.p = hm->uri.p + api_prefix.len;
                uri.len = hm->uri.len - api_prefix.len;
                if (has_prefix(&uri, &camera_prefix)) {
                    func.p = uri.p + camera_prefix.len;
                    func.len = uri.len - camera_prefix.len;
                    handle_camera_api(nc, hm, &func);
                } else if (has_prefix(&uri, &liveview_prefix)) {
                    func.p = uri.p + liveview_prefix.len;
                    func.len = uri.len - liveview_prefix.len;
                    handle_liveview_api(nc, hm, &func);
                } else if (has_prefix(&uri, &osd_prefix)) {
                    func.p = uri.p + osd_prefix.len;
                    func.len = uri.len - osd_prefix.len;
                    handle_osd_api(nc, hm, &func);
                } else if (has_prefix(&uri, &input_prefix)) {
                    func.p = uri.p + input_prefix.len;
                    func.len = uri.len - input_prefix.len;
                    handle_input_api(nc, hm, &func);
                }
            } else {
                mg_serve_http(nc, hm, s_http_server_opts);
            }
            break;
        default:
            break;
    }
}

void api_server_run(void)
{
    struct mg_mgr mgr;
    struct mg_connection *nc;

    mg_mgr_init(&mgr, NULL);

    s_http_server_opts.document_root = "web_root";
    s_http_server_opts.enable_directory_listing = "yes";

    nc = mg_bind(&mgr, s_http_port, ev_handler);
    mg_set_protocol_http_websocket(nc);

    print_log("Starting threaded server on port %s\n", s_http_port);

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
}
