#
# Copyright (c) 2011, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# *  Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# *  Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# *  Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#
# insmod_rmmod.sh
#
# Tests that edmak isn't leaking memory / pages when being repeatedly
# inserted and removed from the kernel. Execute ./insmod_rmmod.sh 1000 for
# instance to do it 1000 times.
#
# ============================================================================
# Copyright (c) Texas Instruments Inc 2006
#
# Use of this software is controlled by the terms and conditions found in the
# license agreement under which this software has been supplied or provided.
# ============================================================================
#

COUNTER=0

while test $COUNTER -lt $1
do
        insmod edmak.ko
        rmmod edmak
        echo insmod and rmmod number $COUNTER
        echo
        let COUNTER=COUNTER+1
done
#
#  @(#) ti.sdo.linuxutils.edma; 1, 0, 0,95; 7-12-2011 11:02:21; /db/atree/library/trees/linuxutils/linuxutils-j05x/src/ xlibrary

#

