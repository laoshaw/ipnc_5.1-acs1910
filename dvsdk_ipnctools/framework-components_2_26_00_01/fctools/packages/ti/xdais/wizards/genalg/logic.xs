/* 
 * Copyright (c) 2010, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * */

var xmlOps = xdc.loadCapsule("xmlOps.xs");
var shared_gui = xdc.loadCapsule("shared_gui.xs");
var wizard_constants = xdc.loadCapsule("wizard_constants.xs");
var mkpkg = xdc.module("xdc.tools.mkpkg.Main");
var csd = String(xdc.csd()).replace(/\\/g,"/");

function argsValidation(algEnv) {

    var proot = csd + '/..';
    var pn = 'ti.xdais.wizards.genalg';
    for (var i = 0; (i = pn.indexOf('.', i) + 1) > 0; ) {
            proot += '/..';
    }
    proot = String(java.io.File(proot).getCanonicalPath()).replace(/\\/g, '/');

    if ( algEnv.xmlFile != null && algEnv.xmlFile != "null") {
        if ( java.io.File( algEnv.xmlFile ).exists() ) {
            try {
                algEnv = xmlOps.loadXml( algEnv.xmlFile, algEnv );
            }
            catch(e) {
                gprint("Error: " + e, algEnv );
            }
        }
        else {
            gprint("XML File, " + algEnv.xmlFile + ", not found. "
                + "Wizard starting without loading XML File...",
                algEnv);
            algEnv.xmlFile = "null";
        }
    }

    if (!algEnv.moduleName) {
        algEnv.moduleName = 'MYMODULE';
    }

    if (!algEnv.vendorName) {
        algEnv.vendorName = 'MYCOMPANY';
    }

    if (!algEnv.interfaceName) {
        algEnv.interfaceName = 'I' + algEnv.moduleName;
    }

    if (!algEnv.packageName) {
        algEnv.packageName = 'mycompany.algs.' + algEnv.moduleName.toLowerCase();
    }

    if (algEnv.output == null || algEnv.output == 'null') {
        algEnv.output = wizard_constants.ENTER_OUTREPO;
    }
    if (!algEnv.templatesDir) {
        algEnv.templatesDir = proot + '/ti/xdais/dm/templates';
    }
    if (!algEnv.target) {
        algEnv.target = wizard_constants.targetList[0];
        try {
            var tmod = xdc.useModule(algEnv.target);
            algEnv.targname = tmod.name;
            algEnv.targsuffix = tmod.suffix;
        }
        catch (e) {
        }
    }

    if (algEnv.genCCSProj != true && algEnv.genCCSProj != false) {
        algEnv.genCCSProj = true;
    }

    if (!algEnv.xdaisInstall) {
        algEnv.xdaisInstall = proot.replace(/\/packages$/, '');
    }
    if (!algEnv.xdcInstall) {
        algEnv.xdcInstall = xdc.getPackageBase('xdc');
        algEnv.xdcInstall = algEnv.xdcInstall.replace(/\\/g, '/');
        algEnv.xdcInstall = algEnv.xdcInstall.replace(/\/+$/, '');
        algEnv.xdcInstall = algEnv.xdcInstall.replace(/\/packages\/xdc$/, '');
        algEnv.xdcVersOK = false;
        for each (var f in java.io.File(algEnv.xdcInstall + '/eclipse/plugins').listFiles()) {
            if (f.isFile()) {
                continue;
            }
            var name = String(f.getName());
            if (name.match(/^org.eclipse.rtsc.xdctools.core_/)) {
                algEnv.xdcVers = name.substr(31);
                if (Number(algEnv.xdcVers.substr(0, 4)) >= 3.16) {
                    algEnv.xdcVersOK = true;
                }
                break;
            }
        }
    }
    if (!algEnv.cgTools) {
        algEnv.cgTools = wizard_constants.ENTER_CGTOOLS
    }

    return algEnv;
}

function gprint(message, algEnv) {
    if (algEnv.gui == true) {
        shared_gui.createMessageBox(message, algEnv);
    }
    else{
        print(message);
    }
}

function createFiles(algEnv) {

    var status = 0;
    algEnv = argsValidation(algEnv);

    var outputDir = algEnv.output + '/' + algEnv.packageName.replace(/\./g,'/');
    outputDir = outputDir.replace("//", "/");
    outputDir = outputDir.replace(/\\/g,"/");
    if (java.io.File(outputDir).exists()) {
        var list = java.io.File(outputDir).list();
        if (list.length!=0) {
            if (!algEnv.gui ) {
                gprint("\nWarning: " + outputDir + " is not empty--files may be "
                    + "overwritten... Would you like to continue? y/n",
                    algEnv);
                var isr = new java.io.InputStreamReader(
                    java.lang.System["in"] );
                var br = new java.io.BufferedReader( isr );

                try{
                    var output = br.readLine();
                }
                catch(e) {}

                if (output == "n") {
                    gprint("Quitting now...", algEnv);
                    status = 1;
                }
                else if(output != "y") {
                    gprint("Invalid selection. Quitting now...", algEnv);
                    status = 1;
                }
            }
        }
    }

    if (status == 0) {
        /* function mkpkg( template_dir, output, args ) */
        try {
            var algObj = {};
            algObj.packageName = algEnv.packageName;
            algObj.module = algEnv.moduleName.toUpperCase();
            algObj.vendor = algEnv.vendorName.toUpperCase();
            algObj.inface = algEnv.interfaceName.toUpperCase();
            algObj.target = algEnv.target;
            algObj.targname = algEnv.targname;
            algObj.targsuffix = algEnv.targsuffix;
            algObj.xdaisInstall = algEnv.xdaisInstall;
            algObj.xdcInstall = algEnv.xdcInstall;
            algObj.cgTools = algEnv.cgTools;
            algObj.xdcVersOK = algEnv.xdcVersOK;

            mkpkg.mkpkg(algEnv.templatesDir + '/' + algEnv.templateName,
                        outputDir, algObj);

            mkpkg.mkpkg("ti/xdais/wizards/genalg/templates/build", outputDir,
                        algObj);

            if (algEnv.genCCSProj) {
                mkpkg.mkpkg("ti/xdais/wizards/genalg/templates/ccsv4", outputDir,
                             algEnv);
            }
        }
        catch (e) {
            gprint("An error while creating package " +
                    algEnv.packageName + ":\n" + e, algEnv);
            status = 1;
        }
    }
    return(status);
}

/*
 *  @(#) ti.xdais.wizards.genalg; 1, 0, 0, 0,31; 5-4-2010 08:46:33; /db/wtree/library/trees/dais/dais-q11x/src/
 */

