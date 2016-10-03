function Gallery(controllers) {
    this.controllers = controllers;
    this.iframe = null;
}

Gallery.prototype.init = function (url) {
    var self = this;

    this.iframe = $('<iframe></iframe>')
        .attr('id', 'galleryContent')
        .attr('src', url);
    $('#gallery').html(this.iframe);

    function resize_iframe()
    {
        var height = window.innerWidth; //Firefox
        if (document.body.clientHeight) {
            height=document.body.clientHeight; //IE
        }
        console.log('height = ' + height);
        if (self.iframe != null) {
            self.iframe.css('height', (height - 50) + 'px');
        }
    }
    resize_iframe();
    window.onresize = resize_iframe;
}
