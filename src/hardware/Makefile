#link to hardware lib based on board
ifeq ($(TARGET),sam3x8e)
	MAKE_DIR = sam3x8eHardwareAPI/
	CP_ARGS =sam3x8eHardwareAPI/build/libsam3x8e.o ./api.o
	DEPEND = libsam3x8e.o
else
# TARGET board not recognized
	MAKE_DIR = BAD_TARGET
	CP_ARGS = BAD_TARGET
	DEPEND = BAD_TARGET
endif


api.o: $(DEPEND)
	cp $(CP_ARGS)

# tell make to allways call library make file
.PHONY: libsam3x8e.o
libsam3x8e.o:
	make -C $(MAKE_DIR)
