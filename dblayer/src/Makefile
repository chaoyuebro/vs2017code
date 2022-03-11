#############################
##     TOP Level PATH      ##
#############################
TO_ROOT = ..

############################
##     Include Paths      ##
############################
INCLUDES = $(3P_INCLUDES) $(COMMON_INCLUDES)

EXTRA_LIBS = -shared -lssl

# set subdirs
#SUBDIRS =

# Target Rules
target := libetpdb.so

############################
##    Common Defines      ##
############################
include $(TO_ROOT)/MakeDefines

SUBSRCS += $(shell find $(SUBDIRS) -name "*.cpp")
SUBOBJS = $(SUBSRCS:.cpp=.o)

$(target) : $(OBJS) $(SUBDIRS)
	$(QUIET_LINK)$(CPP) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJS) $(SUBOBJS) $(3P_LIBS) $(EXTRA_LIBS)


clean ::
	rm -rf $(target)

DEPENDINCDSTPATH := /usr/local/include/
DEPENDLIBPATH := $(TO_ROOT)/lib64/
DEPENDINCMYSQL := $(TO_ROOT)/include/mysql/*.h

TARGETNAME = etpdb
INCPREFIX = /usr/local/include/$(TARGETNAME)/
LIBPREFIX = /usr/local/lib/
LIBPREFIX2 = /opt/etp/lib/
RELEASE_INC = $(TO_ROOT)/include/*.h
INCMYSQL = /usr/local/include/etpmysql/

install: $(target)
	rm -rf $(INCPREFIX)
	rm -rf $(INCMYSQL)
	mkdir -p $(INCPREFIX) 
	mkdir -p $(LIBPREFIX) 
	mkdir -p $(LIBPREFIX2) 
	mkdir -p $(INCMYSQL)
	cp $(DEPENDINCMYSQL) $(INCMYSQL)
#	cp $(RELEASE_INC) $(INCPREFIX)
	cp $(target) $(LIBPREFIX)
	cp $(target) $(LIBPREFIX2)

