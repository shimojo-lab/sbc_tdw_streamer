# Makefile

CXX = g++
CXXFLAGS = -Wall -std=c++11 -O3 -mtune=native -march=native
HEAD = $(PWD)/src/head
DISP = $(PWD)/src/display
COMN = $(PWD)/src/common
CONF = $(PWD)/conf
BIN = $(PWD)/bin
CV_HDR = /usr/local/include/opencv2
JPEG_HDR = /opt/libjpeg-turbo/include
HEAD_LDFLAGS = -lboost_system -lboost_thread -lpthread -lturbojpeg \
               -lopencv_core -lopencv_imgproc -lopencv_videoio -L/opt/libjpeg-turbo/lib64
DISP_LDFLAGS = -lboost_system -lboost_thread -lpthread -lturbojpeg \
               -L/opt/libjpeg-turbo/lib32

# build the program for the head node
.PHONY: head
head: build_common build_head

# build the program for the display node
.PHONY: display
display: build_common build_display

# build the common modules
.PHONY: build_common
build_common: $(COMN)/mutex_logger.o $(COMN)/json_handler.o $(COMN)/base_config_parser.o \
			  $(COMN)/transceive_framebuffer.o

$(COMN)/mutex_logger.o: $(COMN)/mutex_logger.cpp
	$(CXX) $(CXXFLAGS) -I$(COMN)/include -c -o $@ $<

$(COMN)/json_handler.o: $(COMN)/json_handler.cpp
	$(CXX) $(CXXFLAGS) -I$(COMN)/include -c -o $@ $<

$(COMN)/base_config_parser.o: $(COMN)/base_config_parser.cpp
	$(CXX) $(CXXFLAGS) -I$(COMN)/include -c -o $@ $<

$(COMN)/transceive_framebuffer.o: $(COMN)/transceive_framebuffer.cpp
	$(CXX) $(CXXFLAGS) -I$(COMN)/include -c -o $@ $<

# build the program for the head node
.PHONY: build_head
build_head: $(COMN)/mutex_logger.o $(COMN)/base_config_parser.o $(COMN)/json_handler.o \
            $(COMN)/transceive_framebuffer.o $(HEAD)/config_parser.o $(HEAD)/frame_encoder.o \
            $(HEAD)/frame_sender.o $(HEAD)/sync_manager.o $(HEAD)/frontend_server.o $(HEAD)/main.o
	$(CXX) $(HEAD_LDFLAGS) -o $(BIN)/head_server $^

$(HEAD)/config_parser.o: $(HEAD)/config_parser.cpp
	$(CXX) $(CXXFLAGS) -I$(HEAD)/include -I$(COMN)/include -I$(JPEG_HDR) -c -o $@ $<

$(HEAD)/frame_encoder.o: $(HEAD)/frame_encoder.cpp
	$(CXX) $(CXXFLAGS) -I$(HEAD)/include -I$(COMN)/include -I$(CV_HDR) -I$(JPEG_HDR) -c -o $@ $<

$(HEAD)/frame_sender.o: $(HEAD)/frame_sender.cpp
	$(CXX) $(CXXFLAGS) -I$(HEAD)/include -I$(COMN)/include -c -o $@ $<

$(HEAD)/sync_manager.o: $(HEAD)/sync_manager.cpp
	$(CXX) $(CXXFLAGS) -I$(HEAD)/include -I$(COMN)/include -I$(JPEG_HDR) -c -o $@ $<

$(HEAD)/frontend_server.o: $(HEAD)/frontend_server.cpp
	$(CXX) $(CXXFLAGS) -I$(HEAD)/include -I$(COMN)/include -I$(JPEG_HDR) -c -o $@ $<

$(HEAD)/main.o: $(HEAD)/main.cpp
	$(CXX) $(CXXFLAGS) -I$(HEAD)/include -I$(COMN)/include -I$(JPEG_HDR) -c -o $@ $<

# build the program for the display node
.PHONY: build_display
build_display: $(COMN)/mutex_logger.o $(COMN)/base_config_parser.o $(COMN)/json_handler.o \
               $(COMN)/transceive_framebuffer.o $(DISP)/config_parser.o $(DISP)/view_framebuffer.o \
               $(DISP)/sync_message_generator.o $(DISP)/frame_receiver.o $(DISP)/frame_decoder.o \
               $(DISP)/frame_viewer.o $(DISP)/display_client.o $(DISP)/main.o
	$(CXX) $(DISP_LDFLAGS) -o $(BIN)/display_client $^

$(DISP)/config_parser.o: $(DISP)/config_parser.cpp
	$(CXX) $(CXXFLAGS) -I$(DISP)/include -I$(COMN)/include -c -o $@ $<

$(DISP)/view_framebuffer.o: $(DISP)/view_framebuffer.cpp
	$(CXX) $(CXXFLAGS) -I$(DISP)/include -I$(COMN)/include -c -o $@ $<

$(DISP)/sync_message_generator.o: $(DISP)/sync_message_generator.cpp
	$(CXX) $(CXXFLAGS) -I$(DISP)/include -I$(COMN)/include -I$(JPEG_HDR) -c -o $@ $<

$(DISP)/frame_receiver.o: $(DISP)/frame_receiver.cpp
	$(CXX) $(CXXFLAGS) -I$(DISP)/include -I$(COMN)/include -c -o $@ $<

$(DISP)/frame_decoder.o: $(DISP)/frame_decoder.cpp
	$(CXX) $(CXXFLAGS) -I$(DISP)/include -I$(COMN)/include -I$(JPEG_HDR) -c -o $@ $<

$(DISP)/frame_viewer.o: $(DISP)/frame_viewer.cpp
	$(CXX) $(CXXFLAGS) -I$(DISP)/include -I$(COMN)/include -I$(JPEG_HDR) -c -o $@ $<

$(DISP)/display_client.o: $(DISP)/display_client.cpp
	$(CXX) $(CXXFLAGS) -I$(DISP)/include -I$(COMN)/include -I$(JPEG_HDR) -c -o $@ $<

$(DISP)/main.o: $(DISP)/main.cpp
	$(CXX) $(CXXFLAGS) -I$(DISP)/include -I$(COMN)/include -I$(JPEG_HDR) -c -o $@ $<

# run the program for the head node
.PHONY: test_head
test_head:
	$(BIN)/head_server $(CONF)/head_conf.json

# run the program for the display node
.PHONY: test_display
test_display:
	$(BIN)/display_client $(CONF)/display_conf.json

# remove the binaries
.PHONY: clean
clean:
	rm -f $(BIN)/*
	rm -f $(HEAD)/*.o
	rm -f $(DISP)/*.o
	rm -f $(COMN)/*.o

