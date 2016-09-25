function NxRemoteController(hostname) {
    this.urlPrefix = '';
    if (hostname != null) {
        this.urlPrefix = 'http://' + hostname; 
    }
    this.nxModelName = "";
    this.nxFwVer = "";
    this.modalEnabled = false;
    this.keepAliveTimer = null;
    this.liveView = null;
    this.osd = null;
    this.input = null;
}

NxRemoteController.prototype.createUrl = function (path) {
    return this.urlPrefix + path;
}

NxRemoteController.prototype.getCameraInfo = function () {
    var self = this;
    $.ajax({
        url: self.createUrl('/api/v1/camera/info'),
        success: function (info) {
            self.nxModelName = info.model;
            self.nxFwVer = info.fw_ver;

            var text = 'NX Remote Controller ' +
                       '[ ' + info.model + ' (fw ' + info.fw_ver + ')]';
            document.title = text;

            if (self.isNx1()) {
                $('.nx1-only').show();
            } else {
                $('.nx1-only').hide();
            }
            if (self.isNx500()) {
                $('.nx500-only').show();
            } else {
                $('.nx500-only').hide();
            }
            if (self.isNx1() || self.isNx500()) {
                $('.nx1-nx500-only').show();
            } else {
                $('.nx1-nx500-only').hide();
            }
            if (self.isNx300()) {
                $('.nx300-only').show();
            } else {
                $('.nx300-only').hide();
            }
            if (!self.isNx1()) {
                $('.not-nx1-only').show();
            } else {
                $('.not-nx1-only').hide();
            }

            if (self.keepAliveTimer) {
                clearInterval(self.keepAliveTimer);
            }
            self.input.injectKeepAlive();
            self.keepAliveTimer = setInterval(function () {
                self.input.injectKeepAlive();
            }, 25*1000);
        }
    });
}

NxRemoteController.prototype.isNx1 = function () {
    return this.nxModelName == 'NX1';
}

NxRemoteController.prototype.isNx500 = function () {
    return this.nxModelName == 'NX500';
}

NxRemoteController.prototype.isNx300 = function () {
    return this.nxModelName == 'NX300';
}

NxRemoteController.prototype.getCameraStatus = function () {
    var self = this;
    $.ajax({
        url: self.createUrl('/api/v1/camera/status'),
        timeout: 1000,
        success: function(status) {
            var html = '';
            if (self.isNx1()) {
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
                self.osd.setTimeoutInterval(50);
                self.liveView.setTimeoutInterval(50);
            } else if (status.hevc == 'off') {
                self.osd.setTimeoutInterval(50);
                self.liveView.setTimeoutInterval(50);
            }

            if (typeof(Storage) !== "undefined") {
                if (self.liveView.started == false) {
                    if (localStorage.getItem("liveview") === "hq") {
                        self.liveView.start(false); // restart hq liveview
                    } else if (localStorage.getItem("liveview") === "lq") {
                        self.liveView.start(true); // restart lq liveview
                    }
                }
                if (self.osd.tarted == false &&
                        localStorage.getItem("osd") === "show") {
                    self.osd.start(); // restart osd
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
            if (self.modalEnabled == true) {
                $('#disconnectedModal').modal('hide');
                self.modalEnabled = false;
            }
        },
        error: function (request, status, error) {
            if (self.modalEnabled == false) {
                $('#disconnectedModal').modal('show');
                self.modalEnabled = true;
            }
        }
    });
}

NxRemoteController.prototype.controlLcd = function (state) {
    var self = this;
    $.ajax({
        url: self.createUrl('/api/v1/lcd/' + state),
        mimeType: 'text/html',
        success: function(data) {
        }
    });
}

NxRemoteController.prototype.shutterSetSilent = function (silent) {
    var self = this;
    $.ajax({
        url: self.createUrl('/api/v1/shutter/' + (silent ? 'silent' : 'normal')),
        mimeType: 'text/html',
        success: function(data) {
        }
    });
}

NxRemoteController.prototype.ledSet = function (on) {
    var self = this;
    $.ajax({
        url: self.createUrl('/api/v1/led/' + (on ? "on" : "off")),
        mimeType: 'text/html',
        success: function(data) {
        }
    });
}

NxRemoteController.prototype.ledBlink = function () {
    var self = this;
    for (var ms = 0; ms < 2000; ms += 200) {
        setTimeout(function () {
            self.ledSet(true);
        }, ms);
        setTimeout(function () {
            self.ledSet(false);
        }, ms + 100);
    }
}

NxRemoteController.prototype.setup = function () {
    var self = this;

    this.getCameraInfo();

    this.liveView = new LiveView(this);

    this.osd = new Osd(this);
    this.osd.setup();

    this.input = new Input(this);
    this.input.setup();

    this.controlLcd('on');
    setInterval(function () {
        self.getCameraStatus();
    }, 1000);

    if (typeof(Storage) !== "undefined") {
        if (localStorage.getItem("liveview") === "lq") {
            this.liveView.start(true);
        } else if (localStorage.getItem("liveview") === "hq") {
            this.liveView.start(false);
        }
    }
    if (typeof(Storage) !== "undefined") {
        if (localStorage.getItem("osd") === "show") {
            this.osd.start();
        }
    }

    this.ledBlink();
}

function showAndroidToast(msg) {
    Android.showToast(msg);
}
