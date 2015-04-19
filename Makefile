APP = dimples

SRCEXT = cpp
SRCDIR = src
OBJDIR = obj
BINDIR = bin

SRCS := $(shell find $(SRCDIR) -name '*.$(SRCEXT)')
SRCDIRS := $(shell find . -name '*.$(SRCEXT)' -exec dirname {} \; | uniq)
OBJS := $(patsubst %.$(SRCEXT),$(OBJDIR)/%.o,$(SRCS))

DEBUG = -g
INCLUDES = -I/usr/local/include
INCLUDES += -I./inc

LIBS = -L/usr/lib/x86_64-linux-gnu
LIBS += -lboost_regex
LIBS += -lboost_system -lboost_thread
LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_videostab
LIBS += -lavcodec \
		-lavformat \
        -lavutil \
        -lavdevice \
        -lavfilter \
        -lswscale \
        -lswresample
        # -lavresample \

# PortAudio Libs
#-lportaudio \

CFLAGS = -c -std=c++11 -fexceptions -fpermissive -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS 
CFLAGS += -Wall $(DEBUG) $(INCLUDES)
LDFLAGS =

ifeq ($(SRCEXT), cpp)
CC = $(CXX)
else
CFLAGS += -std=gnu99
endif

.PHONY: all clean distclean

all: $(BINDIR)/$(APP)

$(BINDIR)/$(APP): buildrepo $(OBJS)
	@mkdir -p `dirname $@`
	@echo "Linking $@..."
	@$(CC) $(OBJS) $(LDFLAGS) -o $@ $(LIBS)

$(OBJDIR)/%.o: %.$(SRCEXT)
	@echo "Generating dependencies for $<..."
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	$(RM) -r $(OBJDIR)

distclean: clean
	$(RM) -r $(BINDIR)

buildrepo:
	@$(call make-repo)

define make-repo
	for dir in $(SRCDIRS); \
	do \
		mkdir -p $(OBJDIR)/$$dir; \
	done
endef

# usage: $(call make-depend,source-file,object-file,depend-file)
define make-depend
   $(CC) -MM \
	 -MF $3 \
	 -MP \
	 -MT $2 \
	 $(CFLAGS) \
	 $1
endef
