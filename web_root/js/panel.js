function Panel(contextualClass, columnSizes, colSizeGetter, colSizeSetter) {
    this.colSizeGetter = colSizeGetter;
    this.colSizeSetter = colSizeSetter;

    this.target = $('<div></div>')
        .addClass('panel ' + contextualClass +
                  ' ' + colSizeGetter() +
                  ' col-xs-12');

    this.columnSizes = columnSizes;

    this.heading = $('<div></div>')
        .addClass('panel-heading')
        .appendTo(this.target);

    this.title = $('<span></span>')
        .css('white-space', 'nowrap')
        .appendTo(this.heading);

    this.body = $('<div></div>')
        .addClass('panel-body collapse in')
        .appendTo(this.target);
    this.bodyIsCollapsed = false;

    this.footer = null;

    this.initHeadingButtons();
}

Panel.prototype.initHeadingButtons = function () {
    var self = this;

    var buttonGroup = $('<div class="btn-group pull-left"></div>')
        .prependTo(this.heading);

    var collapseButton = $('<button type="button"></button>')
        .addClass('btn btn-default btn-xs')
        .append($('<span></span>')
                .addClass('glyphicon glyphicon-menu-up'))
        .click(function () {
            if (!self.isCollapsed()) {
                self.body.collapse('hide');
            } else {
                self.body.collapse('show');
            }
        })
        .appendTo(buttonGroup);
    this.body.on('hidden.bs.collapse', function() {
        collapseButton.html($('<span></span>')
                            .addClass('glyphicon glyphicon-menu-down'))
    });
    this.body.on('shown.bs.collapse', function() {
        collapseButton.html($('<span></span>')
                            .addClass('glyphicon glyphicon-menu-up'))
    });
    this.body.on('hide.bs.collapse', function() {
        self.bodyIsCollapsed = true;
    });
    this.body.on('show.bs.collapse', function() {
        self.bodyIsCollapsed = false;
    });


    $('<button type="button"></button>')
        .addClass('btn btn-default btn-xs')
        .append($('<span></span>')
                .addClass('glyphicon glyphicon-minus'))
        .click(function () {
            var classes = self.target.attr('class');
            var i;
            for (i = 0; i < self.columnSizes.length; i++) {
                var columnSize = self.columnSizes[i];
                if (classes.includes('col-sm-' + columnSize)) {
                    if (i == 0) {
                        // min size
                    } else {
                        self.target.removeClass('col-sm-' + columnSize);
                        var colSize = 'col-sm-' + self.columnSizes[i - 1];
                        self.colSizeSetter(colSize);
                        self.target.addClass(colSize);
                        self.setBodyButtonFontSize(colSize);
                    }
                }
            }
        })
        .appendTo(buttonGroup);

    $('<button type="button"></button>')
        .addClass('btn btn-default btn-xs')
        .append($('<span></span>')
                .addClass('glyphicon glyphicon-plus'))
        .click(function () {
            var classes = self.target.attr('class');
            var i;
            for (i = 0; i < self.columnSizes.length; i++) {
                var columnSize = self.columnSizes[i];
                if (classes.includes('col-sm-' + columnSize)) {
                    if (i == self.columnSizes.length - 1) {
                        // max size
                    } else {
                        self.target.removeClass('col-sm-' + columnSize);
                        var colSize = 'col-sm-' + self.columnSizes[i + 1];
                        self.colSizeSetter(colSize);
                        self.target.addClass(colSize);
                        self.setBodyButtonFontSize(colSize);
                    }
                }
            }
        })
        .appendTo(buttonGroup);

    $('<button type="button"></button>')
        .addClass('btn btn-default btn-xs')
        .append($('<span></span>')
                .addClass('glyphicon glyphicon-arrow-left'))
        .click(function () {
            var prev = self.target.prev();
            if (prev.length != 0) {
                self.target.detach().fadeIn('fast').insertBefore(prev);
            } else {
                self.target.detach().fadeIn('fast').appendTo($('#panelsArea'));
            }
        })
        .appendTo(buttonGroup);

    $('<button type="button"></button>')
        .addClass('btn btn-default btn-xs')
        .append($('<span></span>')
                .addClass('glyphicon glyphicon-arrow-right'))
        .click(function () {
            var next = self.target.next();
            if (next.length != 0) {
                self.target.detach().fadeIn('fast').insertAfter(next);
            } else {
                self.target.detach().fadeIn('fast').prependTo($('#panelsArea'));
            }
        })
        .appendTo(buttonGroup);
}

Panel.prototype.addFooter = function () {
    if (this.footer == null) {
        this.footer = $('<div></div>')
            .addClass('panel-footer')
            .appendTo(this.target);
    }
}

Panel.prototype.setTitle = function (title) {
    this.title.html(title);
}

Panel.prototype.setBody = function (body) {
    this.body.html(body);
    this.setBodyButtonFontSize(this.colSizeGetter())
}

Panel.prototype.setBodyButtonFontSize = function (colSize) {
    var colSizeNum = parseInt(colSize.replace(/col-sm-/, ''));
    if (colSizeNum == 3) {
        this.body.find('button').css('font-size', '9px');
    } else if (colSizeNum == 4) {
        this.body.find('button').css('font-size', '13px');
    } else if (colSizeNum <= 6) {
        this.body.find('button').css('font-size', '16px');
    } else if (colSizeNum <= 8) {
        this.body.find('button').css('font-size', '18px');
    } else {
        this.body.find('button').css('font-size', '20px');
    }
}

Panel.prototype.isCollapsed = function () {
    return this.bodyIsCollapsed;
}
