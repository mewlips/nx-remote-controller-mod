function ControlPanel(controllers) {
    this.controllers = controllers;
    this.panel = null;
    this.panelIsAppended = false;

    this.init();
}

ControlPanel.prototype.init = function () {
    var self = this;

    this.panel = new Panel('panel-success',
                           [3,4,5,6,7,8,9,10,11,12],
                           Settings.getControlColSize,
                           Settings.setControlColSize);

    var controlBody = ControlPanel.controlBody;
    if (controlBody == null) {
        ControlPanel.controlBody = $('#controlPanel').detach();
    }
    this.panel.setBody(ControlPanel.controlBody.show());
}

ControlPanel.prototype.destroy = function () {
    ControlPanel.controlBody = this.panel.body.children().detach();
    this.panel.target.remove();
}

ControlPanel.prototype.updateTitle = function () {
    var title = '';
    var count = 0;
    var hostname;

    if (!this.panelIsAppended) {
        $('#panelsArea').append(this.panel.target);
        this.panelIsAppended = true;
    }

    for (hostname in this.controllers) {
        var controller = this.controllers[hostname];
        if (controller.isKeyInputEnabled()) {
            var name = controller.settings.getName();
            title += '&nbsp;[';
            if (name == '') {
                title += controller.hostname;
            } else {
                title += name;
            }
            title += ']';
            count++;
        }
    }
    if (count == 0) {
        title = '&nbsp;No camera selected.';
        this.panel.body.collapse('hide');
    } else {
        this.panel.body.collapse('show');
    }

    this.panel.setTitle(title);
}

ControlPanel.prototype.setVisibility = function () {
    var nx1Exist = false;
    var nx500Exist = false;
    var nx300Exist = false;

    var hostname;
    for (hostname in this.controllers) {
        var controller = this.controllers[hostname];
        if (!controller.isKeyInputEnabled()) {
            continue;
        }
        if (controller.isNx1()) {
            nx1Exist = true;
        } else if (controller.isNx500()) {
            nx500Exist= true;
        } else if (controller.isNx300()) {
            nx300Exist = true;
        }
    }

    if (nx1Exist && nx500Exist && nx300Exist) {
        this.panel.target.find('.nx1-nx500-only').hide();
        this.panel.target.find('.nx500-only').hide();
        this.panel.target.find('.nx300-only').hide();
    } else if (nx1Exist && nx500Exist && !nx300Exist) {
        this.panel.target.find('.nx1-nx500-only').show();
        this.panel.target.find('.nx500-only').hide();
        this.panel.target.find('.nx300-only').hide();
    } else if (nx1Exist && !nx500Exist && nx300Exist) {
        this.panel.target.find('.nx1-nx500-only').hide();
        this.panel.target.find('.nx500-only').hide();
        this.panel.target.find('.nx300-only').hide();
    } else if (nx1Exist && !nx500Exist && !nx300Exist) {
        this.panel.target.find('.nx1-nx500-only').show();
        this.panel.target.find('.nx500-only').hide();
        this.panel.target.find('.nx300-only').hide();
    } else if (!nx1Exist && nx500Exist && nx300Exist) {
        this.panel.target.find('.nx1-nx500-only').hide();
        this.panel.target.find('.nx500-only').hide();
        this.panel.target.find('.nx300-only').hide();
    } else if (!nx1Exist && nx500Exist && !nx300Exist) {
        this.panel.target.find('.nx1-nx500-only').show();
        this.panel.target.find('.nx500-only').show();
        this.panel.target.find('.nx300-only').hide();
    } else if (!nx1Exist && !nx500Exist && nx300Exist) {
        this.panel.target.find('.nx1-nx500-only').hide();
        this.panel.target.find('.nx500-only').hide();
        this.panel.target.find('.nx300-only').show();
    } else if (!nx1Exist && !nx500Exist && !nx300Exist) {
        this.panel.target.find('.nx1-nx500-only').hide();
        this.panel.target.find('.nx500-only').hide();
        this.panel.target.find('.nx300-only').hide();
    }
}
