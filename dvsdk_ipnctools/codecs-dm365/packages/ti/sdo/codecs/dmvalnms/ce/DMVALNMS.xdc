/*!
 *  ======== DMVALNMS========
 *  DMVALNMS codec specification
 *
 *  This file specifies information necessary to integrate with the Codec
 *  Engine.
 *
 *  By inheriting ti.sdo.ce.video1.IVIDENC1, DMVALNMS declares that it "is
 *  a" video1 algorithm.  This allows the codec engine to automatically
 *  supply simple stubs and skeletons for transparent execution of DSP
 *  codecs by the GPP.
 *
 *  In addition to declaring the type of the DMVALNMS algorithm, we
 *  declare the external symbol required by xDAIS that identifies the
 *  algorithms implementation functions.
 */
metaonly module DMVALNMS inherits ti.sdo.ce.video1.IVIDENC1
{
    readonly config ti.sdo.codecs.dmvalnms.DMVALNMS.Module alg =
        ti.sdo.codecs.dmvalnms.DMVALNMS;
    
    override readonly config String ialgFxns = "DMVALNMS_TI_IDMVALNMS";

    override readonly config String iresFxns = "DMVALNMS_TI_IRES";

}