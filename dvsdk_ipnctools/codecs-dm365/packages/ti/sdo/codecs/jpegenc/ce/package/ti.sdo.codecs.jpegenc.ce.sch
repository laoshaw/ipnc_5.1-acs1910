xdc.loadCapsule('xdc/om2.xs');

var $om = xdc.om;
var __CFG__ = $om.$name == 'cfg';
var __ROV__ = $om.$name == 'rov';
var $$pkgspec = xdc.$$ses.findPkg('ti.sdo.codecs.jpegenc.ce');

/* ======== IMPORTS ======== */

    xdc.loadPackage('ti.sdo.ce.image1');
    xdc.loadPackage('ti.sdo.codecs.jpegenc');
    xdc.loadPackage('xdc');
    xdc.loadPackage('xdc.corevers');

/* ======== OBJECTS ======== */

// package ti.sdo.codecs.jpegenc.ce
    var pkg = $om.$$bind('ti.sdo.codecs.jpegenc.ce.Package', $$PObj());
    $om.$$bind('ti.sdo.codecs.jpegenc.ce', $$VObj('ti.sdo.codecs.jpegenc.ce', pkg));
// module JPEGENC
    var po = $om.$$bind('ti.sdo.codecs.jpegenc.ce.JPEGENC.Module', $$PObj());
    var vo = $om.$$bind('ti.sdo.codecs.jpegenc.ce.JPEGENC', $$VObj('ti.sdo.codecs.jpegenc.ce.JPEGENC', po));
    pkg.$$bind('JPEGENC', vo);

/* ======== CONSTS ======== */

// module JPEGENC

/* ======== CREATES ======== */

// module JPEGENC

/* ======== FUNCTIONS ======== */


/* ======== SIZES ======== */


/* ======== TYPES ======== */

// module JPEGENC
    var cap = $om.$$bind('ti.sdo.codecs.jpegenc.ce.JPEGENC$$capsule', xdc.loadCapsule('ti/sdo/codecs/jpegenc/ce/JPEGENC.xs'));
    var po = $om['ti.sdo.codecs.jpegenc.ce.JPEGENC.Module'].$$init('ti.sdo.codecs.jpegenc.ce.JPEGENC.Module', $om['ti.sdo.ce.image1.IIMGENC1.Module']);
        po.$$fld('$hostonly', $$T_Num, 1, 'r');
        po.$$fld('alg', $om['ti.sdo.codecs.jpegenc.JPEGENC.Module'], xdc.om['ti.sdo.codecs.jpegenc.JPEGENC'], 'rh');
        po.$$fld('ialgFxns', $$T_Str, "JPEGENC_TI_IJPEGENC", 'rh');
        po.$$fld('iresFxns', $$T_Str, "JPEGENC_TI_IRES", 'rh');
        var fxn = cap['module$meta$init'];
        if (fxn) $om.$$bind('ti.sdo.codecs.jpegenc.ce.JPEGENC$$module$meta$init', true);
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

// package ti.sdo.codecs.jpegenc.ce
    var po = $om['ti.sdo.codecs.jpegenc.ce.Package'].$$init('ti.sdo.codecs.jpegenc.ce.Package', $om['xdc.IPackage.Module']);
    po.$$bind('$capsule', undefined);
    var pkg = $om['ti.sdo.codecs.jpegenc.ce'].$$init(po, 'ti.sdo.codecs.jpegenc.ce', $$DEFAULT, false);
    pkg.$$bind('$name', 'ti.sdo.codecs.jpegenc.ce');
    pkg.$$bind('$category', 'Package');
    pkg.$$bind('$$qn', 'ti.sdo.codecs.jpegenc.ce.');
    pkg.$$bind('$vers', [1, 0, 0]);
    pkg.$attr.$seal('length');
    pkg.$$bind('$imports', [
        ['ti.sdo.ce.image1', []],
        ['ti.sdo.codecs.jpegenc', []],
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
    
// module JPEGENC
    var vo = $om['ti.sdo.codecs.jpegenc.ce.JPEGENC'];
    var po = $om['ti.sdo.codecs.jpegenc.ce.JPEGENC.Module'];
    vo.$$init(po, 'ti.sdo.codecs.jpegenc.ce.JPEGENC', $$DEFAULT, false);
    vo.$$bind('Module', po);
    vo.$$bind('$category', 'Module');
    vo.$$bind('$capsule', $om['ti.sdo.codecs.jpegenc.ce.JPEGENC$$capsule']);
    vo.$$bind('$package', $om['ti.sdo.codecs.jpegenc.ce']);
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
    vo.$$bind('$$meta_iobj', 0 + ('ti.sdo.codecs.jpegenc.ce.JPEGENC$$instance$static$init' in $om));
    vo.$$bind('$$fxntab', []);
    vo.$$bind('$$logEvtCfgs', []);
    vo.$$bind('$$errorDescCfgs', []);
    vo.$$bind('$$assertDescCfgs', []);
    vo.$attr.$seal('length');
    pkg.$$bind('JPEGENC', vo);
    pkg.$unitNames.$add('JPEGENC');

/* ======== INITIALIZATION ======== */

if (__CFG__) {
} // __CFG__
    $om['ti.sdo.codecs.jpegenc.ce.JPEGENC'].module$meta$init();
    pkg.init();
    $om['ti.sdo.codecs.jpegenc.ce.JPEGENC'].$$bless();
    $om.$packages.$add(pkg);
