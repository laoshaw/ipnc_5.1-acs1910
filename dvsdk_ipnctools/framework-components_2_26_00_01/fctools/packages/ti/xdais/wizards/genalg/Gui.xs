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

var swt;
var browser;
var custom;
var graphics;
var layout;
var widgets;
var events;
var dnd;
var program;

var display;
var image;
var algEnv = null;

var xmlOps = xdc.loadCapsule("xmlOps.xs");
var wizard_constants = xdc.loadCapsule("wizard_constants.xs");
var fileModule = xdc.module('xdc.services.io.File');
var logic = xdc.loadCapsule("logic.xs");
var shared_gui = xdc.loadCapsule("shared_gui.xs");
xmlOps.setLogic(logic);
var os = String(java.lang.System.getProperty('os.name'));
var isWin = (os.match(/^Windows/i) != null);
var SHELL_WIDTH = isWin ? 575 : 600;
var SHELL_HEIGHT = isWin ? 440 : 510;
var XDCVERS_MSG = 'The version of xdctools is older than 3.16. Either restart '
+'GenAlg Wizard with xdctools 3.16 or higher, or modify the generated '
+'makefile to point at a newer version of xdctools.';

function initSWT() {
    swt = xdc.jre.org.eclipse.swt;
    browser = xdc.jre.org.eclipse.swt.browser;
    custom = xdc.jre.org.eclipse.swt.custom;
    graphics = xdc.jre.org.eclipse.swt.graphics;
    layout = xdc.jre.org.eclipse.swt.layout;
    widgets = xdc.jre.org.eclipse.swt.widgets;
    events = xdc.jre.org.eclipse.swt.events;
    dnd = xdc.jre.org.eclipse.swt.dnd;
    program = xdc.jre.org.eclipse.swt.program;
}

function setAlgEnv(ae)
{
    algEnv = ae;
}

function createPartControl(parent)
{
    main(algEnv, parent);
}

var buttons = {
    finishEnabled:    false,
    updated:          false
}

var pageFxns = [page0Fxn];
var numberOfPages = pageFxns.length;

var hoverTip = null;
var hoverLabel = null;

var filterExt = ['*.genalg', '*.*'];
var filterNames = ['Genalg Files (*.genalg)', 'All Files (*.*)'];

var targetList = wizard_constants.targetList;

function init(algEnv, parent)
{
    if (parent) {
        display = parent.getDisplay();
    }
    else {
        display = new widgets.Display();
    }
    algEnv.display = display;
    image = new graphics.Image(display, csd + "/img/tray.JPG");
    ICO_ERROR = graphics.Image(display, csd + "/img/error_tsk.gif");
    ICO_WARNING = graphics.Image(display, csd + "/img/warning.gif");

    var shellLocation = graphics.Point(66,87);

    algEnv.shellLocation = shellLocation;
/*
    var shell = new widgets.Shell(display, swt.SWT.DIALOG_TRIM |
                                  swt.SWT.APPLICATION_MODAL);

    shell.setImage(image);
    shell.setSize(200, 100);
    shell.setText("Loading...");

    if (algEnv.shellLocation!=null) {
        shell.setLocation(algEnv.shellLocation);
    }

    var grid = new layout.GridLayout(1, false);
    shell.setLayout(grid);

    var comp = new widgets.Composite(shell, swt.SWT.NONE);
    comp.setLayout(new layout.GridLayout());

    var text = new widgets.Text(comp, swt.SWT.WRAP | swt.SWT.READ_ONLY);
    text.setText(wizard_constants.INIT_MESSAGE);
    text.setLayoutData(new layout.GridData(180, 50));

    var pb = new widgets.ProgressBar(comp, swt.SWT.HORIZONTAL);
    pb.setMinimum(0);
    pb.setMaximum(100);
    pb.setBounds(10, 10, 200, 20);
    shell.pack();
    shell.open();

    pb.setSelection(30);

    pb.setSelection(50);
    pb.setSelection(80);
    pb.setSelection(100);
    shell.dispose();
*/
}


function page0Fxn(comp)
{
    /* required user input:
       optional user input: */

    var invalidTarget = false;
    var targMsg = '';
    var shell = comp.getShell();

    var errObj = createImageLabel(comp, 2, 2);

    var col1Comp = widgets.Composite(comp, swt.SWT.BORDER);
    var col1Layout = layout.GridLayout();
    col1Layout.numColumns = 2;
    col1Comp.setLayout(col1Layout);
    var col1Data = layout.GridData(layout.GridData.VERTICAL_ALIGN_BEGINNING|layout.GridData.FILL_HORIZONTAL);
    col1Data.minimumWidth = SHELL_WIDTH / 3;
    col1Data.widthHint = SHELL_WIDTH / 3;
    col1Comp.setLayoutData(col1Data);

    var lbl = widgets.Label(col1Comp, swt.SWT.NONE);
    lbl.setText('Module:');
    lbl.setToolTipText(wizard_constants.GENALG_MODULE_HOVER);
    var textModuleName = widgets.Text(col1Comp, swt.SWT.BORDER);
    textModuleName.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));
    textModuleName.addListener(swt.SWT.Modify, function(e) {
        textInterfaceName.setText('I' + textModuleName.getText());
        if (!expertCheck.getSelection()) {
            textPackageName.setText(textVendorName.getText() + '.' +
                                    textModuleName.getText());
        }
        setState();
    });
    textModuleName.addListener(swt.SWT.Verify, function(e) {
        e.text = String(e.text).toUpperCase();
    });
    addSelectAllListener(textModuleName);

    var lbl = widgets.Label(col1Comp, swt.SWT.NONE);
    lbl.setText('Vendor:');
    lbl.setToolTipText(wizard_constants.GENALG_VENDOR_HOVER);
    var textVendorName = widgets.Text(col1Comp, swt.SWT.BORDER);
    textVendorName.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));
    textVendorName.addListener(swt.SWT.Modify, function(e) {
        if (!expertCheck.getSelection()) {
            textPackageName.setText(textVendorName.getText() + '.' +
                                    textModuleName.getText());
        }
        setState();
    });
    textVendorName.addListener(swt.SWT.Verify, function(e) {
        e.text = String(e.text).toUpperCase();
    });
    addSelectAllListener(textVendorName);

    var lbl = widgets.Label(col1Comp, swt.SWT.NONE);
    lbl.setText('Interface:');
    lbl.setToolTipText(wizard_constants.GENALG_INTERFACE_HOVER);
    var textInterfaceName = widgets.Text(col1Comp, swt.SWT.BORDER);
    textInterfaceName.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));
    textInterfaceName.addListener(swt.SWT.Modify, function(e) {
        setState();
    });
    textInterfaceName.addListener(swt.SWT.Verify, function(e) {
        e.text = String(e.text).toUpperCase();
    });
    addSelectAllListener(textInterfaceName);

    var col2Comp = widgets.Composite(comp, swt.SWT.NONE);
    var col2Layout = layout.GridLayout();
    col2Layout.numColumns = 2;
    col2Comp.setLayout(col2Layout);
    var col2Data = layout.GridData(layout.GridData.VERTICAL_ALIGN_BEGINNING|layout.GridData.FILL_HORIZONTAL);
    col2Data.minimumWidth = SHELL_WIDTH / 2;
    col2Data.widthHint = SHELL_WIDTH / 2;
    col2Comp.setLayoutData(col2Data);

    // Templates Drop-down ComboBox
    var lbl = new widgets.Label(col2Comp, swt.SWT.NONE);
    lbl.setText("Base Interface: ");
    var templateComboBox = new widgets.Combo(col2Comp, swt.SWT.DROP_DOWN |
        swt.SWT.READ_ONLY);
    templateComboBox.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));

    var lbl = widgets.Label(col2Comp, swt.SWT.NONE);
    lbl.setText('Package Name:');
    lbl.setToolTipText(wizard_constants.GENALG_XDM_ALG_PKG_HOVER);
    var textPackageName = widgets.Text(col2Comp, swt.SWT.BORDER);
    textPackageName.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));
    textPackageName.addListener(swt.SWT.Modify, function(e) {
        setState();
    });
    textPackageName.addListener(swt.SWT.Verify, function(e) {
        e.text = String(e.text).toLowerCase();
    });
    addSelectAllListener(textPackageName);

    // Target Drop-down ComboBox
    var lbl = new widgets.Label(col2Comp, swt.SWT.NONE );
    lbl.setText("Target: ");
    lbl.setToolTipText(wizard_constants.GENALG_TARGET);
    var targetComboBox = new widgets.Combo(col2Comp, swt.SWT.DROP_DOWN);
    for (var i = 0; i < targetList.length; i++) {
        targetComboBox.add(targetList[i]);
    }
    targetComboBox.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));
    targetComboBox.addListener(swt.SWT.Modify, function(e) {
        try {
            invalidTarget = false;
            targMsg = '';
            var targ = String(targetComboBox.getText());
            var tmod = xdc.useModule(targ);
            var base = String(tmod.$super.base);
            if (base.indexOf('xdc.bld.ITarget') == -1) {
                invalidTarget = true;
                targMsg = 'Target must inherit from xdc.bld.ITarget';
            }
            else {
                algEnv.targname = tmod.name;
                algEnv.targsuffix = tmod.suffix;
            }
        }
        catch (e) {
            invalidTarget = true;
            targMsg = targ + ':  Invalid target';
        }
        setState();
    });

    var row2Comp = widgets.Composite(comp, swt.SWT.NONE);
    var row2Layout = layout.GridLayout();
    row2Layout.numColumns = 2;
    row2Comp.setLayout(row2Layout);
    var row2Data = new layout.GridData(layout.GridData.FILL_HORIZONTAL);
    row2Data.horizontalSpan = 2;
    row2Comp.setLayoutData(row2Data);

    // Browse for output directory
    var output = new BrowseButton(row2Comp, shell, 150);
    output.setMessage("Set Destination Directory");
    output.btn.setToolTipText(wizard_constants.OUTPUT_HOVER);
    output.addListener("", "Please enter a valid package directory", false);
    output.btn.addListener(swt.SWT.Selection, function(additionalEvent) {
    });
    output.textBox.addListener(swt.SWT.Modify, function(e) {
        setState();
    });

    // Browse for cgtools dir
    var cgTools = new BrowseButton(row2Comp, shell, 150);
    cgTools.setMessage("Set C6000 TI 'cgtools' Dir");
    cgTools.btn.setToolTipText(wizard_constants.CGTOOLS_DESC);
    cgTools.textBox.setToolTipText(wizard_constants.CGTOOLS_DESC);
    cgTools.addListener("", "Please enter a valid TI CodeGen tools directory", false);
    cgTools.btn.addListener(swt.SWT.Selection, function(additionalEvent) {
    });
    cgTools.textBox.addListener(swt.SWT.Modify, function(e) {
        setState();
    });

    var expertCheck = widgets.Button(comp, swt.SWT.CHECK);
    expertCheck.setText('Expert Mode');
    var expCheckData = new layout.GridData();
    expCheckData.horizontalIndent = 8;
    expCheckData.verticalIndent = 8;
    expertCheck.setLayoutData(expCheckData);
    expertCheck.setToolTipText(wizard_constants.EXPERT_HOVER);
    expertCheck.addListener(swt.SWT.Selection, function(e) {
        algEnv.expertCheck = e.widget.getSelection() ? true : false;
        if (!algEnv.expertCheck) {
            textPackageName.setText(textVendorName.getText() + '.' +
                                    textModuleName.getText());
        }
        setState();
    });

    var CCSProjCheck = widgets.Button(comp, swt.SWT.CHECK);
    CCSProjCheck.setText('Generate CCSv4 project');
    var projCheckData = new layout.GridData(layout.GridData.HORIZONTAL_ALIGN_END);
    CCSProjCheck.setLayoutData(projCheckData);

    var rulesCheck = widgets.Button(comp, swt.SWT.CHECK);
    rulesCheck.setText("Don't check for build dependencies, I'll modify the generated makefile (e.g. Rules.make users)");
    var rulesCheckData = new layout.GridData();
    rulesCheckData.verticalIndent = 8;
    rulesCheckData.horizontalIndent = 8;
    rulesCheckData.horizontalSpan = 2;
    rulesCheck.setLayoutData(rulesCheckData);
    rulesCheck.addListener(swt.SWT.Selection, function(e) {
        algEnv.rulesCheck = e.widget.getSelection() ? true : false;
        setState();
    });

    if (!algEnv.xdcVersOK) {
        var msgComp = widgets.Composite(comp, swt.SWT.NONE);
        var msgLayout = layout.GridLayout();
        msgLayout.numColumns = 2;
        msgComp.setLayout(msgLayout);
        var compData = layout.GridData(layout.GridData.FILL_HORIZONTAL);
        compData.horizontalSpan = 2;
        msgComp.setLayoutData(compData);

        var imgLabel = widgets.Label(msgComp, swt.SWT.NONE);
        imgData = layout.GridData();
        imgData.widthHint = 15;
        imgLabel.setLayoutData(imgData);
        imgLabel.setImage(ICO_WARNING);

        var textBox = new widgets.Text(msgComp, swt.SWT.WRAP|swt.SWT.MULTI|
                                                swt.SWT.READ_ONLY);
        var gridHint = new layout.GridData(layout.GridData.FILL_HORIZONTAL);
        textBox.setLayoutData(gridHint);
        textBox.setText(XDCVERS_MSG);
    }


    // Create menu bar with listeners
    var menuBar = createMenu(shell, 0);
    menuBar.getItem(0).getMenu().getItem(0).addListener(swt.SWT.Selection,
    function (openListener) {
        algEnv = readXmlFile(shell, image, algEnv);
        populatePanel();
    });
    menuBar.getItem(0).getMenu().addListener(swt.SWT.Show, function(e) {
        menuBar.getItem(0).getMenu().getItem(1).setEnabled(!hasError());
    } );
    menuBar.getItem(0).getMenu().getItem(1).addListener(swt.SWT.Selection,
    function(saveListener) {
        collectInput();

        var complete = saveXmlFile(shell, algEnv);
        if (complete == true) {
            shared_gui.createMessageBox( "Save successful", algEnv);
        }
    } );

    menuBar.getItem(0).getMenu().getItem(3).addListener(swt.SWT.Selection,
    function() {
        var dialog = new widgets.DirectoryDialog(shell);
        dialog.setFilterPath(algEnv.templatesDir);
        var dir = dialog.open();
        if (dir != null) {
            algEnv.templatesDir = String(dir).replace(/\\/g,"/");
            updateTemplatesCombo();
            setState();
        }
    });

    function updateTemplatesCombo() {
        var tFile = java.io.File(algEnv.templatesDir);
        templateComboBox.removeAll();
        // save/try to restore previous
        for each (var f in tFile.listFiles()) {
            if (f.isDirectory() &&

                java.io.File(f.getCanonicalPath() + '/mkpkg.xdt').exists()) {
                templateComboBox.add(f.getName());
            }
        }
        if (templateComboBox.getItemCount()) {
            var text = null;
            if (algEnv.templateName &&
                templateComboBox.indexOf(algEnv.templateName) != -1) {

               text = algEnv.templateName;
            }
            if (!text) {
                text = templateComboBox.getItem(0)
            }
            templateComboBox.setText(text);
        }
    }

    function populatePanel() {
        if (algEnv.packageName != null) {
            textPackageName.setText(algEnv.packageName);
        }
        if (algEnv.moduleName != null) {
            textModuleName.setText(algEnv.moduleName);
        }
        if (algEnv.vendorName != null) {
            textVendorName.setText(algEnv.vendorName);
        }
        if (algEnv.interfaceName != null) {
            textInterfaceName.setText(algEnv.interfaceName);
        }
        if (algEnv.templateName != null) {
            templateComboBox.setText(algEnv.templateName);
        }
        if (algEnv.output != null && algEnv.output != "null") {
            output.textBox.setText(algEnv.output);
        }
        if (algEnv.target != null) {
            targetComboBox.setText(algEnv.target);
        }
        if (algEnv.cgTools != null) {
            cgTools.textBox.setText(algEnv.cgTools);
        }
        CCSProjCheck.setSelection((algEnv.genCCSProj == true) ? true : false);
        rulesCheck.setSelection((algEnv.rulesCheck == true) ? true : false);
        expertCheck.setSelection((algEnv.expertCheck == true) ? true : false);

        setState();
    }

    updateTemplatesCombo();
    populatePanel();

    function hasError() {
        return (errObj.label1.getImage() != null);
    }

    function setState() {
        textInterfaceName.setEnabled(algEnv.expertCheck ? true : false);
        textPackageName.setEnabled(algEnv.expertCheck ? true : false);
        var valid = validateInput();
        buttons.finishEnabled = valid;
        buttons.updated = true;
    }

    function validateInput() {
        var warnMsg = null;
        var msg;
        var valid = true;
        var validOutput = true;
        var validCgTools = true;
        if (String(textPackageName.getText()).length == 0 ||
            String(textModuleName.getText()).length == 0  ||
            String(textVendorName.getText()).length == 0  ||
            String(textInterfaceName.getText()).length == 0 ||
            String(output.textBox.getText()).length == 0  ||
            String(cgTools.textBox.getText()).length == 0) {

            valid = false;
            msg = 'One or more empty fields';
        }
        else if (String(templateComboBox.getText()).length == 0) {
            valid = false;
            msg = 'No templates';
        }
        if (valid) {
            if (invalidTarget) {
                valid = false;
                msg = targMsg;
            }
        }
        if (valid) {
            var outdir = String(output.textBox.getText());
            if (!outdir.length || !java.io.File(outdir).isDirectory()) {
                valid = validOutput = false;
            }
            if (valid) {
                /* check for overwrite */
                outdir = outdir + '/' +
                         String(textPackageName.getText()).replace(/\./g,'/');
                outdir = outdir.replace(/\\/g, '/');
                var outf = java.io.File(outdir);
                if (outf.exists() && outf.list().length > 0) {
                    warnMsg = 'Destination Directory is not empty--files may be overwritten';
                }
            }

            if (!rulesCheck.getSelection()) {
                var clFile = 'cl6x';
                if (isWin) {
                    clFile += '.exe';
                }
                if (!java.io.File(cgTools.textBox.getText() + '/bin/' +
                                  clFile).exists()) {
                    valid = validCgTools = false;
                }
            }
            if (!valid) {
                msg = "Please enter valid ";
                if (!validOutput) {
                    msg += 'Destination Directory';
                    if (!validCgTools) {
                        msg += ' and ';
                    }
                }
                if (!validCgTools) {
                    msg += "C6000 TI 'cgTools' Dir";
                }
            }
        }
        if (!valid) {
            setImageLabel(errObj, ICO_ERROR, msg);
        }
        else if (warnMsg) {
            setImageLabel(errObj, ICO_WARNING, warnMsg);
        }
        else {
            setImageLabel(errObj, null, '');
        }
        return (valid);
    }

    function collectInput() {
        algEnv.packageName = String(textPackageName.getText());
        algEnv.moduleName = String(textModuleName.getText());
        algEnv.vendorName = String(textVendorName.getText());
        algEnv.interfaceName = String(textInterfaceName.getText());
        algEnv.templateName = String(templateComboBox.getText());
        algEnv.output = String(output.textBox.getText());
        algEnv.target = String(targetComboBox.getText());
        algEnv.genCCSProj = CCSProjCheck.getSelection() ? true : false;
        algEnv.cgTools = String(cgTools.textBox.getText());
    }

    return {
        title:              "   Basic Algorithm Information",
        composite:          comp,
        collectInputFxn:    collectInput
    };
}

function finishOK(algEnv)
{
    var OK = true;
    return (OK);
}

function posNumberText(textBox, img, zeroOk)
{
    var valid = true;
    if (!isPosNumber(textBox.getText(), zeroOk)) {
        img.setImage(ICO_ERROR);
        valid = false;
    }
    else {
        img.setImage(null);
    }
    return (valid);
}



function createGroup(comp, text)
{
    var group = widgets.Group(comp, swt.SWT.SHADOW_NONE);
    group.setText(text);
    var layoutGroup = layout.GridLayout();
    layoutGroup.numColumns = 2;
    group.setLayout(layoutGroup);
    group.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));
    return (group);
}

/**
 * Creates the menu bar at the top of each panel
 * @param {Shell}
 * @param {String} Message to display when user clicks on Instructions
 * @return menuBar
 */
function createMenu(shell, pageNum)
{
    var menuBar = new widgets.Menu(shell, swt.SWT.BAR);
    shell.setMenuBar(menuBar);

    var fileItem = new widgets.MenuItem(menuBar, swt.SWT.CASCADE);
    fileItem.setText ("&File");

    var fileSubmenu = new widgets.Menu(shell, swt.SWT.DROP_DOWN);
    fileItem.setMenu(fileSubmenu);

    if (pageNum == 0) {
        var instructionsText = wizard_constants.INSTRUCTIONS0;
        var open = new widgets.MenuItem(fileSubmenu, swt.SWT.PUSH);
        open.setText("&Open");
        open.setAccelerator(swt.SWT.CTRL + 111); //111 is dec of o
        var save = new widgets.MenuItem(fileSubmenu, swt.SWT.PUSH);
        save.setText("&Save");
        save.setAccelerator(swt.SWT.CTRL + 115); //115 is dec of s
        new widgets.MenuItem(fileSubmenu, swt.SWT.SEPARATOR);
        var templatesDir = new widgets.MenuItem(fileSubmenu, swt.SWT.PUSH);
        templatesDir.setText("Set &Templates Directory...");
        templatesDir.setAccelerator(swt.SWT.CTRL + 116); //116 is dec of t

    }

    var helpItem = new widgets.MenuItem(menuBar, swt.SWT.CASCADE);
    helpItem.setText ("&Help");
    helpItem.setAccelerator(swt.SWT.CTRL + 104); //104 is dec of h

    var helpSubmenu = new widgets.Menu(shell, swt.SWT.DROP_DOWN);
    helpItem.setMenu(helpSubmenu);

    var instruction = new widgets.MenuItem(helpSubmenu, swt.SWT.PUSH);
    instruction.setText("Instructions");
    instruction.addListener(swt.SWT.Selection, function (instructionListener) {
        var dlg = new widgets.Shell(shell, swt.SWT.DIALOG_TRIM |
                                           swt.SWT.RESIZE |
                                           swt.SWT.APPLICATION_MODAL);
        if (algEnv.shellLocation!=null) {
            dlg.setLocation(algEnv.shellLocation);
        }
        dlg.setImage(image);
        dlg.setSize(410, 380);
        dlg.setText("Wizard Instructions");

        var grid = new layout.GridLayout(1, false);
        dlg.setLayout(grid);

        var comp = new widgets.Composite(dlg, swt.SWT.NONE);
        comp.setLayout(new layout.GridLayout());
        var gridData = new layout.GridData(layout.GridData.FILL_BOTH);
        comp.setLayoutData(gridData);

        var text = new widgets.Link(comp, swt.SWT.NONE);
        text.addListener(swt.SWT.Selection,
        function(e) {
            var pro = program.Program.launch(e.text);
        });
        text.setText(instructionsText);
        text.setLayoutData(new layout.GridData(380, 290));

        var okBtn = new widgets.Button(comp, swt.SWT.PUSH);
        okBtn.setText("    OK    ");

        okBtn.addListener(swt.SWT.Selection, function (okEvent) {
            dlg.dispose();
        });

        dlg.open();
    });

    var OSinstruction = new widgets.MenuItem(helpSubmenu, swt.SWT.PUSH);
    OSinstruction.setText("Open/Save Instructions");
    OSinstruction.addListener(swt.SWT.Selection,
        function (OSinstructionListener) {
        var dlg = new widgets.Shell(shell, swt.SWT.DIALOG_TRIM |
                                           swt.SWT.RESIZE |
                                           swt.SWT.APPLICATION_MODAL);
        if (algEnv.shellLocation!=null) {
            dlg.setLocation(algEnv.shellLocation);
        }
        dlg.setImage(image);
        dlg.setSize(410, 280);
        dlg.setText("Opening & Saving Wizard Files");

        var grid = new layout.GridLayout(1, false);
        dlg.setLayout(grid);

        var comp = new widgets.Composite(dlg, swt.SWT.NONE);
        comp.setLayout(new layout.GridLayout());

        var text = new widgets.Text(comp, swt.SWT.WRAP | swt.SWT.READ_ONLY |
                                          swt.SWT.MULTI);
        text.setText(wizard_constants.SAVE_OPEN_INSTRUCTIONS);
        text.setLayoutData(new layout.GridData(350, 180));

        var okBtn = new widgets.Button(comp, swt.SWT.PUSH);
        okBtn.setText("    OK    ");

        okBtn.addListener(swt.SWT.Selection, function (okEvent) {
            dlg.dispose();
        });

        dlg.open();
    });

    var templatesDirHlp = new widgets.MenuItem(helpSubmenu, swt.SWT.PUSH);
    templatesDirHlp.setText("Templates Directory");
    templatesDirHlp.addListener(swt.SWT.Selection,
        function (templatesDirHlpListener) {
        var dlg = new widgets.Shell(shell, swt.SWT.DIALOG_TRIM |
                                           swt.SWT.RESIZE |
                                           swt.SWT.APPLICATION_MODAL);
        if (algEnv.shellLocation!=null) {
            dlg.setLocation(algEnv.shellLocation);
        }
        dlg.setImage(image);
        dlg.setSize(410, 280);
        dlg.setText("The Templates Directory");

        var grid = new layout.GridLayout(1, false);
        dlg.setLayout(grid);

        var comp = new widgets.Composite(dlg, swt.SWT.NONE);
        comp.setLayout(new layout.GridLayout());

        var text = new widgets.Link(comp, swt.SWT.NONE);
        text.addListener(swt.SWT.Selection,
        function(e) {
            var pro = program.Program.launch(e.text);
        });
        text.setText(wizard_constants.TEMPLATES_DIR_HELP);
        text.setLayoutData(new layout.GridData(380, 170));

        var okBtn = new widgets.Button(comp, swt.SWT.PUSH);
        okBtn.setText("    OK    ");

        okBtn.addListener(swt.SWT.Selection, function (okEvent) {
            dlg.dispose();
        });

        dlg.open();
    });

    var about = new widgets.MenuItem(helpSubmenu, swt.SWT.PUSH);
    about.setText("About");
    about.addListener(swt.SWT.Selection, function (aboutListener) {
        var msg = wizard_constants.ABOUT_MESSAGE + '\n\n' + csd;
        shared_gui.createMessageBox(msg, algEnv);
    });

    return menuBar;
}

/**
 * Generates a button that when selected creates a file dialog
 * @param {Composite}
 * @param {Shell}
 */
function BrowseButton(comp, shell, width) {

    this.comp = comp;
    this.shell = shell;

    var message = "";
    var errorMessage = "";
    var stringMatch = "";

    this.setMessage = function(message) {
        this.message = message;
        btn.setText(message);
    }

    var btn = new widgets.Button(comp, swt.SWT.PUSH);
    var data = new layout.GridData();
    data.widthHint = width;
    btn.setLayoutData(data);
    this.btn = btn;

    this.addListener = function(stringMatch, errorMessage, fileBool) {
        if (fileBool==true) {
            btn.addListener(swt.SWT.Selection, function(event) {
                var dialog = new widgets.FileDialog(shell);
                dialog.setFilterPath(textBox.getText());

                var file = dialog.open();
                if (file != null) {
                    file = String(file).replace(/\\/g,"/");
                    var suffix = file + "";
                    suffix = suffix.substring(
                        (suffix.lastIndexOf("/")+1), suffix.length);

                    if (suffix.match(stringMatch)!=null) {
                        textBox.setText(file);
                    }
                    else {
                        textBox.setText("");
                        shared_gui.createMessageBox(errorMessage, algEnv);
                    }

                }
            });
        }
        else {
            btn.addListener(swt.SWT.Selection, function(event) {
                var dialog = new widgets.DirectoryDialog(shell);
                dialog.setFilterPath(textBox.getText());

                var dir = dialog.open();
                if (dir != null) {
                    dir = String(dir).replace(/\\/g,"/");
                    var suffix = dir;
                    suffix = suffix.substring(
                        (suffix.lastIndexOf("/")+1), suffix.length);

                    if (suffix.match(stringMatch)!=null) {
                        textBox.setText(dir);
                    }
                    else {
                        textBox.setText("");
                        shared_gui.createMessageBox(errorMessage, algEnv);
                    }
                }
            });
        }
    }

    var textBox = new widgets.Text(comp, swt.SWT.SINGLE |
                                         swt.SWT.BORDER);
    textBox.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));
    addSelectAllListener(textBox);
    this.textBox = textBox;
}

/**
* GUI to save current algEnv to user_specified_loc.xml
* @param {Shell}
* @param {algEnv}
* @return Boolean: true if save successful, false otherwise
*/
function saveXmlFile(shell, algEnv) {

    /* create text for XML file from algEnv */
    var xmlText = xmlOps.saveXml(algEnv);

    /* get output location */
    var dialog = new widgets.FileDialog(shell, swt.SWT.SAVE);
    dialog.setFilterExtensions(filterExt);
    dialog.setFilterNames(filterNames);
    dialog.setFilterPath(algEnv.output);
    dialog.setFileName(algEnv.packageName.replace(/\./g,'_')
        + '_wizard.genalg');
    var file = dialog.open();
    if (file != null) {
        try {
            if (java.io.File(file).exists()) {
                var btnClicked = shared_gui.createYesNoBox(
                        file + " already exists! Do you want to overwrite it?",
                        algEnv);
                if (btnClicked == 1) {
                    var status = xmlOps.textFileWriter(file, xmlText);
                    if (status==-1) { return false; }
                    return true;
                }
            } else {
                var status = xmlOps.textFileWriter(file, xmlText);
                if (status==-1) { return false; }
                return true;
            }
        }
        catch(e) {
            shared_gui.createMessageBox("Error occured writing XML file:\n" + e,
                algEnv);
        }
    }
    return false;
}

/**
* GUI needed to read XML file
* @param image to be displayed on shell
* @param algEnv to be modified based on XML file
* @param shellLocation
* @return updated algEnv based on XML file
*/
function readXmlFile(shell, image, algEnv) {

    var xshell = new widgets.Shell(shell, swt.SWT.DIALOG_TRIM |
                                          swt.SWT.RESIZE);
    xshell.setSize(100,100);
    if (algEnv.shellLocation!=null) {
        xshell.setLocation(algEnv.shellLocation);
    }
    xshell.setImage(image);
    var dialog = new widgets.FileDialog(shell);
    dialog.setFilterExtensions(filterExt);
    dialog.setFilterNames(filterNames);
    var file = dialog.open();
    if (file != null) {
        try {
            algEnv = xmlOps.loadXml(file, algEnv);
        }
        catch(e) {
            var mbox = widgets.MessageBox(shell, swt.SWT.ICON_INFORMATION);
            mbox.setText("GenAlg Wizard Message");
            mbox.setMessage("Error occured reading XML file: " + e);
            mbox.open();
        }
    }
    xshell.close();
    return algEnv;
}

function controlMouseOver(shell, control, e)
{
    var item =  control.getItem(graphics.Point(e.x, e.y));
    if (item != null  && item.getData() != null) {
        var infoStr = String(item.getData());
        if (algEnv.reposMap[infoStr]) {
            infoStr += "\n in " + algEnv.reposMap[infoStr];
        }
        itemHover(shell, control, item, infoStr, graphics.Point(e.x, e.y));
    }
}

function itemHover(shell, control, item, text, curPt)
{
    var pt;

    if (hoverTip != null  && !hoverTip.isDisposed()) {
        hoverTip.dispose();
    }
    hoverTip = widgets.Shell(shell, swt.SWT.ON_TOP|swt.SWT.NO_FOCUS|swt.SWT.TOOL);
    hoverTip.setBackground (display.getSystemColor (swt.SWT.COLOR_INFO_BACKGROUND));
    var lo = layout.FillLayout();
    lo.marginWidth = 2;
    hoverTip.setLayout(lo);
    hoverLabel = widgets.Label(hoverTip, swt.SWT.NONE);
    hoverLabel.setForeground(display.getSystemColor(swt.SWT.COLOR_INFO_FOREGROUND));
    hoverLabel.setBackground(display.getSystemColor(swt.SWT.COLOR_INFO_BACKGROUND));
    hoverLabel.setText(text);
    hoverLabel.addListener(swt.SWT.MouseExit, function(e) { mouseLeave(e); });
    hoverLabel.addListener(swt.SWT.MouseDown, function(e) { mouseLeave(e); });
    var size = hoverTip.computeSize(swt.SWT.DEFAULT, swt.SWT.DEFAULT);
    var rect = item.getBounds(0);
    if (curPt == undefined) {
        pt = control.toDisplay(rect.x, rect.y);
    }
    else {
        pt = control.toDisplay(curPt.x, curPt.y);
    }
    var monRect = display.getPrimaryMonitor().getClientArea();
    var left = (pt.x + size.x <= monRect.width) ? pt.x :
               pt.x - size.x;

    var top = (pt.y + rect.height + 4 + size.y <= monRect.height) ?
              pt.y + rect.height + 4 : pt.y - size.y - 4;
    if (top < 0) {
        top = 0;
    }
    hoverTip.setBounds(left, top, size.x, size.y);
    hoverTip.setVisible(true);
}

function mouseLeave(e)
{
    if (hoverTip != null) {
        hoverTip.dispose();
        hoverTip = null;
        hoverLabel = null;
    }
}

function navigatePages(parent)
{
    var currentPage = 0;

    while (true) {

        var shell = new widgets.Shell(display, swt.SWT.DIALOG_TRIM |
            swt.SWT.RESIZE | swt.SWT.MIN);
        shell.setImage(image);
        shell.setLayout(new layout.FormLayout());
        shell.setText("XDM GenAlg Wizard");
        shell.setMinimumSize(SHELL_WIDTH, SHELL_HEIGHT);
        shell.addListener(swt.SWT.Move, function(event) {
            try {
                algEnv.shellLocation = shell.getLocation();
            }
            catch(e) {}
        });
        algEnv.shell = shell;

        var compMain = new widgets.Composite(shell, swt.SWT.NONE);
        var layoutMain = new layout.GridLayout();
        layoutMain.numColumns = 2;
        layoutMain.horizontalSpacing = 15;
        compMain.setLayout(layoutMain);

        var labelTitle = new widgets.Label(compMain, swt.SWT.WRAP);
        var fillLabelTitle = new layout.GridData(layout.GridData.FILL_HORIZONTAL);
        fillLabelTitle.horizontalSpan = 2;
        labelTitle.setLayoutData(fillLabelTitle);

        var space = new widgets.Label(compMain, swt.SWT.SEPARATOR |
            swt.SWT.HORIZONTAL);
        fillLabelTitle = new layout.GridData(layout.GridData.FILL_HORIZONTAL);
        fillLabelTitle.horizontalSpan = 2;
        space.setLayoutData(fillLabelTitle);

        var pageData = pageFxns[ currentPage ](compMain);
        var point = pageData.composite.computeSize(swt.SWT.DEFAULT,
            swt.SWT.DEFAULT, false);
        labelTitle.setText(pageData.title);

        var formMain = new layout.FormData();
        formMain.top = new layout.FormAttachment(0, 10);
        formMain.left = new layout.FormAttachment(0, 10);
        formMain.right = new layout.FormAttachment(100, -10);
        compMain.setLayoutData(formMain);

        var compLine = new widgets.Composite(shell, swt.SWT.NONE);
        var layoutLine = new layout.RowLayout();
        layoutLine.pack = false;
        compLine.setLayout(layoutLine);
        var lbl = new widgets.Label(compLine, swt.SWT.SEPARATOR |
            swt.SWT.HORIZONTAL);
        lbl.setLayoutData(new layout.RowData(1000, swt.SWT.DEFAULT));

        var comp = new widgets.Composite(shell, swt.SWT.NONE);
        var lay = new layout.RowLayout();
        lay.pack = false;
        comp.setLayout(lay);

        var btnFinish = new widgets.Button(comp, swt.SWT.PUSH);
        btnFinish.setText("   &Finish   ");

        var btnCancel = new widgets.Button(comp, swt.SWT.PUSH);
        btnCancel.setText("   &Cancel   ");

        var formRow = new layout.FormData();
        formRow.left = new layout.FormAttachment(0, 200);
        formRow.bottom = new layout.FormAttachment(100, -10);
        formRow.right = new layout.FormAttachment(100, -10);
        comp.setLayoutData(formRow);

        var formLine = new layout.FormData();
        formLine.left = new layout.FormAttachment(0, 10);
        formLine.right = new layout.FormAttachment(100, -10);
        formLine.bottom = new layout.FormAttachment(comp, -5, swt.SWT.TOP);
        compLine.setLayoutData(formLine);

        btnFinish.enabled = buttons.finishEnabled;

        var eventFinish = false;
        var eventCancel = false;

        btnCancel.addListener(swt.SWT.Selection, function(e) {
            eventCancel = true;
        });

        btnFinish.addListener(swt.SWT.Selection, function(e) {
            eventFinish = true;
        });

        var displayRect = display.getBounds();
        if (algEnv.shellLocation!=null) {
            shell.setLocation(algEnv.shellLocation);
        }

        shell.setSize(SHELL_WIDTH, SHELL_HEIGHT);
        shell.open();
        buttons.updated = false;

        shell.addListener(swt.SWT.Close, function(e) {
              pageData.collectInputFxn();
              var btnClicked = shared_gui.createYesNoBox(
                  "Do you want to quit?", algEnv);
              if (btnClicked == 1) {
                  print("Closing XDM GenAlg Wizard now...");
                  shell.dispose();
                  eventCancel = true;
              }
        });
        while(!shell.isDisposed()) {
            if (!display.readAndDispatch()) {
                java.lang.Thread.sleep(50);
                if (eventFinish) {
                    pageData.collectInputFxn();
/*
                    if (!finishOK(algEnv)) {
                        eventFinish = false;
                        continue;
                    }
*/
                    var btnClicked = shared_gui.createYesNoBox(
                        "Would you like to save the values entered into "
                        + "the XDM GenAlg Wizard?", algEnv);
                    if (btnClicked == 1) {
                        var complete = saveXmlFile(shell, algEnv);
                        if (complete == true) {
                            shared_gui.createMessageBox("Save successful",
                                                        algEnv);
                        }
                    }
                    break;
                }
                if (eventCancel) {
                    pageData.collectInputFxn();
                    var btnClicked = shared_gui.createYesNoBox(
                        "Do you want to quit?", algEnv);
                    if (btnClicked == 1) {
                        print("Closing GUI XDM GenAlg Wizard now...");
                    }
                    else {
                        eventCancel = false;
                    }
                    break;
                }
                if (buttons.updated) {
                    buttons.updated   = false;
                    btnFinish.enabled = buttons.finishEnabled;
                }
            }
            try {
                shellLocation = shell.getLocation();
            }
            catch(e) {}
        }
        if (!shell.isDisposed()) {
            shell.dispose();
        }
        if (eventFinish || eventCancel) {
            break;
        }
    }
    if (!parent) {
        display.dispose();
    }
    return (eventFinish == true);
}

function main(algEnv, parent) {
    print("\nStarting GUI XDM GenAlg Wizard now...");
    print('GUI XDM GenAlg Wizard user:\n'
          + 'You can ignore any warnings below about Mozilla this or that missing.');

    initSWT();

    init(algEnv, parent);


    this.algEnv = algEnv;
    var success = navigatePages(parent);


    if (!success) {
        algEnv.error = true;
    }
    return (algEnv);
}


function isPosNumber(text, zeroOK)
{
    var ret = false;

    text = String(text);
    if (text.length > 0) {
        if (!text.match(/\D/g)) {
            if ((zeroOK && Number(text) >= 0) ||
                Number(text) > 0) {

                ret = true;
            }
        }
    }
    return (ret);
}


/**
 * Creates a banner of instructions at the top of each panel
 * @param {Shell}
 * @param {String} Message to display at top of panel
 */
function InstructionBanner(comp) {
    var text = "";

    var textBox = new widgets.Text(comp, swt.SWT.WRAP
        |swt.SWT.MULTI | swt.SWT.BORDER|swt.SWT.READ_ONLY);
    var gridHint = new layout.GridData(layout.GridData.FILL_HORIZONTAL);
    gridHint.horizontalSpan = 2;
    textBox.setLayoutData(gridHint);

    this.setText = function(text) {
        this.text = text;
        textBox.setText(text);
    }
}




function CustomBrowseButton(comp, shell, width, indent)
{
    this.comp = comp;
    this.shell = shell;

    var message = "";

    this.setMessage = function(message) {
        this.message = message;
        btn.setText(message);
    }

    var btn = new widgets.Button(comp, swt.SWT.PUSH);
    var data = new layout.GridData();
    data.widthHint = width;
    if (indent) {
        data.horizontalIndent = 10;
    }
    btn.setLayoutData(data);
    this.btn = btn;

    this.addListener = function(fileList) {
        btn.addListener(swt.SWT.Selection, function(event) {
            var dialog = new widgets.DirectoryDialog(shell);
            dialog.setFilterPath(textBox.getText());

            while (true) {
                var dir = dialog.open();
                if (dir != null) {
                    var notFoundList = [];
                    var dirList = java.io.File(dir).list();
                    for each (var src in fileList) {
                        var found = false;
                        for each (var dest in dirList) {
                            if (String(dest) == src) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            notFoundList.push(src);
                        }
                    }
                    if (notFoundList.length == 0) {
                        textBox.setText(dir);
                    }
                    else {
                        textBox.setText('');
                        var msg = 'Not found: ';
                        for each (var nf in notFoundList) {
                            msg += ' ' + nf;
                        }
                        shared_gui.createMessageBox(msg, algEnv);
                    }
                }
                else {
                    break;
                }
            }
        });
    }

    var textBox = new widgets.Text(comp, swt.SWT.SINGLE |
                                         swt.SWT.BORDER|swt.SWT.READ_ONLY);
    textBox.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));
    this.textBox = textBox;
}


function createImageLabel(comp, cols, span)
{
    var lblComp = widgets.Composite(comp, swt.SWT.NONE);
    var lblLayout = layout.GridLayout();
    lblLayout.numColumns = cols;
    lblComp.setLayout(lblLayout);
    var compData = layout.GridData(layout.GridData.FILL_HORIZONTAL);
    compData.horizontalSpan = span;
    lblComp.setLayoutData(compData);

    var label1 = widgets.Label(lblComp, swt.SWT.NONE);
    label1Data = layout.GridData();
    label1Data.widthHint = 15;
    label1.setLayoutData(label1Data);
    label1.setImage(null);

    var label2 = widgets.Label(lblComp, swt.SWT.NONE);
    label2.setLayoutData(layout.GridData(layout.GridData.FILL_HORIZONTAL));
    return {
        label1: label1,
        label2: label2
    };
}

function setImageLabel(msgObj, img, msg)
{
    msgObj.label1.setImage(img);
    msgObj.label2.setText(msg);
}

function selectAllListener(e)
{
    if (e.widget.getEditable()) {
        e.widget.setSelection(0, String(e.widget.getText()).length);
    }
}

function addSelectAllListener(text)
{
    text.addListener(swt.SWT.FocusIn, selectAllListener);
    text.addListener(swt.SWT.MouseDown, selectAllListener);
}

/*
 *  @(#) ti.xdais.wizards.genalg; 1, 0, 0, 0,31; 5-4-2010 08:46:33; /db/wtree/library/trees/dais/dais-q11x/src/
 */

