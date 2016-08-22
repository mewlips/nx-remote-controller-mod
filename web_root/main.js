var nxModelName;
var nxFwVer;
var modalEnabled = false;
var keepAliveTimer;

function getCameraInfo() {
    $.ajax({
        url: '/api/v1/camera/info',
        success: function(info) {
            nxModelName = info.model;
            nxFwVer = info.fw_ver;

            var text = 'NX Remote Controller ' +
                       '[ ' + info.model + ' (fw ' + info.fw_ver + ')]';
            document.title = text;

            if (isNx1()) {
                $('.nx1-only').show();
            } else {
                $('.nx1-only').hide();
            }
            if (isNx500()) {
                $('.nx500-only').show();
            } else {
                $('.nx500-only').hide();
            }
            if (isNx1() || isNx500()) {
                $('.nx1-nx500-only').show();
            } else {
                $('.nx1-nx500-only').hide();
            }
            if (isNx300()) {
                $('.nx300-only').show();
            } else {
                $('.nx300-only').hide();
            }
            if (!isNx1()) {
                $('.not-nx1-only').show();
            } else {
                $('.not-nx1-only').hide();
            }

            if (keepAliveTimer) {
                clearInterval(keepAliveTimer);
            }
            keepAliveTimer = setInterval(function () {
                inputInjectKeepAlive();
            }, 25*1000);
        }
    });
}

function isNx1() {
    return nxModelName == 'NX1';
}

function isNx500() {
    return nxModelName == 'NX500';
}

function isNx300() {
    return nxModelName == 'NX300';
}

function getCameraStatus() {
    $.ajax({
        url: '/api/v1/camera/status',
        timeout: 1000,
        success: function(status) {
            var html = '';
            if (isNx1()) {
                var batteryIcon;
                if (status.battery_percent > 75) {
                    batteryIcon = '<i class="fa fa-battery-4"></i>';
                } else if (status.battery_percent > 50) {
                    batteryIcon = '<i class="fa fa-battery-3"></i>';
                } else if (status.battery_percent > 25) {
                    batteryIcon = '<i class="fa fa-battery-2"></i>';
                } else if (status.battery_percent > 10) {
                    batteryIcon = '<i class="fa fa-battery-1"></i>';
                } else {
                    batteryIcon = '<i class="fa fa-battery-0" style="color:red"></i>';
                }
                // TODO: vgrip
                html += batteryIcon + ' ' + status.battery_percent + '%' +
                        (status.battery_charging == true ?
                         ' (<i class="fa fa-bolt"></i>)' : "");
            } else {
                var batteryIcon;
                if (status.battery_level == 5) {
                    batteryIcon = '<i class="fa fa-battery-4"></i>';
                } else if (status.battery_level == 4) {
                    batteryIcon = '<i class="fa fa-battery-3"></i>';
                } else if (status.battery_level == 3) {
                    batteryIcon = '<i class="fa fa-battery-2"></i>';
                } else if (status.battery_level == 2) {
                    batteryIcon = '<i class="fa fa-battery-1"></i>';
                } else {
                    batteryIcon = '<i class="fa fa-battery-0" style="color:red"></i>';
                }

                html += batteryIcon +
                        (status.battery_charging == true ?
                         ' <i class="fa fa-bolt"></i>' : "");
            }

            html += ' / Mode: ' + status.mode;

            $('#panel-content').html(html);

            if (status.hevc == 'on') {
                setOsdTimeoutInterval(50);
                setLiveviewTimeoutInterval(50);
            } else if (status.hevc == 'off') {
                setOsdTimeoutInterval(50);
                setLiveviewTimeoutInterval(50);
            }

            if (typeof(Storage) !== "undefined") {
                if (liveviewStarted == false) {
                    if (localStorage.getItem("liveview") === "hq") {
                        startLiveview(false); // restart hq liveview
                    } else if (localStorage.getItem("liveview") === "lq") {
                        startLiveview(true); // restart lq liveview
                    }
                }
                if (osdStarted == false &&
                        localStorage.getItem("osd") === "show") {
                    startOsd(); // restart osd
                }
            }

            var cameras = "";
            for (var i = 0; i < status.cameras.length; i++) {
                var ip = status.cameras[i].ip;
                var model = status.cameras[i].packet.split('|')[2];
                cameras += '<li><a href="http://' + ip + '">' + model +
                           ' (' + status.cameras[i].ip + ')</a></li>';
            }
            $('#cameras').html(cameras);
            //debug(status.cameras[0].ip + status.cameras[0].packet);
            if (modalEnabled == true) {
                $('#disconnectedModal').modal('hide');
                modalEnabled = false;
            }
        },
        error: function (request, status, error) {
            if (modalEnabled == false) {
                $('#disconnectedModal').modal('show');
                modalEnabled = true;
            }
        }
    });
}

function controlLcd(state) {
    $.ajax({
        url: '/api/v1/lcd/' + state,
        success: function(data) {
        }
    });
}

function shutterSetSilent(silent) {
    $.ajax({
        url: '/api/v1/shutter/' + (silent ? 'silent' : 'normal'),
        success: function(data) {
        }
    });
}

function debug(str) {
    $('#debug').html(str);
}

function ledSet(on) {
    $.ajax({
        url: '/api/v1/led/' + (on ? "on" : "off"),
    });
}

function ledBlink() {
    for (var ms = 0; ms < 2000; ms += 200) {
        setTimeout(function () {
            ledSet(true);
        }, ms);
        setTimeout(function () {
            ledSet(false);
        }, ms + 100);
    }
}

function setupRemoteController() {
    getCameraInfo();
    setupOsdInput();
    setupInput();

    controlLcd('on');
    setInterval(getCameraStatus, 1000);

    if (typeof(Storage) !== "undefined") {
        if (localStorage.getItem("liveview") === "lq") {
            startLiveview(true);
        } else if (localStorage.getItem("liveview") === "hq") {
            startLiveview(false);
        }
    }
    if (typeof(Storage) !== "undefined") {
        if (localStorage.getItem("osd") === "show") {
            startOsd();
        }
    }

    ledBlink();
}

function showAndroidToast(msg) {
    Android.showToast(msg);
}
