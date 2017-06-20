/*
 *  ======== package.xs ========
 *
 */
/* 

* Needed so lib link deck has this lib after CODEC.

*/

function close() {

     xdc.useModule('ti.sdo.fc.hdvicpsync.HDVICPSYNC');

}

/*
 *  ======== getLibs ========
 */
function getLibs(prog)
{
    var lib = null;
  
    if (prog.build.target.isa == "v5T") {        
        if ( this.IPRUN.watermark == true ) {
                lib = "lib/libvicpGlbce365.a;lib/libimgproc365.a;lib/libvicpDmva365.a;lib/libiprun.a;lib/libimx.a;lib/libalgfw.a";
        }
        else {
                lib = null;
        }
        print("    will link with " + this.$name + ":" + lib);
    }
    return (lib);
}

/*
 *  ======== getSects ========
 */
function getSects()
{
    var template = null;

    if (Program.build.target.isa == "v5T") {
        template = "ti/sdo/codecs/iprun/link.xdt";
    }

    return (template);
}
/*
 *  ======== VICPSYNC ========
 */
function close() {
xdc.useModule('ti.sdo.fc.vicpsync.VICPSYNC');
}
