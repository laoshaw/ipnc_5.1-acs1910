/*!
 *  ======== AES========
 *  AES codec specification
 *
 *  This file specifies information necessary to integrate with the Codec
 *  Engine.
 *
 *  By inheriting ti.sdo.ce.video1.IVIDENC1, AES declares that it "is
 *  a" video1 algorithm.  This allows the codec engine to automatically
 *  supply simple stubs and skeletons for transparent execution of DSP
 *  codecs by the GPP.
 *
 *  In addition to declaring the type of the AES algorithm, we
 *  declare the external symbol required by xDAIS that identifies the
 *  algorithms implementation functions.
 */
metaonly module AES inherits ti.sdo.ce.video1.IVIDENC1
{
    readonly config ti.sdo.codecs.aes.AES.Module alg =
        ti.sdo.codecs.aes.AES;
    
    override readonly config String ialgFxns = "AES_TI_IAES";

    override readonly config String iresFxns = "AES_TI_IRES";

}
