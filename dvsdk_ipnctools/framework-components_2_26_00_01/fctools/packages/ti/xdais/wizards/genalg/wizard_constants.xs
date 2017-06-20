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
 * 
 */
var wizardVersion = "6.25.02";

var INIT_MESSAGE = "XDM GenAlg Wizard v" + wizardVersion
    + "\nTexas Instruments";

var INSTRUCTIONS0 =
'Instructions for GenAlg\n'
+'\n'
+'Module: The name of the algorithm module\n\n'
+'Vendor: The name of the algorithm vendor\n\n'
+'Interface: Has the form I<Vendor>.  This field should be automatically '
+'filled after naming the module.\n\n'
+'Base Interface: The ti.xdais.dm interface that is being implemented\n\n'
+'Package Name: Edit this field to specify the algorithm package\n\n'
+'Destination Directory: The repository to create the algorithm package in\n\n'
+"C6000 TI 'cgtools' Dir: The location of C6000 TI CodeGen tools\n\n"
+'Expert Mode: When left unchecked, the Interface and Package Name '
+'fields will be read-only and will be automatically generated based '
+'on Module and Vendor\n\n'
+'For more information, see: '
+'<a href="http://tiexpressdsp.com/index.php/XDM_GenAlg_Wizard_FAQ">XDM GenAlg Wizard FAQ</a>';


var SAVE_OPEN_INSTRUCTIONS = "To save the information into into the GUI, use "
    + "File->Save. Specify the output location and file name. You may then "
    + "re-use this file in the GUI via File->Open or on the command line with "
    + "the -g -f=/yourfilename options (see xs ti.xdais.wizards.genalg --help "
    + "for more information on command line usage.)";

var ABOUT_MESSAGE = INIT_MESSAGE;

/* abridged instructions to show at the top of each panel */
var BANNER0 =  "Specify Module, Vendor, Interface and Package Name." +
" Select a Base Interface and Target. Specify a valid Destination" +
" Directory and C6000 TI 'cgtools' Dir." +
" See Help for detailed information about field input. ";

var targetList = ['ti.targets.C64P', 'ti.targets.C674'];

var ENTER_CGTOOLS = '<C6000 TI codegen tools directory>';
var ENTER_OUTREPO = '<Algorithm package destination repository>';

var TEMPLATES_DIR_HELP =
'The Templates directory contains subdirectories of XDC templates. '
+ 'Each template generates an XDM algorithm package that '
+ 'implements an XDM interface. The name of each subdirectory is '
+ "the name of the XDM interface, and is displayed in the 'Base Interface' "
+ 'combo box. For more information, see: '
+ '<a href="http://rtsc.eclipse.org/docs-tip/XDCscript_Language_Reference#Templates">XDCScript Templates</a>';

var CGTOOLS_DESC = 'Your TI C6000 codegen tools directory, e.g.\n' +
'"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/c6000"';

var GENALG_MODULE_HOVER = 'The name of the algorithm module';
var GENALG_VENDOR_HOVER = 'The name of the algorithm vendor';
var GENALG_INTERFACE_HOVER = 'The name of the algorithm interface.\nHas the form I<Module>';
var GENALG_XDM_ALG_PKG_HOVER = 'The name of the algorithm package';
var GENALG_TARGET = "The algorithm's build target";
var OUTPUT_HOVER = 'Destination directory. The generated package will be in:\n'
+'<Destination Directory>/<Package Name with.dots.turned.into.forward.slashes>';

var EXPERT_HOVER = 'When left unchecked, the Interface and Package Name '
+'fields will be read-only\nand will be automatically generated based '
+'on Module and Vendor';
/*
 *  @(#) ti.xdais.wizards.genalg; 1, 0, 0, 0,31; 5-4-2010 08:46:33; /db/wtree/library/trees/dais/dais-q11x/src/
 */

