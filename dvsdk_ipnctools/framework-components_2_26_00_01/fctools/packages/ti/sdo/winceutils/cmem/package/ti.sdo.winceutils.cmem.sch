xdc.loadCapsule('xdc/om2.xs');

var $om = xdc.om;
var __CFG__ = $om.$name == 'cfg';
var __ROV__ = $om.$name == 'rov';
var $$pkgspec = xdc.$$ses.findPkg('ti.sdo.winceutils.cmem');

/* ======== IMPORTS ======== */

    xdc.loadPackage('xdc');
    xdc.loadPackage('xdc.corevers');

/* ======== OBJECTS ======== */

// package ti.sdo.winceutils.cmem
    var pkg = $om.$$bind('ti.sdo.winceutils.cmem.Package', $$PObj());
    $om.$$bind('ti.sdo.winceutils.cmem', $$VObj('ti.sdo.winceutils.cmem', pkg));
// module CMEM
    var po = $om.$$bind('ti.sdo.winceutils.cmem.CMEM.Module', $$PObj());
    var vo = $om.$$bind('ti.sdo.winceutils.cmem.CMEM', $$VObj('ti.sdo.winceutils.cmem.CMEM', po));
    pkg.$$bind('CMEM', vo);

/* ======== CONSTS ======== */

// module CMEM

/* ======== CREATES ======== */

// module CMEM

/* ======== FUNCTIONS ======== */


/* ======== SIZES ======== */


/* ======== TYPES ======== */

// module CMEM
    var po = $om['ti.sdo.winceutils.cmem.CMEM.Module'].$$init('ti.sdo.winceutils.cmem.CMEM.Module', $$Module);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');

/* ======== ROV ======== */

if (__ROV__) {


} // __ROV__

/* ======== SINGLETONS ======== */

// package ti.sdo.winceutils.cmem
    var po = $om['ti.sdo.winceutils.cmem.Package'].$$init('ti.sdo.winceutils.cmem.Package', $om['xdc.IPackage.Module']);
    var cap = $om.$$bind('xdc.IPackage$$capsule', xdc.loadCapsule('ti/sdo/winceutils/cmem/package.xs'));
        if (cap['init']) po.$$fxn('init', $om['xdc.IPackage$$init'], cap['init']);
        if (cap['close']) po.$$fxn('close', $om['xdc.IPackage$$close'], cap['close']);
        if (cap['validate']) po.$$fxn('validate', $om['xdc.IPackage$$validate'], cap['validate']);
        if (cap['exit']) po.$$fxn('exit', $om['xdc.IPackage$$exit'], cap['exit']);
        if (cap['getLibs']) po.$$fxn('getLibs', $om['xdc.IPackage$$getLibs'], cap['getLibs']);
        if (cap['getSects']) po.$$fxn('getSects', $om['xdc.IPackage$$getSects'], cap['getSects']);
    po.$$bind('$capsule', cap);
    var pkg = $om['ti.sdo.winceutils.cmem'].$$init(po, 'ti.sdo.winceutils.cmem', $$DEFAULT, false);
    pkg.$$bind('$name', 'ti.sdo.winceutils.cmem');
    pkg.$$bind('$category', 'Package');
    pkg.$$bind('$$qn', 'ti.sdo.winceutils.cmem.');
    pkg.$$bind('$spec', $$pkgspec);
    pkg.$$bind('$vers', [1, 0, 0]);
    pkg.$attr.$seal('length');
    pkg.$$bind('$imports', [
    ]);
    if (pkg.$vers.length >= 3) {
        pkg.$vers.push(Packages.xdc.services.global.Vers.getDate(xdc.csd() + '/..'));
    }
    
    pkg.build.libraries = [
    ];
    
    pkg.build.libDesc = [
    ];
    if ('suffix' in xdc.om['xdc.IPackage$$LibDesc']) {
    }
    
// module CMEM
    var vo = $om['ti.sdo.winceutils.cmem.CMEM'];
    var po = $om['ti.sdo.winceutils.cmem.CMEM.Module'];
    vo.$$init(po, 'ti.sdo.winceutils.cmem.CMEM', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.sdo.winceutils.cmem.CMEM'));
    vo.$$bind('$capsule', undefined);
    vo.$$bind('$package', $om['ti.sdo.winceutils.cmem']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 0);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.sdo.winceutils.cmem.CMEM$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr.$seal('length');
    pkg.$$bind('CMEM', vo);
    pkg.$unitNames.$add('CMEM');

/* ======== INITIALIZATION ======== */

if (__CFG__) {
} // __CFG__
    $om['ti.sdo.winceutils.cmem.CMEM'].module$meta$init();
    pkg.init();
    $om['ti.sdo.winceutils.cmem.CMEM'].$$bless();
    $om.$packages.$add(pkg);
