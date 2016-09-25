function ViewFinder() {
    this.select = $('#viewfinderSelect');
    this.controllers = {};
    this.cameras = [];

    this.init();
}

ViewFinder.prototype.init = function () {
    var self = this;
    this.select.selectpicker('deselectAll');
    this.select.on('show.bs.select', function (event) {
        self.updateSelectPicker();
    });
    this.select.on('changed.bs.select',
        function (event, clickedIndex, newValue, oldValue) {
            var ip = event.target[0].value;
            if (newValue == true) {
                if (self.controllers[ip] == null) {
                    self.addController(ip);
                }
            } else {
                if (self.controllers[ip] != null) {
                    delete self.controllers[ip];
                }
            }

        }
    );
}

ViewFinder.prototype.setCameras = function (cameras) {
    this.cameras = cameras;
}

ViewFinder.prototype.updateSelectPicker = function () {
    this.select.empty();
    for (var i = 0; i < this.cameras.length; i++) {
        var camera = this.cameras[i];
        var option = $('<option></option>')
                        .text(camera.model + ' [' + camera.ip + ']')
                        .attr('value', camera.ip);
        this.select.append(option);

        // test
        var option2 = $('<option></option>')
                        .text(camera.model + ' [' + camera.ip + ']')
                        .attr('value', camera.ip);
        this.select.append(option2);
    }
    this.select.selectpicker('refresh');
}

ViewFinder.prototype.addController = function (ip) {
    var self = this;
    var targetId = 'viewfinder-' + ip.replace(/\./g, '-');
    console.log('targetId = ' + targetId);
    var target = $('<div></div>')
                    .attr('id', targetId)
                    .attr('class', 'col-sm-4');
    $('#viewfinderRow1').append(target);
    this.controllers[ip] = new NxRemoteController(ip, target);
}
