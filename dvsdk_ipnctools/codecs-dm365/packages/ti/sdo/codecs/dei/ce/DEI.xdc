/*!
 *  ======== DEI========
 *  DEI codec specification
 *
 *  This file specifies information necessary to integrate with the Codec
 *  Engine.
 *
 *  By inheriting ti.sdo.ce.video1.IVIDENC1, DEI declares that it "is
 *  a" video1 algorithm.  This allows the codec engine to automatically
 *  supply simple stubs and skeletons for transparent execution of DSP
 *  codecs by the GPP.
 *
 *  In addition to declaring the type of the DEI algorithm, we
 *  declare the external symbol required by xDAIS that identifies the
 *  algorithms implementation functions.
 */
metaonly module DEI inherits ti.sdo.ce.video1.IVIDENC1
{
    readonly config ti.sdo.codecs.dei.DEI.Module alg =
        ti.sdo.codecs.dei.DEI;
    
    override readonly config String ialgFxns = "DEI_TI_IDEI";

    override readonly config String iresFxns = "DEI_TI_IRES";

}
