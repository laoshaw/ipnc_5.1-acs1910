# (c) Texas Instruments 

ifndef $(COMMON_FOOTER_MK)
COMMON_FOOTER_MK = 1

OBJS=$(subst .c,.o, $(FILES) )

vpath %.o $(OBJ_DIR) 

LIB = $(MODULE).a

EXE = $(EXE_DIR)/$(MODULE).out

SO_LIB= $(EXE_DIR)/lib$(MODULE).so

.c.o:
	@echo  $(MODULE): Compiling $< 
	$(CC) $(CC_OPTS) $(OPTI_OPTS) $(DEFINE) $(INCLUDE) -o$(OBJ_DIR)/$@ $<


lib : $(LIB_DIR)/$(LIB)

$(LIB_DIR)/$(LIB) : $(OBJS)
	@echo  $(MODULE): Creating archive $(LIB)
	$(AR)	$(AR_OPTS) $(LIB_DIR)/$(LIB) $(OBJ_DIR)/*.o

obj: $(OBJS)
		
clean:
	@echo $(MODULE): Deleting temporary files   
	-rm -Rf MAKEFILE.DEPEND 	
	-rm -Rf $(LIB_DIR)
	-rm -Rf $(OBJ_DIR)
	-rm -Rf $(EXE_DIR)
	
depend:
	@echo $(MODULE): Making Directories, if not already created
	-mkdir -p $(LIB_DIR)	
	-mkdir -p $(OBJ_DIR)
	@echo $(MODULE): Building dependancies
	$(CC)	$(CC_OPTS) $(DEFINE) $(INCLUDE) $(FILES) -M > MAKEFILE.DEPEND

so:
	@echo $(MODULE): Linking to .so
	-mkdir -p $(EXE_DIR)
	$(LD) $(LD_OPTS) -shared -o$(SO_LIB) $(OBJ_DIR)/*.o $(LIBS)
	@echo Final shared library $(SO_LIB) !!!	

exe:
	@echo $(MODULE): Linking to .out
	-mkdir -p $(EXE_DIR)
	$(LD) $(LD_OPTS) -o$(EXE) $(LIBS)
	@echo Final executable $(EXE) !!!	
	
install:
	
			 
-include MAKEFILE.DEPEND  

endif # ifndef $(COMMON_FOOTER_MK)

