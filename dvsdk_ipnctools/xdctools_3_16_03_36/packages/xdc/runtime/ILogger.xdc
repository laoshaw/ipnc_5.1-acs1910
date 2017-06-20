/* --COPYRIGHT--,ESD
 *  Copyright (c) 2008 Texas Instruments. All rights reserved. 
 *  This program and the accompanying materials are made available under the 
 *  terms of the Eclipse Public License v1.0 and Eclipse Distribution License
 *  v. 1.0 which accompanies this distribution. The Eclipse Public License is
 *  available at http://www.eclipse.org/legal/epl-v10.html and the Eclipse
 *  Distribution License is available at 
 *  http://www.eclipse.org/org/documents/edl-v10.php.
 *
 *  Contributors:
 *      Texas Instruments - initial implementation
 * --/COPYRIGHT--*/
/*!
 *  ======== ILogger ========
 *  Interface to service Log events
 *
 *  A logger is responsible for recording, transmitting, or otherwise
 *  handling `{@link Log#EventDesc Log Events}` generated by clients of the
 *  `{@link Log}` module. The `Log` module uses modules that implement the
 *  `ILogger` interface to record the log events. Most application code will
 *  use the `Log` module instead of directly calling the specific `ILogger`
 *  module implementation.
 *
 *  All logger implementations must inherit from this interface. The
 *  derived implementations must implement the functions defined in this
 *  interface but may also add additional configuration parameters and
 *  functions.
 */
interface ILogger {

    /*!
     *  ======== getMetaArgs ========
     *  Returns any meta data needed to support RTA.
     *
     *  This meta data should be returned in the form of a structure which
     *  can be converted into XML. This data is added to the RTA XML file
     *  during the application's configuration, and can be accessed later
     *  through the xdc.rta.MetaData module.
     *
     *  The MetaData is returned per instance of the ILogger module. The
     *  instance object is passed to the function as the first argument.
     *
     *  The second argument is the index of the instance in the list of 
     *  the ILogger's static instances.
     */
    function getMetaArgs(inst, instNum);
     
instance:

    /*! 
     *  ======== write4 ========
     *  Process a log event with up to 4 arguments.
     *
     *      At the time this method is called, `evt` encodes two values: the
     *      module ID of the module that "triggered" a `{@link Log#Event}`
     *      and the `{@link Log#EventId}` of the event.  The module ID can
     *      be obtained via `{@link Types#getModuleId}(evt)`
     *      and the event ID can be obtained via
     *      `{@link Types#getEventId}(evt)`.
     *
     *      The event ID can be used to compare against other known
     *      `Log.Event`s.
     *      @p(code)
     *          if (Log_getEventId(MY_EVENT) == Types_getEventId(evt)) {
     *              :
     *          }
     *      @p
     *
     *      The event ID value of `0` is used to indicate an event triggered
     *      by a call to one of the `{@link Log#print0 Log_print[0-6]}`
     *      methods. These methods take a 
     *      format string rather than a `Log_Event` argument and, as a result,
     *      the event ID encoded in `evt` is `0` and the parameter `a1` is
     *      the format string.
     *
     *      Non-zero event IDs can also be used to access the `msg` string
     *      associated with the `{@link Log#EventDesc}` that originally
     *      defined the `Log` event.
     *      @p(code)
     *          Types_EventId id = Types_getEventId(evt));
     *          if (id != 0) {
     *              String msg = Text_ropeText(id);
     *              System_aprintf(msg, a1, a2, a3, a4);
     *          }
     *      @p
     *      This works because an event's ID is simply an offset into a table
     *      of characters (maintained by the `{@link Text}` module)
     *      containing the event's msg string.
     *
     *  @param(evt) event to be logged
     *
     *  @param(a1)  arbitrary argument passed by caller
     *
     *      This parameter, along with `a2`, `a3`, and `a4` are parameters
     *      that are to be interpreted according to the message format string
     *      associated with `evt`.
     *
     *  @see Log#Event
     *  @see Log#EventDesc
     *  @see Text#ropeText
     *  @see Types#getEventId
     */
    Void write4(Types.Event evt, IArg a1, IArg a2, IArg a3, IArg a4);

    /*! 
     *  ======== write8 ========
     *  Process a log event with up to 8 arguments.
     *
     *  Same as `write4` except with 8 arguments rather than 4.
     *
     *  @see ILogger#write4()
     */
    Void write8(Types.Event evt, IArg a1, IArg a2, IArg a3, IArg a4,
                IArg a5, IArg a6, IArg a7, IArg a8);
}
/*
 *  @(#) xdc.runtime; 2, 0, 0, 0,237; 2-22-2010 10:33:58; /db/ztree/library/trees/xdc/xdc-u17x/src/packages/
 */

