#############################
##     TOP Level PATH      ##
#############################
TO_ROOT = ..

############################
##     Include Paths      ##
############################
EXTRA_INCLUDES = -I$(TO_ROOT)/src/redis -I$(TO_ROOT)/src -I/home/fuyf/ctsp_d/release/include
INCLUDES = $(3P_INCLUDES) $(COMMON_INCLUDES) $(EXTRA_INCLUDES)

EXTRA_LIBS = -L$(TO_ROOT)/src -lctspredis_d -lpbserdes 

# set subdirs
SUBDIRS = 

# Target Rules
target := test

############################
##    Common Defines      ##
############################
include $(TO_ROOT)/MakeDefines

$(target) : $(OBJS) $(SUBDIRS)
	$(QUIET_LINK)$(CPP) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJS) $(EXTRA_LIBS) $(3P_LIBS)

clean ::
	rm -rf $(target)
