CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG -DUSE_LOG4CXX -DUSE_LOG4CXX_PTR
MIDOBJS = SdkSrc/net_interface.o SdkSrc/net_interface_impl.o  \
		  SdkSrc/net_data_layer.o SdkSrc/net_inter_layer.o SdkSrc/net_core_layer.o \

OBJS =	UnitTest/ClientServerInterface.o UnitTest/interface_test.o  $(MIDOBJS)

LIBS =	-lrt -lgtest  -lboost_filesystem -lboost_thread -lboost_regex \
		-llog4cxx  -levent -ljson -Wl,-rpath,/usr/local/lib 

TARGET =	UnitTest/ClientServerInterface

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) 

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
