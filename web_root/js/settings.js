function Settings(macAddress) {
    this.cameraId = macAddress;
}

Settings.init = function () {
    var li = $('<li></li>')
        .append($('<a href="#"></a>')
                .append('Screen off on video recording ' +
                        '<span id="screenOffOnRecordCheck" ' +
                        '      class="glyphicon glyphicon-ok"></span>'))
        .click(function () {
            var screenOffOnRecord = Settings.getScreenOffOnRecord();
            if (screenOffOnRecord) {
                $('#screenOffOnRecordCheck').hide();
                Settings.setScreenOffOnRecord(false);
            } else {
                $('#screenOffOnRecordCheck').show();
                Settings.setScreenOffOnRecord(true);
            }
        });
    function setCheck() {
        if (Settings.getScreenOffOnRecord()) {
            $('#screenOffOnRecordCheck').show();
        } else {
            $('#screenOffOnRecordCheck').hide();
        }
    }
    $('#settingsMenu')
        .prepend(li)
        .click(function () {
            setCheck()
        });
    setCheck();
}

Settings.reset = function () {
    if (typeof(Storage) !== "undefined") {
        localStorage.clear();
    }
}

Settings.getScreenOffOnRecord = function () {
    return localStorage.getItem('screen_off_on_record') === 'true';
}

Settings.setScreenOffOnRecord = function (value) {
    if (value === true) {
        localStorage.setItem('screen_off_on_record', 'true');
    } else {
        localStorage.setItem('screen_off_on_record', 'false');
    }
}

Settings.getControlColSize = function () {
    return localStorage.getItem('control_col_size') || 'col-sm-4';
}

Settings.setControlColSize = function (value) {
    localStorage.setItem('control_col_size', value);
}

Settings.getNameByMacAddress = function (macAddress) {
    return localStorage.getItem(macAddress + '-name') || '';
}

Settings.prototype.getLiveView = function () {
    return localStorage.getItem(this.cameraId + '-liveview') || 'hq';
}

Settings.prototype.setLiveView = function (value) {
    localStorage.setItem(this.cameraId + '-liveview', value);
}

Settings.prototype.getOsd = function () {
    return localStorage.getItem(this.cameraId + '-osd') || 'show';
}

Settings.prototype.setOsd = function (value) {
    localStorage.setItem(this.cameraId + '-osd', value);
}

Settings.prototype.getName = function () {
    return localStorage.getItem(this.cameraId + '-name') || '';
}

Settings.prototype.setName = function (value) {
    localStorage.setItem(this.cameraId + '-name', value);
}

Settings.prototype.getColSize = function () {
    return localStorage.getItem(this.cameraId + '-col_size') || 'col-sm-8';
}
    
Settings.prototype.setColSize = function (value) {
    localStorage.setItem(this.cameraId + '-col_size', value);
}

Settings.prototype.getKeyInputEnabled = function () {
    return localStorage.getItem(this.cameraId + '-key_input_enabled') === 'true';
}

Settings.prototype.setKeyInputEnabled = function (value) {
    if (value === true) {
        localStorage.setItem(this.cameraId + '-key_input_enabled', 'true');
    } else {
        localStorage.setItem(this.cameraId + '-key_input_enabled', 'false');
    }
}
