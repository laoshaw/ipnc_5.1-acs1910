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

var csd = String(xdc.csd()).replace(/\\/g,"/");
var fileModule = xdc.module('xdc.services.io.File');
var logic;

function setLogic(log)
{
    logic = log;
}

/**
 * Logic to save userInput to an XML file
 * @param userInput
 * @return text to be saved to XML file
/*
 *  ======== saveXml ========
 */
function saveXml(algEnv)
{
    algEnv = logic.argsValidation(algEnv);
    try {
        var doc = <xdais_genalg_wizard/>;

        doc.packageName = algEnv.packageName;
        doc.moduleName = algEnv.moduleName;
        doc.vendorName = algEnv.vendorName;
        doc.interfaceName = algEnv.interfaceName;
        doc.templatesDir = algEnv.templatesDir;
        doc.templateName = algEnv.templateName;
        doc.outputRepository = algEnv.output;
        var targets = <targets/>;
        var targ = <target/>;
        targ.@target = algEnv.target;
        targ.@name = algEnv.targname;
        targ.@suffix = algEnv.targsuffix;
        targets.appendChild(targ);
        doc.appendChild(targets);
        doc.genCCSProj = algEnv.genCCSProj;
        doc.cgTools = algEnv.cgTools;
        doc.rulesCheck = algEnv.rulesCheck == true ? 'true' : 'false';
        doc.expertCheck = algEnv.expertCheck == true ? 'true' : 'false';

        var version = "<?xml version=\"1.0\"?>";
        return(version + doc.toXMLString());
    }
    catch (e) {
        print('exception ' + e.message);
        return ('');
    }
}


/**
 * Writes text to a file
 * @param {String} Output file location
 * @param {String} Text to be written to file
 * @return {Integer} 0 for completion, 1 for error
 */
function textFileWriter( outputFileLocation, fileText ) {
    var file = fileModule.open( outputFileLocation, "r" );
    file = fileModule.open( outputFileLocation, "w" );
    var completed = file.writeLine( fileText.toString() );
    file.close();
    if (completed==false) {
        return -1;
    }
    return 0;
}

function textFileReader( inputFileLocation ) {
    var text = "";
    var file = fileModule.open(inputFileLocation, "r");
    var line;
    while ((line = file.readLine()) != null) {
          text += line + "\n";
    }
    file.close();
    return text;
}

/**
* Logic to update userInput based on XML file
* @param XML file containing userInput data
* @return updated userInput based on XML file
*/
function loadXml(file, algEnv)
{
    var readXml = textFileReader(file);
    var doc = new XML(readXml);

    algEnv.wizardVersion = String(doc.version);
    algEnv.packageName = String(doc.packageName);
    algEnv.moduleName = String(doc.moduleName);
    algEnv.vendorName = String(doc.vendorName);
    algEnv.interfaceName = String(doc.interfaceName);
    algEnv.templatesDir = String(doc.templatesDir);
    algEnv.templateName = String(doc.templateName);
    algEnv.output = String(doc.outputRepository);
    algEnv.target = String(doc.targets.target[0].@target);
    algEnv.targname = String(doc.targets.target[0].@name);;
    algEnv.targsuffix = String(doc.targets.target[0].@suffix);;
    algEnv.genCCSProj = (doc.genCCSProj == 'true') ? true : false;
    algEnv.cgTools = String(doc.cgTools);
    algEnv.rulesCheck = doc.rulesCheck == 'true' ? true : false;
    algEnv.expertCheck = doc.expertCheck == 'true' ? true : false;

    return algEnv;
}

/**
* A relative path in a saved XML file (used in the examples dir) must be
* evaluated
* @param {String} Exact string from XML file
* @return {String} Canonical Path of Input
*/
function relativePathCheck(path)
{
    path = String(path);
    if (path.length) {
        if (!java.io.File(path).isAbsolute()) {
            var canonicalPath = csd + path.substring(1, path.length);
        }
        else {
            var canonicalPath = path;
        }
        canonicalPath = String(java.io.File(canonicalPath).getCanonicalPath());
        return (canonicalPath.replace(/\\/g,"/"));
    }
    return ("");
}



/*
 *  @(#) ti.xdais.wizards.genalg; 1, 0, 0, 0,31; 5-4-2010 08:46:33; /db/wtree/library/trees/dais/dais-q11x/src/
 */

