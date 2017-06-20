/*!
 *  ======== DMVALRLE========
 *  DMVALRLE codec specification
 *
 *  This file specifies information necessary to integrate with the Codec
 *  Engine.
 *
 *  By inheriting ti.sdo.ce.video1.IVIDENC1, DMVALRLE declares that it "is
 *  a" video1 algorithm.  This allows the codec engine to automatically
 *  supply simple stubs and skeletons for transparent execution of DSP
 *  codecs by the GPP.
 *
 *  In addition to declaring the type of the DMVALRLE algorithm, we
 *  declare the external symbol required by xDAIS that identifies the
 *  algorithms implementation functions.
 */
metaonly module DMVALRLE inherits ti.sdo.ce.video1.IVIDENC1
{
    readonly config ti.sdo.codecs.dmvalrle.DMVALRLE.Module alg =
        ti.sdo.codecs.dmvalrle.DMVALRLE;
    
    override readonly config String ialgFxns = "DMVALRLE_TI_IDMVALRLE";

    override readonly config String iresFxns = "DMVALRLE_TI_IRES";

}
