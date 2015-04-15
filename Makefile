APP = dimples

SRCEXT = cpp
SRCDIR = src
OBJDIR = obj
BINDIR = bin

SRCS := $(shell find $(SRCDIR) -name '*.$(SRCEXT)')
SRCDIRS := $(shell find . -name '*.$(SRCEXT)' -exec dirname {} \; | uniq)
OBJS := $(patsubst %.$(SRCEXT),$(OBJDIR)/%.o,$(SRCS))

DEBUG = -g
INCLUDES = -I./include
INCLUDES += -I/home/developer/3rdParty/boost/include
INCLUDES += -I/home/developer/3rdParty/ffmpeg/include

LIBS = -L/usr/lib/x86_64-linux-gnu 
LIBS += -L/usr/local/lib
LIBS += -L/home/developer/3rdParty/boost/lib
LIBS += -L/home/developer/3rdParty/ffmpeg/lib
LIBS += -lboost_regex

CFLAGS = -c -std=c++0x -fexceptions
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
