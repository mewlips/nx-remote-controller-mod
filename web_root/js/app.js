var app = null;

function App() {
    this.controller = null;
    if (document.location.hostname == '' ||
        document.location.hostname == 'localhost' ||
        document.location.hostname == '127.0.0.1') {
        var hostname = prompt("Enter ip address of your camera", '');
        this.controller = new NxRemoteController(hostname);
    } else {
        this.controller = new NxRemoteController();
    }

    //this.viewfinder = new ViewFinder();
}

App.init = function () {
    app = new App();
}
