#include <stdio.h>
#include <string.h>

#include "nx_model.h"
#include "util.h"

#define VERSION_INFO_PATH "/etc/version.info"

static NxModel s_nx_model = NX_MODEL_UNKNOWN;
static char s_nx_model_name[16];
static char s_nx_model_version[16];
static char s_mac_address[32];

void nx_model_init(void)
{
    FILE *version_info;

    version_info = fopen(VERSION_INFO_PATH, "r");
    if (version_info != NULL) {
        if (fgets(s_nx_model_version, sizeof(s_nx_model_version), version_info) == NULL) {
            die("failed to read version.");
        }
        if (fgets(s_nx_model_name, sizeof(s_nx_model_name), version_info) == NULL) {
            die("failed to read name.");
        }
        fclose(version_info);
    } else {
        die("failed to open " VERSION_INFO_PATH ".");
    }

    s_nx_model_name[strlen(s_nx_model_name) - 1] = '\0';
    s_nx_model_version[strlen(s_nx_model_version) - 1] = '\0';
    print_log("nx model = %s", s_nx_model_name);
    print_log("firmware version = %s", s_nx_model_version);

    if (strcmp(s_nx_model_name, "NX1") == 0) {
        s_nx_model = NX_MODEL_NX1;
    } else if (strcmp(s_nx_model_name, "NX300") == 0) {
        s_nx_model = NX_MODEL_NX300;
    } else if (strcmp(s_nx_model_name, "NX300M") == 0) { // TODO: check
        s_nx_model =  NX_MODEL_NX300M;
    } else if (strcmp(s_nx_model_name, "NX2000") == 0) {
        s_nx_model = NX_MODEL_NX2000;
    } else if (strcmp(s_nx_model_name, "NX3000") == 0) { // TODO: check
        s_nx_model = NX_MODEL_NX3000;
    } else if (strcmp(s_nx_model_name, "NX500") == 0) {
        s_nx_model = NX_MODEL_NX500;
    } else {
        die("unsupported nx model.");
    }
}

NxModel get_nx_model(void)
{
    return s_nx_model;
}

const char *get_nx_model_name(void)
{
    return s_nx_model_name;
}

const char *get_nx_model_version(void)
{
    return s_nx_model_version;
}

bool is_new_nx_model(void)
{
    if (s_nx_model == NX_MODEL_NX1 || s_nx_model == NX_MODEL_NX500) {
        return true;
    }
    return false;
}

bool is_old_nx_model(void)
{
    return !is_new_nx_model();
}

bool is_nx1(void)
{
    return get_nx_model() == NX_MODEL_NX1;
}

bool is_nx500(void)
{
    return get_nx_model() == NX_MODEL_NX500;
}

#define OLD_NX_FRAME_WIDTH 800
#define FRAME_WIDTH 720
#define FRAME_HEIGHT 480
#define OLD_NX_VIDEO_FRAME_SIZE (OLD_NX_FRAME_WIDTH * FRAME_HEIGHT * 3 / 2)
#define VIDEO_FRAME_SIZE (FRAME_WIDTH * FRAME_HEIGHT * 3 / 2)

int get_frame_width(void)
{
    if (is_old_nx_model()) {
        return OLD_NX_FRAME_WIDTH;
    } else {
        return FRAME_WIDTH;
    }
}

int get_frame_height(void)
{
    return FRAME_HEIGHT;
}

int get_frame_size(void)
{
    return get_frame_width() * get_frame_height() * 3 / 2;
}

static off_t s_addrs_nx300[] = {
    0xc6a14000,
    0xc6db6000,
    0xc7158000,
};

static off_t s_addrs_nx500[] = {
    0xbbaea500,
    0xbbb68e00,
    0xbbbe7700,
    0xbba6bc00,

//    0xa4a0d000, // MMAP_SIZE_2
//    0xa4ad2000, // MMAP_SIZE_2
//    0xaefd0000, // MMAP_SIZE_2
//    0xaf150000, // MMAP_SIZE_2

//    0x9f600000,
//    0x9f6fd000,
//    0x9f77b000,
//    0x9f8f7000,
};

static off_t s_addrs_nx1[] = {
    0xfb1be000,
    0xfb2de000,
    0xfb3fe000,
    0xfb51e000,
    0xfb63e000,
    0xfb75e000,
};

off_t get_video_addr(int index)
{
    if (is_nx1()) {
        return s_addrs_nx1[index];
    } else if (is_nx500()) {
        return s_addrs_nx500[index];
    } else {
        return s_addrs_nx300[index];
    }
}

int get_num_video_addrs(void)
{
    if (is_nx1()) {
        return ARRAY_SIZE(s_addrs_nx1);
    } else if (is_new_nx_model()) {
        return ARRAY_SIZE(s_addrs_nx500);
    } else {
        return ARRAY_SIZE(s_addrs_nx300);
    }
}

size_t get_mmap_video_size(void)
{
    return get_frame_size() + 4096;
}

const char *get_mac_address(void)
{
    FILE *file = NULL;;

    if (s_mac_address[0] != '\0') {
        return s_mac_address;
    }

    if (is_nx1()) {
        file = fopen("/sys/devices/platform/dw_mmc_sdio.0/mmc_host/"
                     "mmc2/mmc2:0002/mmc2:0002:1/net/mlan0/address", "r");
    } else if (is_nx500()) {
        file = fopen("/sys/devices/platform/dw_mmc_sdio.0/mmc_host/"
                     "mmc1/mmc1:0002/mmc1:0002:1/net/mlan0/address", "r");
    } else { // NX300
        file = fopen("/sys/devices/platform/dw_mmc_sdio.0/mmc_host/"
                     "mmc0/mmc0:0001/mmc0:0001:1/net/wlan0/address", "r");
    }
    if (file == NULL) {
        print_error("failed to get mac address!");
        return NULL;
    }

    if (fgets(s_mac_address, sizeof(s_mac_address), file) != NULL) {
        s_mac_address[strlen(s_mac_address) - 1] = '\0';
    }
    return s_mac_address;
}
