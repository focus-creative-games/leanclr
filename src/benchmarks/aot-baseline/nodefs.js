// Test-only Node host access; never used in browser/product builds.
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
    FS.mkdir('/host');
    FS.mount(NODEFS, {root: process.cwd()}, '/host');
    FS.chdir('/host');
});
