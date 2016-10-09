function NxRemoteController(hostname) {
    this.hostname = hostname;
    this.viewFinder = null;
    this.urlPrefix = 'http://' + hostname; 
    this.nxModelName = "NX1"; // FIXME: fallback
    this.nxFwVer = "";
    this.macAddress = "";
    this.modalEnabled = false;
    this.liveView = null;
    this.osd = null;
    this.mouseInput = null;
    this.statusTimer = null;
    this.settings = null;

    this.init();
}

NxRemoteController.prototype.createUrl = function (path) {
    return this.urlPrefix + path;
}

NxRemoteController.prototype.setVisibility = function () {
    if (this.isNx1()) {
        this.viewFinder.panel.target.find('.nx1-only').show();
    } else {
        this.viewFinder.panel.target.find('.nx1-only').hide();
    }
    if (this.isNx500()) {
        this.viewFinder.panel.target.find('.nx500-only').show();
    } else {
        this.viewFinder.panel.target.find('.nx500-only').hide();
    }
    if (this.isNx1() || this.isNx500()) {
        this.viewFinder.panel.target.find('.nx1-nx500-only').show();
    } else {
        this.viewFinder.panel.target.find('.nx1-nx500-only').hide();
    }
    if (this.isNx300()) {
        this.viewFinder.panel.target.find('.nx300-only').show();
    } else {
        this.viewFinder.panel.target.find('.nx300-only').hide();
    }
    if (!this.isNx1()) {
        this.viewFinder.panel.target.find('.not-nx1-only').show();
    } else {
        this.viewFinder.panel.target.find('.not-nx1-only').hide();
    }
}

NxRemoteController.prototype.getCameraInfo = function () {
    var self = this;
    $.ajax({
        url: self.createUrl('/api/v1/camera/info'),
        success: function (info) {
            self.nxModelName = info.model;
            self.nxFwVer = info.fw_ver;
            self.macAddress = info.mac_address;

            var text = 'NX Remote Controller ' +
                       '[ ' + info.model + ' (fw ' + info.fw_ver + ')]';
            document.title = text;

            self.settings = new Settings(self.macAddress);
            self.viewFinder = new ViewFinder(self);
            self.osd = new Osd(self);
            self.liveView = new LiveView(self);
            self.mouseInput = new MouseInput(self);

            self.setVisibility();
            self.controlLcd('on');
            self.statusTimer = setInterval(function () {
                self.getCameraStatus();
            }, 1000);

            app.controlPanel.updateTitle();
            app.controlPanel.setVisibility();
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

NxRemoteController.prototype.restartScreen = function () {
    var self = this;

    if (self.liveView != null && self.liveView.started == false) {
        var value = this.settings.getLiveView();
        if (value === "hq") {
            self.liveView.start(false); // restart hq liveview
        } else if (value === "lq") {
            self.liveView.start(true); // restart lq liveview
        } else if (value === "hide") {
            self.liveView.stop();
        }
    }
    if (self.osd != null && self.osd.started == false) {
        var value = this.settings.getOsd();
        if (value === "show") {
            self.osd.start(); // restart osd
        } else if (value === "hide") {
            self.osd.stop();
        }
    }
}

NxRemoteController.prototype.getCameraStatus = function () {
    var self = this;
    $.ajax({
        url: self.createUrl('/api/v1/camera/status'),
        timeout: 3000,
        success: function(status) {
            var title = $('<span></span>')
                .click(function () {
                    self.ledBlink();
                });
            var html = '&nbsp;' + self.nxModelName + ' / ' +
                       self.hostname + ' / ';
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

            self.viewFinder.panel.setTitle(title.html(html));

            if (status.hevc == 'on') {
                if (self.osd != null) {
                    self.osd.setTimeoutInterval(500);
                }
                if (self.liveView != null) {
                    self.liveView.setTimeoutInterval(500);
                }
                if (Settings.getScreenOffOnRecord()) {
                    self.stopScreen();
                }
            } else if (status.hevc == 'off') {
                if (self.osd != null) {
                    self.osd.setTimeoutInterval(50);
                }
                if (self.liveView != null) {
                    self.liveView.setTimeoutInterval(50);
                }
            }

            if (!self.viewFinder.panel.isCollapsed()
                    && app.getActiveTab() == '#controller') {
                if (status.hevc != 'on') {
                    self.restartScreen();
                } else if (!Settings.getScreenOffOnRecord()) {
                    self.restartScreen();
                }
            }

            for (var i = 0; i < status.cameras.length; i++) {
                var ip = status.cameras[i].ip;
                var model = status.cameras[i].packet.split('|')[2];
                status.cameras[i].model = model;
                status.cameras[i].macAddress =
                    status.cameras[i].packet.split('|')[4];
                app.setCameras(status.cameras);
            }
            if (self.modalEnabled == true) {
                $('#disconnectedModal').modal('hide');
                self.modalEnabled = false;
            }
        },
        error: function (request, status, error) {
            if (self.modalEnabled == false) {
                var hostname;
                var found = false;
                for (hostname in app.controllers) {
                    var controller = app.controllers[hostname];
                    if (controller.hostname == self.hostname) {
                        found = true;
                    }
                }
                if (!found) {
                    return;
                }

                var name = self.settings.getName();
                if (name == '') {
                    name = self.hostname;
                }
                $('#disconnectedModalBody')
                    .html($('<p></p>').append(name + ' is disconnected.'));
                $('#disconnectedModal').modal('show');
                self.modalEnabled = true;
                setTimeout(function() {
                    if (self.modalEnabled) {
                        app.removeController(self.hostname);
                        $('#disconnectedModal').modal('hide');
                        self.modalEnabled = false;
                    }
                }, 5000); // wait 5 second;
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
    for (var ms = 0; ms < 2000; ms += 500) {
        setTimeout(function () {
            self.ledSet(true);
        }, ms);
        setTimeout(function () {
            self.ledSet(false);
        }, ms + 250);
    }
}

NxRemoteController.prototype.init = function () {
    var self = this;

    this.getCameraInfo();
    this.ledBlink();
}

NxRemoteController.prototype.stopScreen = function () {
    if (this.osd != null) {
        this.osd.stop();
    }
    if (this.liveView != null) {
        this.liveView.stop();
    }
}

NxRemoteController.prototype.destroy = function () {
    this.osd.destroy();
    this.osd = null;
    this.liveView.destroy();
    this.liveView = null;
    this.mouseInput = null;
    if (this.statusTimer != null) {
        clearInterval(this.statusTimer);
        this.statusTimer = null;
    }
    if (this.viewFinder != null) {
        this.viewFinder.destroy();
        this.viewFinder = null;
    }
}

NxRemoteController.prototype.isKeyInputEnabled = function () {
    if (this.settings != null) {
        return this.settings.getKeyInputEnabled();
    } else {
        return false;
    }
}

function showAndroidToast(msg) {
    Android.showToast(msg);
}
