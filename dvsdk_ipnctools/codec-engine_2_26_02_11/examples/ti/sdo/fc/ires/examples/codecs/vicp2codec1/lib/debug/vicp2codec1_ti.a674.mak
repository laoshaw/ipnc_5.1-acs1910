#
#  Do not edit this file.  This file is generated from 
#  package.bld.  Any modifications to this file will be 
#  overwritten whenever makefiles are re-generated.
#
#  target compatibility key = ti.targets.C674{1,0,6.1,5
#
ifneq (clean,$(MAKECMDGOALS))
-include package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.o674.dep
endif

package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.o674: | .interfaces
package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.o674: package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.c lib/debug/vicp2codec1_ti.a674.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) cl674 $< ...
	$(ti.targets.C674.rootDir)/bin/cl6x -c  -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti/package -fr=./package/lib/lib/debug/vicp2codec1_ti/package -fc $<
	$(MKDEP) -a $@.dep -p package/lib/lib/debug/vicp2codec1_ti/package -s o674 $< -C   -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti/package -fr=./package/lib/lib/debug/vicp2codec1_ti/package
	-@$(FIXDEP) $@.dep $@.dep
	
package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.o674:C_DIR=
package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.o674: PATH:=$(ti.targets.C674.rootDir)/bin/:$(PATH)

package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.s674: | .interfaces
package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.s674: package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.c lib/debug/vicp2codec1_ti.a674.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) cl674 -n $< ...
	$(ti.targets.C674.rootDir)/bin/cl6x -c -n -s --symdebug:none -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti/package -fr=./package/lib/lib/debug/vicp2codec1_ti/package -fc $<
	$(MKDEP) -a $@.dep -p package/lib/lib/debug/vicp2codec1_ti/package -s o674 $< -C  -n -s --symdebug:none -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti/package -fr=./package/lib/lib/debug/vicp2codec1_ti/package
	-@$(FIXDEP) $@.dep $@.dep
	
package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.s674:C_DIR=
package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.s674: PATH:=$(ti.targets.C674.rootDir)/bin/:$(PATH)

ifneq (clean,$(MAKECMDGOALS))
-include package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.o674.dep
endif

package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.o674: | .interfaces
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.o674: vicp2codec1_ti.c lib/debug/vicp2codec1_ti.a674.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) cl674 $< ...
	$(ti.targets.C674.rootDir)/bin/cl6x -c  -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti -fr=./package/lib/lib/debug/vicp2codec1_ti -fc $<
	$(MKDEP) -a $@.dep -p package/lib/lib/debug/vicp2codec1_ti -s o674 $< -C   -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti -fr=./package/lib/lib/debug/vicp2codec1_ti
	-@$(FIXDEP) $@.dep $@.dep
	
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.o674:C_DIR=
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.o674: PATH:=$(ti.targets.C674.rootDir)/bin/:$(PATH)

package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.s674: | .interfaces
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.s674: vicp2codec1_ti.c lib/debug/vicp2codec1_ti.a674.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) cl674 -n $< ...
	$(ti.targets.C674.rootDir)/bin/cl6x -c -n -s --symdebug:none -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti -fr=./package/lib/lib/debug/vicp2codec1_ti -fc $<
	$(MKDEP) -a $@.dep -p package/lib/lib/debug/vicp2codec1_ti -s o674 $< -C  -n -s --symdebug:none -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti -fr=./package/lib/lib/debug/vicp2codec1_ti
	-@$(FIXDEP) $@.dep $@.dep
	
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.s674:C_DIR=
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.s674: PATH:=$(ti.targets.C674.rootDir)/bin/:$(PATH)

ifneq (clean,$(MAKECMDGOALS))
-include package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.o674.dep
endif

package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.o674: | .interfaces
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.o674: vicp2codec1_ti_ires.c lib/debug/vicp2codec1_ti.a674.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) cl674 $< ...
	$(ti.targets.C674.rootDir)/bin/cl6x -c  -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti -fr=./package/lib/lib/debug/vicp2codec1_ti -fc $<
	$(MKDEP) -a $@.dep -p package/lib/lib/debug/vicp2codec1_ti -s o674 $< -C   -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti -fr=./package/lib/lib/debug/vicp2codec1_ti
	-@$(FIXDEP) $@.dep $@.dep
	
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.o674:C_DIR=
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.o674: PATH:=$(ti.targets.C674.rootDir)/bin/:$(PATH)

package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.s674: | .interfaces
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.s674: vicp2codec1_ti_ires.c lib/debug/vicp2codec1_ti.a674.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) cl674 -n $< ...
	$(ti.targets.C674.rootDir)/bin/cl6x -c -n -s --symdebug:none -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti -fr=./package/lib/lib/debug/vicp2codec1_ti -fc $<
	$(MKDEP) -a $@.dep -p package/lib/lib/debug/vicp2codec1_ti -s o674 $< -C  -n -s --symdebug:none -qq -pdsw225 -mv6740 -eo.o674 -ea.s674 -mi10 -mo  -pdr -pden -pds=452 -pds=195  -D_DEBUG_=1 -DDBC_ASSERTS=1 -DGT_ASSERT=0 -DGT_TRACE=0 -Dxdc_target_name__=C674 -Dxdc_target_types__=ti/targets/std.h -Dxdc_bld__profile_debug -Dxdc_bld__vers_1_0_6_1_5 --symdebug:dwarf  $(XDCINCS) -I$(ti.targets.C674.rootDir)/include -fs=./package/lib/lib/debug/vicp2codec1_ti -fr=./package/lib/lib/debug/vicp2codec1_ti
	-@$(FIXDEP) $@.dep $@.dep
	
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.s674:C_DIR=
package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.s674: PATH:=$(ti.targets.C674.rootDir)/bin/:$(PATH)

clean,674 ::
	-$(RM) package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.o674
	-$(RM) package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.o674
	-$(RM) package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.o674
	-$(RM) package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.s674
	-$(RM) package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.s674
	-$(RM) package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.s674

lib/debug/vicp2codec1_ti.a674: package/lib/lib/debug/vicp2codec1_ti/package/package_ti.sdo.fc.ires.examples.codecs.vicp2codec1.o674 package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti.o674 package/lib/lib/debug/vicp2codec1_ti/vicp2codec1_ti_ires.o674 lib/debug/vicp2codec1_ti.a674.mak

clean::
	-$(RM) lib/debug/vicp2codec1_ti.a674.mak
