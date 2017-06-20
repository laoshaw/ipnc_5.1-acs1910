/*!
 *  ======== DMVALISF========
 *  DMVALISF codec specification
 *
 *  This file specifies information necessary to integrate with the Codec
 *  Engine.
 *
 *  By inheriting ti.sdo.ce.video1.IVIDENC1, DMVALISF declares that it "is
 *  a" video1 algorithm.  This allows the codec engine to automatically
 *  supply simple stubs and skeletons for transparent execution of DSP
 *  codecs by the GPP.
 *
 *  In addition to declaring the type of the DMVALISF algorithm, we
 *  declare the external symbol required by xDAIS that identifies the
 *  algorithms implementation functions.
 */
metaonly module DMVALISF inherits ti.sdo.ce.video1.IVIDENC1
{
    readonly config ti.sdo.codecs.dmvalisf.DMVALISF.Module alg =
        ti.sdo.codecs.dmvalisf.DMVALISF;
    
    override readonly config String ialgFxns = "DMVALISF_TI_IDMVALISF";

    override readonly config String iresFxns = "DMVALISF_TI_IRES";

}
