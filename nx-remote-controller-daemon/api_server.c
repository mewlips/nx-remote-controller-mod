#include "api_server.h"
#include "command.h"
#include "input.h"
#include "lcd.h"
#include "led.h"
#include "liveview.h"
#include "mongoose.h"
#include "network.h"
#include "nx_model.h"
#include "osd.h"
#include "shutter.h"
#include "status.h"
#include "util.h"

static const char *s_http_port = "80";
static struct mg_serve_http_opts s_http_server_opts;

void send_200(struct mg_connection *nc)
{
    mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Access-Control-Allow-Headers: X-Requested-With\r\n"
                        "Content-Length: 0\r\n\r\n");
}

void send_404(struct mg_connection *nc)
{
    mg_printf(nc, "%s", "HTTP/1.1 404 Not Found\r\n"
                        "Content-Length: 3\r\n\r\n"
                        "404");
}

void send_500(struct mg_connection *nc)
{
    mg_printf(nc, "%s", "HTTP/1.1 500 Internal Server Error\r\n"
                        "Content-Length: 3\r\n\r\n"
                        "500");
}

static void handle_camera_api(struct mg_connection *nc,
                              struct http_message *hm,
                              struct mg_str *func)
{
    if (mg_vcmp(func, "info") == 0) {
        mg_printf(nc, "HTTP/1.1 200 OK\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Access-Control-Allow-Headers: X-Requested-With\r\n"
                      "Transfer-Encoding: chunked\r\n"
                      "Content-Type: application/json; charset=utf-8\r\n\r\n");
        mg_printf_http_chunk(nc, "{\"model\":\"%s\",\"fw_ver\":\"%s\"}",
                             get_nx_model_name(), get_nx_model_version());
        mg_send_http_chunk(nc, "", 0);
    } else if (mg_vcmp(func, "status") == 0) {
        char json_cameras[1024];
        int json_size = network_get_discovered_cameras(
                            json_cameras, sizeof(json_cameras));
        if (json_size == -1) {
            snprintf(json_cameras, sizeof(json_cameras), "[]");
        }
        mg_printf(nc, "HTTP/1.1 200 OK\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Access-Control-Allow-Headers: X-Requested-With\r\n"
                      "Transfer-Encoding: chunked\r\n"
                      "Content-Type: application/json; charset=utf-8\r\n\r\n");
        mg_printf_http_chunk(nc,
                             "{\"battery_charging\":%s,"
                             "\"battery_percent\":%d,"
                             "\"battery_level\":%d,"
                             "\"drive\":\"%s\","
                             "\"mode\":\"%s\","
                             "\"hevc\":\"%s\","
                             "\"cameras\":%s}",
                             is_battery_charging() ? "true" : "false",
                             get_battery_percent(),
                             get_battery_level(),
                             get_dial_drive_string(),
                             get_dial_mode_string(),
                             get_hevc_state_string(),
                             json_cameras);
        mg_send_http_chunk(nc, "", 0);
    } else {
        send_404(nc);
    }
}

static void handle_lcd_api(struct mg_connection *nc,
                           struct http_message *hm,
                           struct mg_str *func)
{
    if (mg_vcmp(func, "on") == 0) {
        lcd_set_state(LCD_ON);
        send_200(nc);
    } else if (mg_vcmp(func, "off") == 0) {
        lcd_set_state(LCD_OFF);
        send_200(nc);
    } else if (mg_vcmp(func, "video") == 0) {
        lcd_set_state(LCD_VIDEO);
        send_200(nc);
    } else {
        send_404(nc);
    }
}

static void handle_led_api(struct mg_connection *nc,
                           struct http_message *hm,
                           struct mg_str *func)
{
    if (mg_vcmp(func, "on") == 0) {
        led_set(true);
        send_200(nc);
    } else if (mg_vcmp(func, "off") == 0) {
        led_set(false);
        send_200(nc);
    } else {
        send_404(nc);
    }
}

static void handle_shutter_api(struct mg_connection *nc,
                               struct http_message *hm,
                               struct mg_str *func)
{
    if (mg_vcmp(func, "normal") == 0) {
        shutter_set_silent(false);
        send_200(nc);
    } else if (mg_vcmp(func, "silent") == 0) {
        shutter_set_silent(true);
        send_200(nc);
    } else {
        send_404(nc);
    }
}

static void handle_liveview_api(struct mg_connection *nc,
                                struct http_message *hm,
                                struct mg_str *func)
{
    if (mg_vcmp(func, "get") == 0) {
        liveview_http_send(nc, hm, false);
    } else if (mg_vcmp(func, "get_low") == 0) {
        liveview_http_send(nc, hm, true);
    }
}

static void handle_osd_api(struct mg_connection *nc,
                           struct http_message *hm,
                           struct mg_str *func)
{
    if (mg_ncasecmp(func->p, "get", 3) == 0) {
        osd_http_send(nc, hm);
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

        send_200(nc);
    } else if (mg_vcmp(func, "inject_keep_alive") == 0) {
        input_inject_keep_alive();
        send_200(nc);
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
    static const struct mg_str lcd_prefix = MG_MK_STR("lcd/");
    static const struct mg_str led_prefix = MG_MK_STR("led/");
    static const struct mg_str shutter_prefix = MG_MK_STR("shutter/");
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
                } else if (has_prefix(&uri, &lcd_prefix)) {
                    func.p = uri.p + lcd_prefix.len;
                    func.len = uri.len - lcd_prefix.len;
                    handle_lcd_api(nc, hm, &func);
                } else if (has_prefix(&uri, &led_prefix)) {
                    func.p = uri.p + led_prefix.len;
                    func.len = uri.len - led_prefix.len;
                    handle_led_api(nc, hm, &func);
                } else if (has_prefix(&uri, &shutter_prefix)) {
                    func.p = uri.p + shutter_prefix.len;
                    func.len = uri.len - shutter_prefix.len;
                    handle_shutter_api(nc, hm, &func);
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
    char web_root[256];
    struct mg_mgr mgr;
    struct mg_connection *nc;

    snprintf(web_root, sizeof(web_root), "%s/web_root", get_app_path());

    mg_mgr_init(&mgr, NULL);

    s_http_server_opts.document_root = web_root;
    s_http_server_opts.enable_directory_listing = "yes";

    nc = mg_bind(&mgr, s_http_port, ev_handler);
    mg_set_protocol_http_websocket(nc);

    print_log("Starting threaded server on port %s", s_http_port);

    for (;;) {
        if (network_get_wifi_ip_address() == NULL) {
            print_log("wifi down! stopping api server...");
            break;
        }
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
}
