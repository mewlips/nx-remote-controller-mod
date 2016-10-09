var app = null;

function App() {
    this.hostname = document.location.hostname;
    if (document.location.hostname == '' ||
        document.location.hostname == 'localhost' ||
        document.location.hostname == '127.0.0.1') {
        this.hostname = prompt("Enter IP address of your camera", '');
    }

    this.controllers = {};
    this.cameras = [];
    this.keyInput = null;
    this.controlPanel = null;
    this.activeTab = '#controller';
}

App.init = function () {
    app = new App();
    Settings.init();
    app.init();
}

App.prototype.init = function () {
    var self = this;

    this.addController(this.hostname);
    this.keyInput = new KeyInput(this.controllers);
    this.controlPanel = new ControlPanel(this.controllers);
    this.gallery = new Gallery(this.controllers);

    $('.nav-tabs a').on('shown.bs.tab', function (event) {
        self.activeTab = $(event.target).attr('href');
        if (self.activeTab != '#controller') {
            var hostname;
            for (hostname in self.controllers) {
                var controller = self.controllers[hostname];
                controller.stopScreen();
            }
        }

        if (self.activeTab == '#gallery') {
            if (self.gallery.iframe == null) {
                self.gallery.init('http://' + self.hostname + '/DCIM');
            }
        }
    });
}

App.prototype.getActiveTab = function () {
    return this.activeTab;
}

App.prototype.setCameras = function (cameras) {
    var self = this;
    this.cameras = cameras;

    var camerasMenu = $('#camerasMenu');
    camerasMenu.children().each(function () {
        var i;
        var found = false;
        for (i = 0; i < self.cameras.length; i++) {
            var camera = self.cameras[i];
            if (camera.ip == $(this)[0].cameraIp) {
                found = true;
                break;
            }
        }
        if (!found) {
            $(this).remove();
        }
    });

    var i;
    for (i = 0; i < cameras.length; i++) {
        var camera = cameras[i];
        var found = false;
        camerasMenu.children().each(function () {
            var i;
            if (camera.ip == $(this)[0].cameraIp) {
                found = true;
            }
        });
        if (!found && camera.model != null) {
            var name = Settings.getNameByMacAddress(camera.macAddress);
            if (name != '') {
                name += ' / ';
            }
            var li = $('<li></li>')
                     .addClass('camera-list text-primary')
                     .text(name + camera.model + ' / ' + camera.ip);
            var ip = camera.ip;
            li.click(function () {
                if (self.activeTab == '#controller') {
                    location.href = 'http://' + ip;
                } else if (self.activeTab == '#gallery') {
                    self.gallery.init('http://' + ip + '/DCIM');
                }
            });
            li[0].cameraIp = ip;
            camerasMenu.append(li);

            if (self.controllers[camera.ip] == null) {
                self.addController(camera.ip);
            }
        }
    }
}

App.prototype.addController = function (hostname) {
    this.controllers[hostname] = new NxRemoteController(hostname);
}

App.prototype.removeController = function (hostname) {
    var controller = this.controllers[hostname];
    controller.stopScreen();
    controller.destroy();
    delete this.controllers[hostname];
    this.controlPanel.updateTitle();
}

App.prototype.toggleFullscreen = function () {
    var self = this;
    if (!document.fullscreenElement &&    // alternative standard method
        !document.mozFullScreenElement &&
        !document.webkitFullscreenElement &&
        !document.msFullscreenElement ) {  // current working methods
        if (document.documentElement.requestFullscreen) {
            document.documentElement.requestFullscreen();
        } else if (document.documentElement.msRequestFullscreen) {
            document.documentElement.msRequestFullscreen();
        } else if (document.documentElement.mozRequestFullScreen) {
            document.documentElement.mozRequestFullScreen();
        } else if (document.documentElement.webkitRequestFullscreen) {
            document.documentElement.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
        }
    } else {
        if (document.exitFullscreen) {
            document.exitFullscreen();
        } else if (document.msExitFullscreen) {
            document.msExitFullscreen();
        } else if (document.mozCancelFullScreen) {
            document.mozCancelFullScreen();
        } else if (document.webkitExitFullscreen) {
            document.webkitExitFullscreen();
        }
    }
}
