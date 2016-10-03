function MouseInput(controller) {
    this.controller = controller;
}

MouseInput.prototype.onMouseDown = function () {
    var self = this;
    $.ajax({
        url: self.controller.createUrl('/api/v1/input/inject?mousedown=1'),
        mimeType: 'text/html',
        success: function(data) {
        }
    });
}

MouseInput.prototype.onMouseMove = function (x, y) {
    var self = this;
    if (this.controller.isNx300()) {
        var temp = y;
        y = x;
        x = 480 - temp;
        y = y;
    }
    $.ajax({
        url: self.controller.createUrl('/api/v1/input/inject?mousemove=' + x + '-' + y),
        mimeType: 'text/html',
        success: function(data) {
        }
    });
}

MouseInput.prototype.onMouseUp = function () {
    var self = this;
    $.ajax({
        url: self.controller.createUrl('/api/v1/input/inject?mouseup=1'),
        mimeType: 'text/html',
        success: function(data) {
        }
    });
}
