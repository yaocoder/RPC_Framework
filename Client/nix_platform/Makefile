CXXFLAGS 	=	-O0 -g -Wall -fmessage-length=0 -DNDEBUG  -fPIC -DUSE_LOG4CXX -DUSE_LOG4CXX_PTR -DLINUX_PLATFORM 

MIDOBJS 	= 	SdkSrc/net_interface.o SdkSrc/net_interface_impl.o  \
		  		SdkSrc/net_data_layer.o SdkSrc/net_inter_layer.o SdkSrc/net_core_layer.o \

OBJS 		=	UnitTest/ClientServerInterface.o UnitTest/interface_test.o  $(MIDOBJS)

LIBS 		=	-lrt -lgtest  -lboost_filesystem -lboost_thread -lboost_regex \
				-llog4cxx  -levent -ljson -Wl,-rpath,/usr/local/lib 
		
LIBS_dynamic= 	-lboost_filesystem -lboost_thread -lboost_regex -lboost_system \
			   	-llog4cxx -levent  -ljson_dy \
				-Wl,-rpath,/usr/local/lib 

TARGET 		=	UnitTest/ClientServerInterface

TARGET_dynamic =libclient_channel.so 

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) 
	
$(TARGET_dynamic): $(MIDOBJS)
	 $(CXX)  -o $(TARGET_dynamic) $(MIDOBJS) -shared -fPIC  $(LIBS_dynamic) 

all:	$(TARGET) $(TARGET_dynamic)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET_dynamic)
