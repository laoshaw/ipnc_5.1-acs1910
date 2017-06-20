/*!
 *  ======== IPRUN========
 *  IPRUN codec specification
 *
 *  This file specifies information necessary to integrate with the Codec
 *  Engine.
 *
 *  By inheriting ti.sdo.ce.video1.IVIDENC1, IPRUN declares that it "is
 *  a" video1 algorithm.  This allows the codec engine to automatically
 *  supply simple stubs and skeletons for transparent execution of DSP
 *  codecs by the GPP.
 *
 *  In addition to declaring the type of the IPRUN algorithm, we
 *  declare the external symbol required by xDAIS that identifies the
 *  algorithms implementation functions.
 */
metaonly module IPRUN inherits ti.sdo.ce.video1.IVIDENC1
{
    readonly config ti.sdo.codecs.iprun.IPRUN.Module alg =
        ti.sdo.codecs.iprun.IPRUN;
    
    override readonly config String ialgFxns = "IPRUN_TI_IIPRUN";

    override readonly config String iresFxns = "IPRUN_TI_IRES";

}
