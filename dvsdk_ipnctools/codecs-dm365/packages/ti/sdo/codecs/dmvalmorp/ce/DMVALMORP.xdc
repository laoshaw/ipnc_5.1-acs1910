/*!
 *  ======== DMVALMORP========
 *  DMVALMORP codec specification
 *
 *  This file specifies information necessary to integrate with the Codec
 *  Engine.
 *
 *  By inheriting ti.sdo.ce.video1.IVIDENC1, DMVALMORP declares that it "is
 *  a" video1 algorithm.  This allows the codec engine to automatically
 *  supply simple stubs and skeletons for transparent execution of DSP
 *  codecs by the GPP.
 *
 *  In addition to declaring the type of the DMVALMORP algorithm, we
 *  declare the external symbol required by xDAIS that identifies the
 *  algorithms implementation functions.
 */
metaonly module DMVALMORP inherits ti.sdo.ce.video1.IVIDENC1
{
    readonly config ti.sdo.codecs.dmvalmorp.DMVALMORP.Module alg =
        ti.sdo.codecs.dmvalmorp.DMVALMORP;
    
    override readonly config String ialgFxns = "DMVALMORP_TI_IDMVALMORP";

    override readonly config String iresFxns = "DMVALMORP_TI_IRES";

}
