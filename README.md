Building: (all DEPS are in tree, no need to do the equivalent of gclient sync):

    git clone https://github.com/sgraham/nope.git
    cd nope
    regen
    ninja -C out\Release content_shell
    out\Release\content_shell

Pull in upstream blink with `git subtree` (at least while it's in its own repo).

![Nope](http://fc09.deviantart.net/fs70/f/2013/257/6/8/grumpy_cat__nope_by_imwithstoopid13-d624kvl.png "Nope")
