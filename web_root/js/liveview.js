function LiveView(controller) {
    this.controller = controller;
    this.started = false;
    this.quality = false;
    this.timeoutInterval = 100;
    this.target = null;

    this.init();
}

LiveView.prototype.init= function () {
    var targetId = this.controller.isMainController
                    ? 'liveview-main'
                    : 'liveview-' + this.controller.hostname.replace(/\./g, '-');
    this.target = $('<canvas class="liveview"></canvas>').attr('id', targetId);
    this.controller.target.append(this.target);
}

LiveView.convertYUVtoRGB = function (y, u, v) {
    var r, g, b;

    r = y + 1.402 * v + 0.5;
    g = y - (0.344 * u + 0.714 * v);
    b = y + 1.772 * u + 0.5;

    r = r>255? 255 : r<0 ? 0 : r;
    g = g>255? 255 : g<0 ? 0 : g;
    b = b>255? 255 : b<0 ? 0 : b;

    return {r:r, g:g, b:b};
}

LiveView.nv12toRgba = function (nv12, rgba, width, height) {
    var size = width * height;
    var offset = size;
    var u, v, y1, y2, y3, y4;

    for (var i = 0, k = 0; i < size; i += 2, k += 2) {
        y1 = nv12[i] & 0xff;
        y2 = nv12[i + 1] & 0xff;
        y3 = nv12[width + i] & 0xff;
        y4 = nv12[width + i + 1] & 0xff;

        u = nv12[offset + k] & 0xff;
        v = nv12[offset + k + 1] & 0xff;
        u = u - 128;
        v = v - 128;

        var rgb = LiveView.convertYUVtoRGB(y1, u, v);
        rgba[i*4] = rgb.r;
        rgba[i*4+1] = rgb.g;
        rgba[i*4+2] = rgb.b;
        rgba[i*4+3] = 255;

        rgb = LiveView.convertYUVtoRGB(y2, u, v);
        rgba[i*4+4] = rgb.r;
        rgba[i*4+5] = rgb.g;
        rgba[i*4+6] = rgb.b;
        rgba[i*4+7] = 255;

        rgb = LiveView.convertYUVtoRGB(y3, u, v);
        rgba[width*4+i*4] = rgb.r;
        rgba[width*4+i*4+1] = rgb.g;
        rgba[width*4+i*4+2] = rgb.b;
        rgba[width*4+i*4+3] = 255;

        rgb = LiveView.convertYUVtoRGB(y4, u, v);
        rgba[width*4+i*4+4] = rgb.r;
        rgba[width*4+i*4+5] = rgb.g;
        rgba[width*4+i*4+6] = rgb.b;
        rgba[width*4+i*4+7] = 255;

        if (i != 0 && (i+2) % width == 0) {
            i += width;
        }
    }
}

LiveView.nv12toRgba2 = function (nv12, rgba, width, height) {
    var size = (width / 2) * (height / 2);
    var offset = size;
    var y, u, v;

    for (var i = 0, j = 0, k = 0; i < size; i++, k += 2) {
        y = nv12[i] & 0xff;
        u = (nv12[offset + k] & 0xff) - 128;
        v = (nv12[offset + k + 1] & 0xff) - 128;

        var rgb = LiveView.convertYUVtoRGB(y, u, v);
        rgba[j+0] = rgb.r;
        rgba[j+1] = rgb.g;
        rgba[j+2] = rgb.b;
        rgba[j+3] = 255;

        rgba[j+4] = rgb.r;
        rgba[j+5] = rgb.g;
        rgba[j+6] = rgb.b;
        rgba[j+7] = 255;

        rgba[j+0+width*4] = rgb.r;
        rgba[j+1+width*4] = rgb.g;
        rgba[j+2+width*4] = rgb.b;
        rgba[j+3+width*4] = 255;

        rgba[j+4+width*4] = rgb.r;
        rgba[j+5+width*4] = rgb.g;
        rgba[j+6+width*4] = rgb.b;
        rgba[j+7+width*4] = 255;

        j+=8;

        if (i != 0 && (i+1) % (width / 2) == 0) {
            j += width * 4;
        }
    }
}

LiveView.prototype.getData = function (low) {
    var self = this;
    var cvs = this.target[0];
    var ctx = cvs.getContext("2d");
    var url = this.controller.createUrl('/api/v1/liveview/get');

    if (low) {
        url = this.controller.createUrl('/api/v1/liveview/get_low');
    }

    this.quality = low;

    function drawBlack(buffer) {
        for (var i = 0; i < buffer.length; i++) {
            if (i % 4 != 3) {
                buffer[i] = 0;
            } else {
                buffer[i] = 255;
            }
        }
    }

    function draw(nv12, width, height, low) {
        var imageData = ctx.createImageData(width, height);
        var buffer = imageData.data;
        if (low) {
            LiveView.nv12toRgba2(nv12, buffer, width, height);
        } else {
            LiveView.nv12toRgba(nv12, buffer, width, height);
        }
        ctx.putImageData(imageData, (cvs.width - width) / 2,
                                    (cvs.height - height) / 2);
        if (width != cvs.width) {
            imageData = ctx.createImageData((cvs.width - width) / 2, height);
            buffer = imageData.data;
            drawBlack(buffer);
            ctx.putImageData(imageData, 0, 0);
            ctx.putImageData(imageData, (cvs.width - width) / 2 + width, 0);
        } else if (height != cvs.height) {
            imageData = ctx.createImageData(width, (cvs.height - height) / 2);
            buffer = imageData.data;
            drawBlack(buffer);
            ctx.putImageData(imageData, 0, 0);
            ctx.putImageData(imageData, 0, (cvs.height - height) / 2 + height);
        }
    }

    function clearLiveview() {
        imageData = ctx.getImageData(0, 0, cvs.width, cvs.height);
        buffer = imageData.data;
        for (var i = 0; i < buffer.length; i++) {
            buffer[i] = 0;
        }
        ctx.putImageData(imageData, 0, 0);
    }

    $.ajax({
        url: url,
        timeout: 1000,
        success: function(str) {
            var imageData;
            var buffer;
            var nv12 = [];
            for (var i = 0; i < str.length; i++) {
                nv12.push(str.charCodeAt(i));
            }

            if (self.controller.isNx300()) {
                if (cvs.width != 800) {
                    cvs.width = 800;
                }
            } else { // NX1 or NX500
                if (cvs.width != 720) {
                    cvs.width = 720;
                }
            }
            if (cvs.height != 480) {
                cvs.height = 480;
            }

            //debug("nv12.length = " + nv12.length);
            if (nv12.length == 720 * 380 * 3 / 2) {
                draw(nv12, 720, 380, false); // 4K video
            } else if (nv12.length == 720 * 380 * 3 / 2 / 2) {
                draw(nv12, 720, 380, true); // 4K video (low)
            } else if (nv12.length == 720 * 404 * 3 / 2) {
                draw(nv12, 720, 404, false); // UHD / FHD / HD video
            } else if (nv12.length == 720 * 404 * 3 / 2 / 2) {
                draw(nv12, 720, 404, true); // UHD / FHD / HD video (low)
            } else if (nv12.length == 640 * 480 * 3 / 2) {
                draw(nv12, 640, 480, false); // VGA video
            } else if (nv12.length == 640 * 480 * 3 / 2 / 2) {
                draw(nv12, 640, 480, true); // VGA video (low)
            } else if (nv12.length == 720 * 480 * 3 / 2) {
                draw(nv12, 720, 480, false); // Normal liveview
            } else if (nv12.length == 720 * 480 / 4 * 3) {
                draw(nv12, 720, 480, true); // Normal liveview (low)
            } else if (nv12.length == 800 * 480 * 3 / 2) {
                draw(nv12, 800, 480, false); // Normal liveview (NX300)
            } else if (nv12.length == 800 * 480 * 3 / 2 / 2) {
                draw(nv12, 800, 480, true); // Normal liveview (NX300, low)
            } else {
                clearLiveview();
            }

            if (self.started) {
                if (self.quality == low) {
                    setTimeout(function () {
                        self.getData(low)
                    }, self.timeoutInterval);
                }
            } else {
                clearLiveview();
            }
        },
        error: function (request, status, error) {
            clearLiveview();
            self.started = false;
        }
    });
}

LiveView.prototype.start = function (low) {
    if (!this.started) {
        this.started = true;
        this.getData(low);
    }
    if (typeof(Storage) !== "undefined") {
        localStorage.setItem("liveview", low ? "lq" : "hq");
    }
}

LiveView.prototype.stop = function () {
    this.started = false;
    if (typeof(Storage) !== "undefined") {
        localStorage.setItem("liveview", "hide");
    }
}

LiveView.prototype.setTimeoutInterval = function (interval) {
    this.timeoutInterval = interval;
}
