/*
 *  Copyright (c) 2010 by Texas Instruments and others.
 *  All rights reserved. This program and the accompanying materials
 *  are made available under the terms of the Eclipse Public License v1.0
 *  which accompanies this distribution, and is available at
 *  http://www.eclipse.org/legal/epl-v10.html
 *
 *  Contributors:
 *      Texas Instruments - initial implementation
 *
 * */

/*
 *  ======== Platform.xdc ========
 *
 */

/*!
 *  ======== Platform ========
 *  Platform support for the sdp2430
 *
 *  This module implements xdc.platform.IPlatform and defines configuration
 *  parameters that correspond to this platform's Cpu's, Board's, etc.
 *
 *  The configuration parameters are initialized in this package's
 *  configuration script (package.cfg) and "bound" to the TCOM object
 *  model.  Once they are part of the model, these parameters are
 *  queried by a program's configuration script.
 *
 *  This particular platform has a single Cpu, and therefore, only
 *  declares a single CPU configuration object.  Multi-CPU platforms
 *  would declare multiple Cpu configuration parameters (one per
 *  platform CPU).
 */
metaonly module Platform inherits xdc.platform.IPlatform
{
    readonly config xdc.platform.IPlatform.Board BOARD = {      
        id:             "0",
        boardName:      "sdp2430",
        boardFamily:    "sdp2430",
        boardRevision:  null,
    };
        
    readonly config xdc.platform.IExeContext.Cpu DSP = {        
        id:             "0",
        clockRate:      330,
        catalogName:    "ti.catalog.c6000",
        deviceName:     "TMS320C2430",
        revision:       "",
    };
    
    readonly config xdc.platform.IExeContext.Cpu GPP = {        
        id:             "1",
        clockRate:      330,
        catalogName:    "ti.catalog.arm",
        deviceName:     "OMAP2430",
        revision:       "1.0",
    };

instance:

    override readonly config xdc.platform.IPlatform.Memory
        externalMemoryMap[string] = [
            ["DDR", {name: "DDR", base: 0x80000000, len: 0x8000000}],
        ];

    override config string dataMemory = "DDR";

    override config string stackMemory = "DDR";
};
/*
 *  @(#) ti.platforms.sdp2430; 1, 0, 0,262; 2-23-2010 16:52:15; /db/ztree/library/trees/platform/platform-k32x/src/
 */

