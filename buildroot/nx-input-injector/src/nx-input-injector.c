#include <stdio.h>
#include <string.h>
#include <xdo.h>

int main(void)
{
    char line[256];
    char key[64];
    int button;
    int x;
    int y;

    xdo_t *xdo = xdo_new(":0");

    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (strncmp(line, "mousedown ", 10) == 0) {
            if (sscanf(line + 10, "%d\n", &button) == 1) {
                xdo_mouse_down(xdo, CURRENTWINDOW, button);
            }
        } else if (strncmp(line, "mouseup ", 8) == 0) {
            if (sscanf(line + 8, "%d\n", &button) ==  1) {
                xdo_mouse_up(xdo, CURRENTWINDOW, button);
            }
        } else if (strncmp(line, "mousemove ", 10) == 0) {
            if (sscanf(line + 10, "%d %d\n", &x, &y) == 2) {
                xdo_move_mouse(xdo, x, y, 0);
            }
        } else if (strncmp(line, "key ", 4) == 0) {
            if (sscanf(line + 4, "%s\n", key) == 1) {
                xdo_send_keysequence_window(xdo, CURRENTWINDOW, key, 0);
            }
        } else if (strncmp(line, "keyup ", 6) == 0) {
            if (sscanf(line + 6, "%s\n", key) == 1) {
                xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, key, 0);
            }
        } else if (strncmp(line, "keydown ", 8) == 0) {
            if (sscanf(line + 8, "%s\n", key) == 1) {
                xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, key, 0);
            }
        }
    }

    xdo_free(xdo);

    return 0;
}
