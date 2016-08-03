function convertYUVtoRGB(y, u, v) {
    var r, g, b;

    r = y + 1.402 * v + 0.5;
    g = y - (0.344 * u + 0.714 * v);
    b = y + 1.772 * u + 0.5;

    r = r>255? 255 : r<0 ? 0 : r;
    g = g>255? 255 : g<0 ? 0 : g;
    b = b>255? 255 : b<0 ? 0 : b;

    return {r:r, g:g, b:b};
}

function nv12toRgba(nv12, rgba, width, height) {
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

        var rgb = convertYUVtoRGB(y1, u, v);
        rgba[i*4] = rgb.r;
        rgba[i*4+1] = rgb.g;
        rgba[i*4+2] = rgb.b;
        rgba[i*4+3] = 255;

        rgb = convertYUVtoRGB(y2, u, v);
        rgba[i*4+4] = rgb.r;
        rgba[i*4+5] = rgb.g;
        rgba[i*4+6] = rgb.b;
        rgba[i*4+7] = 255;

        rgb = convertYUVtoRGB(y3, u, v);
        rgba[width*4+i*4] = rgb.r;
        rgba[width*4+i*4+1] = rgb.g;
        rgba[width*4+i*4+2] = rgb.b;
        rgba[width*4+i*4+3] = 255;

        rgb = convertYUVtoRGB(y4, u, v);
        rgba[width*4+i*4+4] = rgb.r;
        rgba[width*4+i*4+5] = rgb.g;
        rgba[width*4+i*4+6] = rgb.b;
        rgba[width*4+i*4+7] = 255;

        if (i != 0 && (i+2) % width == 0) {
            i += width;
        }
    }
}

function nv12toRgba2(nv12, rgba, width, height) {
    var size = (width / 2) * (height / 2);
    var offset = size;
    var y, u, v;

    for (var i = 0, j = 0, k = 0; i < size; i++, k += 2) {
        y = nv12[i] & 0xff;
        u = (nv12[offset + k] & 0xff) - 128;
        v = (nv12[offset + k + 1] & 0xff) - 128;

        var rgb = convertYUVtoRGB(y, u, v);
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

function getLiveview() {
    var cvs = document.getElementById("liveview");
    var ctx = cvs.getContext("2d");
    var imageData = ctx.createImageData(cvs.width, cvs.height);
    var buffer = imageData.data;
    $.ajax({
        url: '/api/v1/liveview/get',
        success: function(str) {
            var nv12 = [];
            for (var i = 0; i < str.length; i++) {
                nv12.push(str.charCodeAt(i));
            }

            if (nv12.length == 720 * 480 * 3 / 2) {
                nv12toRgba(nv12, buffer, 720, 480);
            } else if (nv12.length == 720 *480 / 4 * 3) {
                nv12toRgba2(nv12, buffer, 720, 480);
            }
            //debug("nv12.length = " + nv12.length);
            ctx.putImageData(imageData, 0, 0);

            //getLiveview();
        }
    });
}

