function getKeyMapping(key) {
    if (key == 'UP') {
        return 'KP_Up';
    } else if (key == 'LEFT') {
        return 'KP_Left';
    } else if (key == 'RIGHT') {
        return 'KP_Right';
    } else if (key == 'DOWN') {
        return 'KP_Down';
    } else if (key == 'DEL') {
        return 'KP_Delete';
    } else if (key == 'DEPTH') {
        return 'Henkan_Mode';
    } else if (key == 'METER') {
        return 'Hiragana_Katakana';
    } else if (key == 'OK') {
        return 'KP_Enter';
    } else if (key == 'PWON') {
        return 'XF86AudioRaiseVolume';
    } else if (key == 'PWOFF') {
        return 'XF86PowerOff';
    } else if (key == 'RESET') {
        return 'XF86PowerOff';
    } else if (key == 'S1') {
        return 'Super_L';
    } else if (key == 'S2') {
        return 'Super_R';
    } else if (key == 'MENU') {
        return 'Menu';
    } else if (key == 'AEL') {
        return 'XF86Favorites';
    } else if (key == 'REC') {
        return 'XF86WebCam';
    } else if (key == 'FN') {
        return 'XF86HomePage';
    } else if (key == 'EV') {
        return 'XF86Reload';
    } else if (key == 'PB') {
        return 'XF86Tools';
    } else if (key == 'AF_MODE') {
        return 'XF86TaskPane';
    } else if (key == 'WB') {
        return 'XF86Launch6';
    } else if (key == 'ISO') {
        return 'XF86Launch7';
    } else if (key == 'AF_ON') {
        return 'XF86Launch9';
    } else if (key == 'LIGHT') {
        return 'XF86TouchpadToggle';
    } else if (key == 'MF_ZOOM') {
        return 'XF86TouchpadOff';
    } else if (key == 'WIFI') {
        return 'XF86Mail';
    } else if (key == 'JOG1_CW') {
        return 'XF86ScrollUp';
    } else if (key == 'JOG1_CCW') {
        return 'XF86ScrollDown';
    } else if (key == 'JOG2_CW') {
        return 'parenleft';
    } else if (key == 'JOG2_CCW') {
        return 'parenright';
    } else if (key == 'JOG_CW') {
        return 'XF86AudioNext';
    } else if (key == 'JOG_CCW') {
        return 'XF86AudioPrev';
    } else if (key == 'MODE_SCENE_GET') {
        return 'F4';
    } else if (key == 'MODE_SCENE') {
        return 'XF86Send';
    } else if (key == 'MODE_SMART_GET') {
        return 'F6';
    } else if (key == 'MODE_SMART') {
        return 'XF86Reply';
    } else if (key == 'MODE_P_GET') {
        return 'F7';
    } else if (key == 'MODE_P') {
        return 'XF86MailForward';
    } else if (key == 'MODE_A_GET') {
        return 'F8';
    } else if (key == 'MODE_A') {
        return 'XF86Save';
    } else if (key == 'MODE_S_GET') {
        return 'F9';
    } else if (key == 'MODE_S') {
        return 'XF86Documents';
    } else if (key == 'MODE_M_GET') {
        return 'F10';
    } else if (key == 'MODE_M') {
        return 'XF86Battery';
    } else if (key == 'MODE_CUSTOM1_GET') {
        return 'KP_Home';
    } else if (key == 'MODE_CUSTOM1') {
        return 'XF86WLAN';
    } else if (key == 'MODE_CUSTOM2_GET') {
        return 'Scroll_Lock';
    } else if (key == 'MODE_CUSTOM2') {
        return 'XF86Bluetooth';
    } else if (key == 'MODE_SAS_GET') {
        return 'F1';
    } else if (key == 'MODE_SAS') {
        return 'XF86KbdBrightnessDown';
    } else if (key == 'WHEEL_CW') {
        return 'Left';
    } else if (key == 'WHEEL_CCW') {
        return 'Right';
    } else if (key == 'DRIVE_SINGLE') {
        return 'XF86Search';
    } else if (key == 'DRIVE_CONTI_N') {
        return 'XF86Go';
    } else if (key == 'DRIVE_CONTI_H') {
        return 'XF86Finance';
    } else if (key == 'DRIVE_TIMER') {
        return 'XF86Game';
    } else if (key == 'DRIVE_BRACKET') {
        return 'XF86Shop';
    }

    return key;
}

function onKey(key) {
    $.ajax({
        url: '/api/v1/input/inject?key=' + getKeyMapping(key),
        success: function(data) {
        }
    });
}

function onKeyDown(key) {
    $.ajax({
        url: '/api/v1/input/inject?keydown=' + getKeyMapping(key),
        success: function(data) {
        }
    });
}

function oKeyUp(key) {
    $.ajax({
        url: '/api/v1/input/inject?keyup=' + getKeyMapping(key),
        success: function(data) {
        }
    });
}
