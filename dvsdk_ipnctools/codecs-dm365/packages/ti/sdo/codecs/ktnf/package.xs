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
        if ( this.KTNF.watermark == false ) {
                lib = "lib/libktnf.a";
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
        template = "ti/sdo/codecs/ktnf/link.xdt";
    }

    return (template);
}
/*
 *  ======== VICPSYNC ========
 */
function close() {
xdc.useModule('ti.sdo.fc.vicpsync.VICPSYNC');
}
