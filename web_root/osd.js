function Osd(controller) {
    this.controller = controller;
    this.mouseDowned = false;
    this.started = false;
    this.hashs = [];
    this.scale = 1.0;
    this.timeoutInterval = 10;
}

Osd.prototype.setup = function () {
    this.onMouseDown();
    this.onMouseMove();
    this.onMouseUp();
    this.onWheel();

    return this;
}

Osd.prototype.onMouseDown = function () {
    var self = this;
    function mouseDown(ev, osd, mouse) {
        ev.preventDefault();
        var parentOffset = osd.parent().offset();
        var pageX;
        var pageY;
        if (mouse) {
            pageX = ev.pageX;
            pageY = ev.pageY;
        } else {
            pageX = ev.originalEvent.touches[0].pageX;
            pageY = ev.originalEvent.touches[0].pageY;
        }
        var cvs = document.getElementById("osd");
        self.scale = cvs.width / $('#cameraScreen').width();
        var x = Math.floor((pageX - parentOffset.left) * self.scale);
        var y = Math.floor((pageY - parentOffset.top) * self.scale);
        self.mouseDowned = true;
        self.controller.input.onMouseMove(x, y);
        self.controller.input.onMouseDown();
    }
    $('#osd').on('mousedown', function (ev) {
        mouseDown(ev, $(this), true);
    });
    $('#osd').on('touchstart', function (ev) {
        mouseDown(ev, $(this), false);
    });
}

Osd.prototype.onMouseMove = function () {
    var self = this;
    function mouseMove(ev, osd, mouse) {
        ev.preventDefault();
        if (self.mouseDowned == true) {
            var parentOffset = osd.parent().offset();
            var pageX;
            var pageY;
            if (mouse) {
                pageX = ev.pageX;
                pageY = ev.pageY;
            } else {
                pageX = ev.originalEvent.touches[0].pageX;
                pageY = ev.originalEvent.touches[0].pageY;
            }
            var x = Math.floor((pageX - parentOffset.left) * self.scale);
            var y = Math.floor((pageY - parentOffset.top) * self.scale);
            self.controller.input.onMouseMove(x, y);
        }
    }

    $('#osd').on('mousemove', function (ev) {
        mouseMove(ev, $(this), true);
    });
    $('#osd').on('touchmove', function (ev) {
        mouseMove(ev, $(this), false);
    });
}

Osd.prototype.onMouseUp = function () {
    var self = this;
    function mouseUp(ev, mouse) {
        ev.preventDefault();
        self.controller.input.onMouseUp();
        self.mouseDowned = false;
    }

    $('#osd').on('mouseup', function (ev) {
        mouseUp(ev, true);
    });
    $('#osd').on('touchcancel', function (ev) {
        mouseUp(ev, false);
    });
    $('#osd').on('touchend', function (ev) {
        mouseUp(ev, false);
    });
}

Osd.prototype.onWheel = function () {
    var self = this;
    $('#osd').bind('DOMMouseScroll mousewheel', function(e) {
        if (e.originalEvent.wheelDelta > 0 || e.originalEvent.detail < 0) {
            self.controller.input.onKey('UP');
        } else {
            self.controller.input.onKey('DOWN');
        }
        return false;
    });
}

function toHex(d) {
    return  ("0000000"+(Number(d).toString(16))).slice(-8)
}

Osd.prototype.clearHashs = function () {
    var numBlocks = 120;
    this.hashs = [];
    for (var i = 0; i < numBlocks; i++) {
        this.hashs.push(0);
    }
}

Osd.prototype.getData = function () {
    var self = this;
    var cvs = document.getElementById("osd");
    var ctx = cvs.getContext("2d");
    var hashs_str = '';
    for (var i = 0; i < this.hashs.length; i++) {
        hashs_str += toHex(this.hashs[i]);
    }

    function clearOsd() {
        imageData = ctx.getImageData(0, 0, cvs.width, cvs.height);
        buffer = imageData.data;
        for (var i = 0; i < buffer.length; i++) {
            buffer[i] = 0;
        }
        ctx.putImageData(imageData, 0, 0);
        self.clearHashs();
    }

    $.ajax({
        url: self.controller.createUrl('/api/v1/osd/get'),
        type: 'POST',
        data: { "hashs" : hashs_str },
        timeout: 1000,
        success: function(str) {
            if (str.length == 0) {
                setTimeout(function () {
                    self.getData();
                }, self.timeoutInterval);
                return;
            }

            var data = [];
            var index = 0;
            for (var i = 0; i < str.length; i++) {
                data.push(str.charCodeAt(i));
            }
            var frameWidth  = (data[index] & 0xff) << 8 | (data[index+1] & 0xff);
            index += 2;
            var frameHeight = (data[index] & 0xff) << 8 | (data[index+1] & 0xff);
            index += 2;
            var blockWidth  = (data[index] & 0xff) << 8 | (data[index+1] & 0xff);
            index += 2;
            var blockHeight = (data[index] & 0xff) << 8 | (data[index+1] & 0xff);
            index += 2;

            if (cvs.width != frameWidth) {
                cvs.width = frameWidth;
            }
            if (cvs.height != frameHeight) {
                cvs.height = frameHeight;
            }

            var imageData = ctx.getImageData(0, 0, frameWidth, frameHeight);
            var buffer = imageData.data;

            for (var i = 0; i < frameWidth * frameHeight / blockWidth / blockHeight; i++) {
                var xOffset = (data[index] & 0xff) << 8 | (data[index+1] & 0xff);
                index += 2;
                var yOffset = (data[index] & 0xff) << 8 | (data[index+1] & 0xff);
                index += 2;
                var blockIndex = ((yOffset / blockHeight) * (frameWidth / blockWidth))
                                   + (xOffset / blockWidth);
                if (xOffset == 0xffff && yOffset == 0xffff) {
                    // end of blocks
                    break;
                }
                self.hashs[blockIndex] = (data[index  ] & 0xff) << 24 |
                                         (data[index+1] & 0xff) << 16 |
                                         (data[index+2] & 0xff) <<  8 |
                                         (data[index+3] & 0xff);
                index += 4;
                for (var j = 0; j < blockHeight; j++) {
                    for (var k = 0; k < blockWidth; k++) {
                        var bufferIndex = ((yOffset + j) * frameWidth * 4) + ((xOffset + k) * 4);
                        var dataIndex = index + (j * blockWidth * 4) + (k * 4);
                        buffer[bufferIndex + 0] = data[dataIndex + 0] & 0xff;
                        buffer[bufferIndex + 1] = data[dataIndex + 1] & 0xff;
                        buffer[bufferIndex + 2] = data[dataIndex + 2] & 0xff;
                        buffer[bufferIndex + 3] = data[dataIndex + 3] & 0xff;
                    }
                }
                index += blockWidth * blockHeight * 4;
            }
            ctx.putImageData(imageData, 0, 0);

            if (self.started) {
                setTimeout(function () {
                    self.getData();
                }, self.timeoutInterval);
            } else {
                clearOsd();
            }
        },
        error: function (request, status, error) {
            clearOsd();
            self.started = false;
        }
    });
}

Osd.prototype.start = function () {
    if (!this.started) {
        this.clearHashs();
        this.started = true;
        this.getData();
    }
    if (typeof(Storage) !== "undefined") {
        localStorage.setItem("osd", "show");
    }
}

Osd.prototype.stop = function () {
    this.started = false;
    if (typeof(Storage) !== "undefined") {
        localStorage.setItem("osd", "hide");
    }
}

Osd.prototype.setTimeoutInterval = function (interval) {
    this.timeoutInterval = interval;
}
