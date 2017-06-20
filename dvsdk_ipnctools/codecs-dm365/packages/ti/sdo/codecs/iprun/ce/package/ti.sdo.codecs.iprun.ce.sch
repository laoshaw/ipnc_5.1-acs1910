xdc.loadCapsule('xdc/om2.xs');

var $om = xdc.om;
var __CFG__ = $om.$name == 'cfg';
var __ROV__ = $om.$name == 'rov';
var $$pkgspec = xdc.$$ses.findPkg('ti.sdo.codecs.iprun.ce');

/* ======== IMPORTS ======== */

    xdc.loadPackage('ti.sdo.ce.video1');
    xdc.loadPackage('ti.sdo.codecs.iprun');
    xdc.loadPackage('xdc');
    xdc.loadPackage('xdc.corevers');

/* ======== OBJECTS ======== */

// package ti.sdo.codecs.iprun.ce
    var pkg = $om.$$bind('ti.sdo.codecs.iprun.ce.Package', $$PObj());
    $om.$$bind('ti.sdo.codecs.iprun.ce', $$VObj('ti.sdo.codecs.iprun.ce', pkg));
// module IPRUN
    var po = $om.$$bind('ti.sdo.codecs.iprun.ce.IPRUN.Module', $$PObj());
    var vo = $om.$$bind('ti.sdo.codecs.iprun.ce.IPRUN', $$VObj('ti.sdo.codecs.iprun.ce.IPRUN', po));
    pkg.$$bind('IPRUN', vo);

/* ======== CONSTS ======== */

// module IPRUN

/* ======== CREATES ======== */

// module IPRUN

/* ======== FUNCTIONS ======== */


/* ======== SIZES ======== */


/* ======== TYPES ======== */

// module IPRUN
    var cap = $om.$$bind('ti.sdo.codecs.iprun.ce.IPRUN$$capsule', xdc.loadCapsule('ti/sdo/codecs/iprun/ce/IPRUN.xs'));
    var po = $om['ti.sdo.codecs.iprun.ce.IPRUN.Module'].$$init('ti.sdo.codecs.iprun.ce.IPRUN.Module', $om['ti.sdo.ce.video1.IVIDENC1.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('alg', $om['ti.sdo.codecs.iprun.IPRUN.Module'], $om['ti.sdo.codecs.iprun.IPRUN'], 'rh');
        po.$$fld('ialgFxns', $$T_Str, "IPRUN_TI_IIPRUN", 'rh');
        po.$$fld('iresFxns', $$T_Str, "IPRUN_TI_IRES", 'rh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.sdo.codecs.iprun.ce.IPRUN$$module$meta$init', true);
        if (fxn) po.$$fxn('module$meta$init', $$T_Met, fxn);
        if (cap['getCreationStackSize']) po.$$fxn('getCreationStackSize', $om['ti.sdo.ce.ICodec$$getCreationStackSize'], cap['getCreationStackSize']);
        if (cap['getDaramScratchSize']) po.$$fxn('getDaramScratchSize', $om['ti.sdo.ce.ICodec$$getDaramScratchSize'], cap['getDaramScratchSize']);
        if (cap['getSaramScratchSize']) po.$$fxn('getSaramScratchSize', $om['ti.sdo.ce.ICodec$$getSaramScratchSize'], cap['getSaramScratchSize']);
        if (cap['getStackSize']) po.$$fxn('getStackSize', $om['ti.sdo.ce.ICodec$$getStackSize'], cap['getStackSize']);
        if (cap['getUUID']) po.$$fxn('getUUID', $om['ti.sdo.ce.ICodec$$getUUID'], cap['getUUID']);

/* ======== ROV ======== */

if (__ROV__) {


} // __ROV__

/* ======== SINGLETONS ======== */

// package ti.sdo.codecs.iprun.ce
    var po = $om['ti.sdo.codecs.iprun.ce.Package'].$$init('ti.sdo.codecs.iprun.ce.Package', $om['xdc.IPackage.Module']);
    po.$$bind('$capsule', undefined);
    var pkg = $om['ti.sdo.codecs.iprun.ce'].$$init(po, 'ti.sdo.codecs.iprun.ce', $$DEFAULT, false);
    $om.$packages.$add(pkg);
    pkg.$$bind('$name', 'ti.sdo.codecs.iprun.ce');
    pkg.$$bind('$category', 'Package');
    pkg.$$bind('$$qn', 'ti.sdo.codecs.iprun.ce.');
    pkg.$$bind('$spec', $$pkgspec);
    pkg.$$bind('$vers', [1, 0, 0]);
    pkg.$attr.$seal('length');
    pkg.$$bind('$imports', [
        ['ti.sdo.ce.video1', []],
        ['ti.sdo.codecs.iprun', []],
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
    
// module IPRUN
    var vo = $om['ti.sdo.codecs.iprun.ce.IPRUN'];
    var po = $om['ti.sdo.codecs.iprun.ce.IPRUN.Module'];
    vo.$$init(po, 'ti.sdo.codecs.iprun.ce.IPRUN', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$spec', xdc.$$ses.findUnit('ti.sdo.codecs.iprun.ce.IPRUN'));
    vo.$$bind('$capsule', $om['ti.sdo.codecs.iprun.ce.IPRUN$$capsule']);
    vo.$$bind('$package', $om['ti.sdo.codecs.iprun.ce']);
    vo.$$bind('$$tdefs', []);
    vo.$$bind('$$proxies', []);
    vo.$$bind('$$mcfgs', []);
    pkg.$modules.$add(vo);
    $om.$modules.$add(vo);
    vo.$$bind('$$instflag', 0);
    vo.$$bind('$$iobjflag', 1);
    vo.$$bind('$$sizeflag', 1);
    vo.$$bind('$$dlgflag', 0);
    vo.$$bind('$$iflag', 1);
    vo.$$bind('$$romcfgs', '|');
    if ('Module_State' in vo) vo.$$bind('$object', new vo.Module_State);
    vo.$$bind('$$meta_iobj', 0 + ('ti.sdo.codecs.iprun.ce.IPRUN$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr.$seal('length');
    pkg.$$bind('IPRUN', vo);
    pkg.$unitNames.$add('IPRUN');

/* ======== INITIALIZATION ======== */

if (__CFG__) {
} // __CFG__
    $om['ti.sdo.codecs.iprun.ce.IPRUN'].module$meta$init();
    pkg.init();
    $om['ti.sdo.codecs.iprun.ce.IPRUN'].$$bless();
