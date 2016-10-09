function LiveView(controller) {
    this.controller = controller;
    this.started = false;
    this.quality = false;
    this.timeoutInterval = 100;
    this.target = null;
    this.cvs = null;
    this.ctx = null;
    this.timeout = null;
    this.init();
}

LiveView.prototype.init = function () {
    this.target = $('<canvas class="liveview"></canvas>');
    if (this.controller.isNx300()) {
        this.target.css('margin-left', '5%');
    }
    this.controller.viewFinder.addLiveViewCanvas(this.target);
    this.cvs = this.target[0];
    this.ctx = this.cvs.getContext("2d");

    if (this.controller.isNx1() || this.controller.isNx500()) {
        this.cvs.width = 720;
    } else {
        this.cvs.width = 800; // NX300
    }
    this.cvs.height = 480;
    this.controller.viewFinder.canvasContainer[0].width = this.cvs.width;
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

LiveView.drawBlack = function (buffer) {
    for (var i = 0; i < buffer.length; i++) {
        if (i % 4 != 3) {
            buffer[i] = 0;
        } else {
            buffer[i] = 255;
        }
    }
}

LiveView.draw = function (cvs, ctx, nv12, width, height, low) {
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
        LiveView.drawBlack(buffer);
        ctx.putImageData(imageData, 0, 0);
        ctx.putImageData(imageData, (cvs.width - width) / 2 + width, 0);
    } else if (height != cvs.height) {
        imageData = ctx.createImageData(width, (cvs.height - height) / 2);
        buffer = imageData.data;
        LiveView.drawBlack(buffer);
        ctx.putImageData(imageData, 0, 0);
        ctx.putImageData(imageData, 0, (cvs.height - height) / 2 + height);
    }
}

LiveView.clearLiveView = function (cvs, ctx) {
    imageData = ctx.getImageData(0, 0, cvs.width, cvs.height);
    buffer = imageData.data;
    for (var i = 0; i < buffer.length; i++) {
        buffer[i] = 0;
    }
    ctx.putImageData(imageData, 0, 0);
}

LiveView.prototype.getData = function (low) {
    var self = this;
    var url = this.controller.createUrl('/api/v1/liveview/get');

    if (low) {
        url = this.controller.createUrl('/api/v1/liveview/get_low');
    }

    this.quality = low;

    $.ajax({
        url: url,
        timeout: 3000,
        success: function(str) {
            var nv12 = [];
            for (var i = 0; i < str.length; i++) {
                nv12.push(str.charCodeAt(i));
            }

            if (self.controller.isNx300()) {
                if (self.cvs.width != 800) {
                    self.cvs.width = 800;
                }
            } else { // NX1 or NX500
                if (self.cvs.width != 720) {
                    self.cvs.width = 720;
                }
            }
            if (self.cvs.height != 480) {
                self.cvs.height = 480;
            }

            //debug("nv12.length = " + nv12.length);
            if (nv12.length == 720 * 380 * 3 / 2) {
                LiveView.draw(self.cvs, self.ctx, nv12, 720, 380, false); // 4K video
            } else if (nv12.length == 720 * 380 * 3 / 2 / 2) {
                LiveView.draw(self.cvs, self.ctx, nv12, 720, 380, true); // 4K video (low)
            } else if (nv12.length == 720 * 404 * 3 / 2) {
                LiveView.draw(self.cvs, self.ctx, nv12, 720, 404, false); // UHD / FHD / HD video
            } else if (nv12.length == 720 * 404 * 3 / 2 / 2) {
                LiveView.draw(self.cvs, self.ctx, nv12, 720, 404, true); // UHD / FHD / HD video (low)
            } else if (nv12.length == 640 * 480 * 3 / 2) {
                LiveView.draw(self.cvs, self.ctx, nv12, 640, 480, false); // VGA video
            } else if (nv12.length == 640 * 480 * 3 / 2 / 2) {
                LiveView.draw(self.cvs, self.ctx, nv12, 640, 480, true); // VGA video (low)
            } else if (nv12.length == 720 * 480 * 3 / 2) {
                LiveView.draw(self.cvs, self.ctx, nv12, 720, 480, false); // Normal liveview
            } else if (nv12.length == 720 * 480 / 4 * 3) {
                LiveView.draw(self.cvs, self.ctx, nv12, 720, 480, true); // Normal liveview (low)
            } else if (nv12.length == 800 * 480 * 3 / 2) {
                LiveView.draw(self.cvs, self.ctx, nv12, 800, 480, false); // Normal liveview (NX300)
            } else if (nv12.length == 800 * 480 * 3 / 2 / 2) {
                LiveView.draw(self.cvs, self.ctx, nv12, 800, 480, true); // Normal liveview (NX300, low)
            } else {
                LiveView.clearLiveView(self.cvs, self.ctx);
            }

            if (self.started) {
                if (self.timeout != null) {
                    clearTimeout(self.timeout);
                }
                self.timeout = setTimeout(function () {
                    self.getData(self.quality)
                }, self.timeoutInterval);
            } else {
                LiveView.clearLiveView(self.cvs, self.ctx);
            }
        },
        error: function (request, status, error) {
            LiveView.clearLiveView(self.cvs, self.ctx);
            self.started = false;
        }
    });
}

LiveView.prototype.start = function (low) {
    if (!this.controller.viewFinder.panel.isCollapsed()) {
        if (!this.started) {
            this.started = true;
            this.getData(low);
        } else {
            this.quality = low;
        }
    }
    this.controller.settings.setLiveView(low ? "lq" : "hq");
}

LiveView.prototype.stop = function () {
    this.started = false;
}

LiveView.prototype.setHide = function () {
    this.controller.settings.setLiveView('hide');
}

LiveView.prototype.setTimeoutInterval = function (interval) {
    this.timeoutInterval = interval;
}

LiveView.prototype.destroy = function () {
    if (self.timeout != null) {
        clearTimeout(self.timeout);
        self.timeout = null;
    }
}
